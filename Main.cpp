#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <map>

/**
 * \file	Main.cpp
 * \brief 	This is an implementation of image comparison using color 
 * 		coherence vectors. Its' goal is mainly to determine how
 *		good this method is in the context of applying textures
 *		in polygonal models obtained from 3d point clouds by
 *		3d reconstruction.
 *
 * \author	Kim Oliver Rinnewitz, krinnewitz@uos.de
 */

using namespace std;

/**
 * \brief 	Implementation of the find algorithm for disjoint sets.
 * 
 * \param 	x	The element to find
 * \param	parent	The disjoint set data structure to work on (tree)
 *
 * \return 	The number of the set which contains the given element
 */
unsigned long int find(unsigned long int x, unsigned long int parent[])
{
	while(parent[x] != x)
	{
		x = parent[x];
	}
	return x;
}

/**
 * \brief	Implementation of the union algorithm for disjoint sets.
 *
 * \param	x	The first set for the two sets to unite 
 * \param	y	The second set for the two sets to unite 
 * \param	parent	The disjoint set data structure to work on (tree)
 */
void unite(unsigned long int x, unsigned long int y, unsigned long int parent[])
{
	parent[find(x, parent)] = find(y, parent);
}

/**
 * \brief 	Labels connected components in the given image.
 *		This is an implementation of the algorithm of
 *		Rosenfeld et al
 * 
 * \param	input	The image to label connected components in
 * \param	output	The destination to hold the labels
 */
void connectedCompLabeling(cv::Mat input, cv::Mat &output)
{
	//Allocate output and set it to zero
	output = cv::Mat(input.size(), CV_16U);
	output.setTo(cv::Scalar(0));
	
	//1 channel pointer to input image
	cv::Mat_<uchar>& ptrInput = (cv::Mat_<uchar>&)input;
	//1 channel pointer to output image
	cv::Mat_<ushort>& ptrOutput = (cv::Mat_<ushort>&)output; 

	//disjoint set data structure to manage the labels 
	unsigned long int* parent = new unsigned long int[output.size().height * output.size().width];
	for(unsigned long int i = 0; i < output.size().height * output.size().width; i++) parent[i] = i;

	//first pass: Initial labeling
	unsigned short int currentLabel = 0;
	for (int y = 0; y < input.size().height; y++)
	{
		for(int x = 0; x < input.size().width; x++)
		{
			if (y == 0)
			{
				if(x == 0)
				{
					//First pixel. Create first label.
					ptrOutput(y,x) = ++currentLabel;
				}
				else
				{
					//First row. Only check left pixel	
					if (ptrInput(y,x) == ptrInput(y, x - 1))
					{
						//same region as left pixel -> assign same label
						ptrOutput(y,x) = ptrOutput(y, x - 1);
					}
					else
					{
						//different region -> create new label
						ptrOutput(y,x) = ++currentLabel;
					}
				}
			}
			else
			{
				if(x == 0)
				{
					//First column. Only check top pixel	
					if (ptrInput(y,x) == ptrInput(y - 1, x))
					{
						//same region as top pixel -> assign same label
						ptrOutput(y,x) = ptrOutput(y - 1, x);
					}
					else
					{
						//different region -> create new label
						ptrOutput(y,x) = ++currentLabel;
					}
				}
				else
				{
					//Regular column. Check top and left pixel
					if (ptrInput(y,x) == ptrInput(y, x - 1) && ptrInput(y,x) == ptrInput(y - 1, x))
					{
						//same region as left and top pixel -> assign minimum label of both
						ptrOutput(y,x) = min(ptrOutput(y, x - 1), ptrOutput(y - 1, x));
						if (ptrOutput(y, x - 1) != ptrOutput(y - 1, x))
						{
							//mark labels as equivalent
							//we are using the union/find algorithm for disjoint sets
							unite(find(ptrOutput(y, x - 1), parent),
							      find(ptrOutput(y - 1, x), parent), parent);
						}
					}
					else
					if (ptrInput(y,x) == ptrInput(y, x - 1))
					{
						//same region as left pixel -> assign same label
						ptrOutput(y,x) = ptrOutput(y, x - 1);
					}
					else
					if (ptrInput(y,x) == ptrInput(y - 1, x))
					{
						//same region as top pixel -> assign same label
						ptrOutput(y,x) = ptrOutput(y - 1, x);
					}
					else
					{
						//different region -> create new label
						ptrOutput(y,x) = ++currentLabel;
					}
				}
			}
		}
	}
	
	//second pass: Merge equivalent labels
	for (int y = 0; y < output.size().height; y++)
	{
		for(int x = 0; x < output.size().width; x++)
		{
			//we are using the union/find algorithm for disjoint sets
			ptrOutput(y,x) = (unsigned short int) find(ptrOutput(y, x), parent);
		}
	}

	delete[] parent;
}

/**
 * \brief 	Calculates the coherence of each pixel. The
 *		coherence is the size of the pixel's connected
 *		component. So we just have to count the number
 *		of occurances of the pixel's label.
 * 
 * \param	inputColors	The image belonging to the labled
 *				connected components
 * \param	inputLabels	The labled connected components 
 * \return	A map containing the size and color value for each
 *		connected component
 */
map<ushort, pair<uchar, ulong> >calcCoherence(cv::Mat inputColors, cv::Mat inputLabels)
{
	//1 channel pointer to input image
	cv::Mat_<ushort>& ptrInputLabels = (cv::Mat_<ushort>&)inputLabels;
	//1 channel pointer to input colors image
	cv::Mat_<uchar>& ptrInputColors = (cv::Mat_<uchar>&)inputColors;

	//Map to hold the number of pixels and the color per label
	map<ushort, pair<uchar, ulong> > coherences;


	//calculate coherence values per label	
	for (int y = 0; y < inputLabels.size().height; y++)
	{
		for(int x = 0; x < inputLabels.size().width; x++)
		{
			if (coherences.find(ptrInputLabels(y,x)) != coherences.end())
			{
				coherences[ptrInputLabels(y,x)].second++;
			}
			else
			{
				coherences[ptrInputLabels(y,x)].second = 1;
				coherences[ptrInputLabels(y,x)].first = ptrInputColors(y,x);
			}
		}
	}

	return coherences;
}


/**
 * \brief Reduces the number of colors in the given image
 * 
 * \param input		The input image to reduce the colors in.
			This must be a 3 channel image with 8 bit
			per channel.
 * \param output 	The destination to store the result in.
			This will be an 8 bit one channel image.
 * \param numColors	The maximum number of colors in the 
 *			output image. Note, that this value must
 *			be less than or equal to 256 since the 
 *			output image has only one 8 bit channel.
 */
void reduceColors(cv::Mat input, cv::Mat &output, int numColors)
{
	//allocate output
	output = cv::Mat(input.size(), CV_8U);
	//3 channel pointer to input image
	cv::Mat_<cv::Vec3b>& ptrInput = (cv::Mat_<cv::Vec3b>&)input; 
	//1 channel pointer to output image
	cv::Mat_<uchar>& ptrOutput = (cv::Mat_<uchar>&)output;

	for (int y = 0; y < input.size().height; y++)
	{
		for(int x = 0; x < input.size().width; x++)
		{
			unsigned long int currCol = 0;
			currCol |= (ptrInput(y, x)[0]) << 16;
			currCol |= (ptrInput(y, x)[1]) <<  8;
			currCol |= (ptrInput(y, x)[2]) <<  0;
			ptrOutput(y,x) = currCol / (pow(2, 24) / numColors);
		}
	}
}


/**
 * \brief Calculates the CCV for the given image.
 * 
 * \param	img			The image to calculate the CCV for
 * \param	numColors		The number of colors to reduce the image to
 * \param	coherenceThreshold	A threshold wich indicates the minimum size
 *					of a color region to consider it's pixels as
 *					coherent
 *
 * \return	A color coherence vector given by a map that holds the alpha and beta
 *		values for each color.
 */
map< uchar, pair<ulong, ulong> > calculateCCV(cv::Mat img, int numColors, int coherenceThreshold)
{
	//blurred image
	cv::Mat blurred;

	//color reduced image
	cv::Mat reduced;

	//connected components
	cv::Mat labledComps;

	//Step 1: Blur the image slightly with a 3x3 box filter
	cv::blur(img, blurred, cv::Size(3,3)); //3x3 box filter

	//Step 2: Discretize the colorspace and reude the number
	//	  colors to numColors
	reduceColors(blurred, reduced, numColors);
	
	//Step 3: Label connected components in the image in order
	//	  to determine the coherence of each pixel. The 
	//	  coherence is the size of the connected component
	//	  of the current pixel.
	connectedCompLabeling(reduced, labledComps);	
	//  label         color  size
	map<ushort, pair<uchar, ulong> > coherenceMap = calcCoherence(reduced, labledComps);	

	//Step 4: Calculate the CCV
	//This map holds the alpha and beta values for each color and can be refered to 
	//as the color coherence vector.
	//   color        alpha  beta
	map< uchar, pair<ulong, ulong> > ccv;

	//Iterator over the coherenceMap
	map<ushort, pair<uchar, ulong> >::iterator it;

	//Walk through the coherence map and sum up the incoherent and 
	//coherent pixels for every color
	for(it = coherenceMap.begin(); it != coherenceMap.end(); it++)
	{
		if (ccv.find(it->second.first) != ccv.end())
		{
			//we already have this color in the ccv
			if (it->second.second >= coherenceThreshold)
			{
				//pixels in current blob are coherent -> increase alpha
				ccv[it->second.first].first += it->second.second;
			}
			else
			{
				//pixels in current blob are incoherent -> increase beta
				ccv[it->second.first].second += it->second.second;
			}
		}
		else
		{
			//we don't have this color in our ccv yet and need to add it
			if (it->second.second >= coherenceThreshold)
			{
				//pixels in current blob are coherent -> set alpha
				ccv[it->second.first].first = it->second.second;
				ccv[it->second.first].second = 0;
			}
			else
			{
				//pixels in current blob are incoherent -> set beta
				ccv[it->second.first].first = 0;
				ccv[it->second.first].second = it->second.second;
			}
		}
			
	}

	//set unused colors to 0	
	for(unsigned char c = 0; c < numColors; c++)
	{
		if (ccv.find(c) == ccv.end())
		{
			ccv[c].first  = 0;
			ccv[c].second = 0;
		}
	}

	return ccv;
}


/**
 * \brief	Calculates the distance between the two given CCVs.
 *
 * \param	ccv1	The first CCV
 * \param	ccv2	The second CCV
 *
 * \return	The distance between the two CCVs
 */
unsigned long int compareCCVs(map< uchar, pair<ulong, ulong> > ccv1, map< uchar, pair<ulong, ulong> > ccv2)
{
	unsigned long int result = 0;

	map< uchar, pair<ulong, ulong> >::iterator ccvit;

	for(ccvit = ccv1.begin(); ccvit != ccv1.end(); ccvit++)
	{
		//|alpha1 - alpha2| + |beta1 - beta2|
		result += abs((int)ccvit->second.first  - (int)ccv2[ccvit->first].first)
			+ abs((int)ccvit->second.second - (int)ccv2[ccvit->first].second);
	}
	return result;
}

int main (int argc, char** argv)
{
	if (argc == 5)
	{
		//number of colors to reduce the color space to.
		//This value has to be smaller than or equal to 256.
		int numColors = atoi(argv[3]);
		
		//threshold that tells the minimum size of a connected
		//component blob needed to treat a pixel as coherent
		int coherenceThreshold = atoi(argv[4]);

		//input images
		cv::Mat img1 = cv::imread(argv[1]);
		cv::Mat img2 = cv::imread(argv[2]);
		

		//   color        alpha  beta
		map< uchar, pair<ulong, ulong> > ccv1 = calculateCCV(img1, numColors, coherenceThreshold);
		map< uchar, pair<ulong, ulong> > ccv2 = calculateCCV(img2, numColors, coherenceThreshold);

		//debug output
		cout<<compareCCVs(ccv1, ccv2)<<endl;

		//debug output: Write the CCV to standard output
		//map< uchar, pair<ulong, ulong> >::iterator ccvit;
		//for(ccvit = ccv1.begin(); ccvit != ccv1.end(); ccvit++)
		//{
		//	cout<<(uint)ccvit->first<<"\t: ("<<ccvit->second.first<<", "<<ccvit->second.second<<")"<<endl;
		//}
		return EXIT_SUCCESS;			
	}
	else
	{
		cout<<"Usage: "<<argv[0]<<" <first image> <second image> <number of colors> <coherence threshold>"<<endl;
		return EXIT_FAILURE;
	}
}
