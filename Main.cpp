#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include "CCV.hpp"
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
		
		//calculate CCVs
		lssr::CCV* ccv1 = new lssr::CCV(img1, numColors, coherenceThreshold);
		lssr::CCV* ccv2 = new lssr::CCV(img2, numColors, coherenceThreshold);

		//debug output
		cout<<ccv1->compareTo(ccv2)<<endl;

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
