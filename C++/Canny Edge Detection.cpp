// Written by Ivan Reedman
// (c) Copyright Ivan J Reedman 2016
// February 2016
// Canny Edge Detection implementation in C++
// Requires CIMG

#include "include\CImg-1.6.9\CImg.h"

using namespace cimg_library;
using namespace std;

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cmath>

//using namespace std;

int main()
{
	// Read image 
	CImg<unsigned char> imgsrc("bridge.bmp"); 

	// Display source image
	CImgDisplay main_disp(imgsrc, "Source"); 

	// Define new source image for use
	CImg<unsigned char> imgsrcgrey;
	
	// Convert source RGB into 8bit greyscale image
	imgsrcgrey=imgsrc.get_RGBtoYCbCr().get_channel(0);

	// Display grey scale source image
	CImgDisplay main_disp_grey(imgsrcgrey, "Grey"); 

	// Get width and height of source image
	unsigned int width=imgsrc.width();
	unsigned int height=imgsrc.height();

	// Define standard x and y used throughout code
	unsigned int x=0, y=0;

	// Define standard pixel used throughout code
	unsigned char pixel;

	// Define standard timing vars
	double currenttime;
	double timetaken;

	// Output to console the image basics
	string imagespecifications;
	imagespecifications = "Image Width=" + to_string(width) + " Height=" + to_string(height) + "\r\n\r\n";
	cout << imagespecifications;

	//---------------------------------------------------------------------------------------------
	// --------------------- Gaussian blur -----------------------
	// Define image to store blur
	CImg<unsigned char> imgblur(width, height, 1, 1, 1);
	
	// Get current time
	currenttime = (double)clock();

	// Apply Gaussian Blur
	cout << "Applying gaussian blur to image\r\n";

	// Definitions
	unsigned int blurpixel=0;
	signed int dx = 0, dy = 0;
	unsigned int pixelweight = 0;

	// Define gaussian blur weightings array
	int weighting[5][5] =
	{
		{ 2, 4, 5, 4, 2},
		{ 4, 9,12, 9, 4},
		{ 5,12,15,12, 5},
		{ 4, 9,12, 9, 4},
		{ 2, 4, 5, 4, 2}
	};


	// Get each pixel and apply the blur filter
	for (x = 2; x <= width - 2; x++) {
		for (y = 2; y <= height - 2; y++) {

			// Clear blurpixel
			blurpixel = 0;

			// +- 2 for each pixel and calculate the weighting
			for (dx = -2; dx <= 2; dx++) {
				for (dy = -2; dy <= 2; dy++) {
					pixelweight = weighting[dx+2][dy+2];		

					// Get pixel
					pixel = imgsrcgrey(x + dx, y + dy, 0, 0);

					// Apply weighting
					blurpixel = blurpixel + pixel * pixelweight;
				}
			}

			// Write pixel to blur image
			imgblur(x, y, 0, 0) = (int)(blurpixel / 159);

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	std::cout << "- Applying gaussian blur to image took " << timetaken << " seconds\r\n\r\n";
	
	// Display blur image
	CImgDisplay main_disp_blur(imgblur, "blur");

	//---------------------------------------------------------------------------------------------
	// --------------------- Gradient intensity pass -----------------------
	// Define image to store gradient intensity
	CImg<unsigned char> imggrad(width, height, 1, 1, 1);

	// Define image to store gradient direction
	CImg<unsigned char> imggraddir(width, height, 1, 1, 1);

	// Get current time
	currenttime = (double)clock();
	
	// Apply Gaussian Blur
	cout << "Gradient intensity pass\r\n";

	// Definitions
	int pix[3] = { 0,0,0 };
	int gradx = 0, grady = 0;
	int graddir = 0, grad = 0;

	// Get pixels and calculate gradient and direction
	for (x = 1; x <= width-1; x++) {
		for (y = 1; y <= height-1; y++) {

			// Get source pixels to calculate the intensity and direction
			pix[0] = imgblur(x, y, 0, 0);			// main pixel
			pix[1] = imgblur(x - 1, y, 0, 0);		// pixel left
			pix[2] = imgblur(x, y - 1, 0, 0);		// pixel above

			// get value for x gradient
			gradx = pix[0] - pix[1];

			// get value for y gradient
			grady = pix[0] - pix[2];

			// Calculate gradient direction
			// We want this rounded to 0,1,2,3 which represents 0, 45, 90, 135 degrees
			graddir = (int)(abs(atan2(grady, gradx)) + 0.22) * 80;

			// Store gradient direction
			imggraddir(x, y, 0, 0) = graddir;

			// Calculate gradient
			grad = (int)sqrt(gradx * gradx + grady * grady) * 2;

			// Store pixel
			imggrad(x, y, 0, 0) = grad;

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	cout << "- Calculating the intensity gradient of the image took " << timetaken << " seconds\r\n\r\n";

	// Display gradient intensity image
	CImgDisplay main_disp_grad(imggrad, "grad");

	// Display gradient direction image
	CImgDisplay main_disp_grad_dir(imggraddir, "graddir");

	//---------------------------------------------------------------------------------------------
	// --------------------- Gaussian blur -----------------------
	// Define image to store blur
	CImg<unsigned char> imgnm(width, height, 1, 1, 1);

	// Get current time
	currenttime = (double)clock(); 
	
	// Apply Gaussian Blur
	cout << "Non-maximal supression pass\r\n";

	// Definitions
	int sensitivity = 10;

	// Get each pixel and apply the blur filter
	for (x = 2; x <= width - 2; x++) {
		for (y = 2; y <= height - 2; y++) {

			// First check that current pixel is above threshold
			if (imggrad(x, y, 0, 0) >= sensitivity) {

				// Get gradient direction
				graddir = imggraddir(x, y, 0, 0);		// Remember this was multiplied by 80 for the sake of display

				// If angle = 0
				if (graddir == 0) {
					// Is pixel local maximal
					if (imggrad(x, y, 0, 0) >= imggrad(x, y - 1, 0, 0) && imggrad(x, y, 0, 0) >= imggrad(x, y + 1, 0, 0)) {
						// Write pixel to as max
						imgnm(x, y, 0, 0) = 255;

						// Supress other two
						imgnm(x, y - 1, 0, 0) = 0;
						imgnm(x, y + 1, 0, 0) = 0;

					}
					else {
						// Supress pixel
						imgnm(x, y, 0, 0) = 0;
					}
				} 

				// If angle = 45 degrees
				else if (graddir == 80) {
					// Is pixel local maximal
					if (imggrad(x, y, 0, 0) >= imggrad(x + 1, y - 1, 0, 0) && imggrad(x, y, 0, 0) >= imggrad(x - 1, y + 1, 0, 0)) {
						// Write pixel to as max
						imgnm(x, y, 0, 0) = 255;

						// Supress other two
						imgnm(x + 1, y - 1, 0, 0) = 0;
						imgnm(x - 1, y + 1, 0, 0) = 0;

					}
					else {
						// Supress pixel
						imgnm(x, y, 0, 0) = 0;
					}
				}

				// If angle = 90 degrees
				else if (graddir == 160) {
					// Is pixel local maximal
					if (imggrad(x, y, 0, 0) >= imggrad(x - 1, y, 0, 0) && imggrad(x, y, 0, 0) >= imggrad(x + 1, y, 0, 0)) {
						// Write pixel to as max
						imgnm(x, y, 0, 0) = 255;

						// Supress other two
						imgnm(x - 1, y, 0, 0) = 0;
						imgnm(x + 1, y, 0, 0) = 0;

					}
					else {
						// Supress pixel
						imgnm(x, y, 0, 0) = 0;
					}
				}

				// If angle = 135 degrees
				else if (graddir == 240) {
					// Is pixel local maximal
					if (imggrad(x, y, 0, 0) >= imggrad(x - 1, y - 1, 0, 0) && imggrad(x, y, 0, 0) >= imggrad(x + 1, y + 1, 0, 0)) {
						// Write pixel to as max
						imgnm(x, y, 0, 0) = 255;

						// Supress other two
						imgnm(x - 1, y - 1, 0, 0) = 0;
						imgnm(x + 1, y + 1, 0, 0) = 0;

					}
					else {
						// Supress pixel
						imgnm(x, y, 0, 0) = 0;
					}
				}
				
			}
			else {

				// Supress pixel
				imgnm(x, y, 0, 0) = 0;

			}

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	std::cout << "- Applying non-maxmimal supression to image took " << timetaken << " seconds\r\n";

	// Display blur image
	CImgDisplay main_disp_nm(imgnm, "nm");


	while (!main_disp.is_closed())
		main_disp.wait();
	return 0;
}
