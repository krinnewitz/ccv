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

	//first pass: Initial labeling
	int currentLabel = 0;
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
					//First row. Only check leftmost pixel	
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
					//First column. Only check topmost pixel	
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
					//Regular column. Check topmost and leftmost pixel
					if (ptrInput(y,x) == ptrInput(y, x - 1) && ptrInput(y,x) == ptrInput(y - 1, x))
					{
						//same region as left and top pixel -> assign minimum label of both
						ptrOutput(y,x) = min(ptrOutput(y, x - 1), ptrOutput(y - 1, x));
						if (ptrOutput(y, x - 1) != ptrOutput(y - 1, x))
						{
							//mark labels as equivalent
							//TODO
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
	for (int y = 0; y < input.size().height; y++)
	{
		for(int x = 0; x < input.size().width; x++)
		{
			//TODO
		}
	}
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


int main (int argc, char** argv)
{
	//number of colors to reduce the color space to
	const int numColors = 64;
	
	//threshold that tells the minimum size of a connected
	//component blob needed to treat a pixel as coherent
	const int coherenceThreshold = 10;

	//input image
	cv::Mat img = cv::imread(argv[1]);
	
	//blurred image
	cv::Mat blurred;

	//color reduced image
	cv::Mat reduced(img.size(), CV_8U);

	//3 channel pointer to blurred image
	cv::Mat_<cv::Vec3b>& ptrBlurred = (cv::Mat_<cv::Vec3b>&)blurred; 
	
	//1 channel pointer to reduced image
	cv::Mat_<uchar>& ptrReduced = (cv::Mat_<uchar>&)reduced; 


	//Step 1: Blur the image slightly with a 3x3 box filter
	cv::blur(img, blurred, cv::Size(3,3)); //3x3 box filter

	//Step 2: Discretize the colorspace and reude the number
	//	  colors to numColors
	for (int y = 0; y < blurred.size().height; y++)
	{
		for(int x = 0; x < blurred.size().width; x++)
		{
			unsigned long int currCol = 0;
			currCol |= (ptrBlurred(y, x)[0]) << 16;
			currCol |= (ptrBlurred(y, x)[1]) <<  8;
			currCol |= (ptrBlurred(y, x)[2]) <<  0;
			ptrReduced(y,x) = currCol / (pow(2, 24) / numColors);
		}
	}
	
	//Step 3: Label connected components in the image in order
	//	  to determine the coherence of each pixel. The 
	//	  coherence is the size of the connected component
	//	  of the current pixel.
	cv::Mat labledComps;
	connectedCompLabeling(reduced, labledComps);	
	//  label         color  size
	map<ushort, pair<uchar, ulong> > coherence_map = calcCoherence(reduced, labledComps);	
	map<ushort, pair<uchar, ulong> >::iterator it;

	//Step 4: Calculate the CCVs
	//   color        alpha  beta
	map< uchar, pair<ulong, ulong> > ccv;
	map< uchar, pair<ulong, ulong> >::iterator ccvit;
	for(it = coherence_map.begin(); it != coherence_map.end(); it++)
	{
		if (ccv.find(it->second.first) != ccv.end())
		{
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

	for(ccvit = ccv.begin(); ccvit != ccv.end(); ccvit++)
	{
		cout<<(uint)ccvit->first<<"\t: ("<<ccvit->second.first<<", "<<ccvit->second.second<<")"<<endl;
	}
		

	return 0;
}
