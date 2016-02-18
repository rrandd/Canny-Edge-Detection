// Written by Ivan Reedman
// (c) Copyright Ivan J Reedman 2016
// February 2016
// Canny Edge Detection implementation in C++
// Requires CIMG
// Optimisation #1

#include "include\CImg-1.6.9\CImg.h"

using namespace cimg_library;
using namespace std;

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cmath>
#include <algorithm>

//using namespace std;

// Define new source image for use
int imgsrcgrey[1000][1000];

// Define image to store blur
byte imgblur[1000][1000];

// Define image to store gradient intensity
byte imggrad[1000][1000];

// Define image to store gradient direction
byte imggraddir[1000][1000];

// Define image to store result of non-maximal suppression
byte imgnm[1000][1000];

int main()
{
	// Read image 
	CImg<unsigned char> imgsrc("bridge.bmp"); 

	// Display source image
	CImgDisplay main_disp(imgsrc, "Source"); 

	// Get width and height of source image
	unsigned int width = imgsrc.width();
	unsigned int height = imgsrc.height();

	// Define standard x and y used throughout code
	unsigned int x = 0, y = 0;

	// Get each pixel and copy to array
	for (x = 0; x <= width; x++) {
		for (y = 0; y <= height; y++) {
			imgsrcgrey[x][y] = imgsrc(x, y, 0, 0) * .2126 + imgsrc(x, y, 0, 1) * 0.7152 + imgsrc(x, y, 0, 2) * 0.0722;
		}
	}

	// Define standard pixel used throughout code
	unsigned char pixel;

	// Define standard timing vars
	double currenttime;
	double timetaken;

	// Output to console the image basics
	string imagespecifications;
	imagespecifications = "Optimisation #2\r\nImage Width=" + to_string(width) + " Height=" + to_string(height) + "\r\n\r\n";
	std::cout << imagespecifications;

	//---------------------------------------------------------------------------------------------
	// --------------------- Gaussian blur -----------------------
	// Get current time
	currenttime = (double)clock();

	// Apply Gaussian Blur
	std::cout << "Applying gaussian blur to image\r\n";

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
					pixel = imgsrcgrey[x + dx][y + dy];

					// Apply weighting
					blurpixel = blurpixel + pixel * pixelweight;
				}
			}

			// Write pixel to blur image
			imgblur[x][y] = (int)(blurpixel / 159);

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	std::cout << "- Applying gaussian blur to image took " << timetaken << " seconds\r\n\r\n";
	
	//---------------------------------------------------------------------------------------------
	// --------------------- Gradient intensity pass -----------------------
	// Get current time
	currenttime = (double)clock();
	
	// Apply Gaussian Blur
	std::cout << "Gradient intensity pass\r\n";

	// Definitions
	int pix[3] = { 0,0,0 };
	int gradx = 0, grady = 0;
	int graddir = 0, grad = 0;
	double tempa = 0, temps = 0, tempr = 0;

	// Get pixels and calculate gradient and direction
	for (x = 1; x <= width-1; x++) {
		for (y = 1; y <= height-1; y++) {

			// Get source pixels to calculate the intensity and direction
			pix[0] = imgblur[x][y];		// main pixel
			pix[1] = imgblur[x - 1][y];		// pixel left
			pix[2] = imgblur[x][y - 1];		// pixel above

			// get value for x gradient
			gradx = pix[0] - pix[1];

			// get value for y gradient
			grady = pix[0] - pix[2];

			// Calculate gradient direction
			// We want this rounded to 0,1,2,3 which represents 0, 45, 90, 135 degrees
			// The follow code replaces graddir = (int)(abs(atan2(grady, gradx)) + 0.22) * 80;

			// atan2 approximation
			if (max(abs(gradx), abs(grady)) == 0) {
				tempa = 0;
			}
			else {
				tempa = min(abs(gradx), abs(grady)) / max(abs(gradx), abs(grady));
			}
			temps = tempa * tempa;

			tempr = ((-0.0464964749 * temps + 0.15931422) * temps - 0.327622764) * temps * tempa + tempa;

			// Now sort out quadrant
			if (abs(grady) > abs(gradx)) tempr = 1.57079637 - tempr;

			if (gradx < 0) tempr = 3.14159274 - tempr;

			if (grady < 0) tempr = -tempr;

			graddir = (int)(tempr + 0.22) * 80;

			// Store gradient direction
			imggraddir[x][y] = graddir;

			// Get absolute values for both gradients
			gradx = abs(gradx);
			grady = abs(grady);

			// Calculate gradient
			if (gradx > grady) {

				grad = (grady * 414) / 1000 + gradx;

			}
			else {

				grad = (gradx * 414) / 1000 + grady;

			}

			// Store pixel
			imggrad[x][y] = grad;

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	std::cout << "- Calculating the intensity gradient of the image took " << timetaken << " seconds\r\n\r\n";

	//---------------------------------------------------------------------------------------------
	// --------------------- Non-Maximal Supression -----------------------
	// Get current time
	currenttime = (double)clock(); 
	
	// Apply Gaussian Blur
	std::cout << "Non-maximal supression pass\r\n";

	// Definitions
	int sensitivity = 10;

	// Get each pixel and apply the blur filter
	for (x = 2; x <= width - 2; x++) {
		for (y = 2; y <= height - 2; y++) {

			// First check that current pixel is above threshold
			if (imggrad[x][y] >= sensitivity) {

				// Get gradient direction
				graddir = imggraddir[x][y];		// Remember this was multiplied by 80 for the sake of display

				// If angle = 0
				if (graddir == 0) {
					// Is pixel local maximal
					if (imggrad[x][y] >= imggrad[x][y] && imggrad[x][y] >= imggrad[x][y + 1]) {
						// Write pixel to as max
						imgnm[x][y] = 255;

						// Supress other two
						imgnm[x][y - 1] = 0;
						imgnm[x][y + 1] = 0;

					}
					else {
						// Supress pixel
						imgnm[x][y] = 0;
					}
				} 

				// If angle = 45 degrees
				else if (graddir == 80) {
					// Is pixel local maximal
					if (imggrad[x][y] >= imggrad[x + 1][y - 1] && imggrad[x][y] >= imggrad[x - 1][y + 1]) {
						// Write pixel to as max
						imgnm[x][y] = 255;

						// Supress other two
						imgnm[x + 1][y - 1] = 0;
						imgnm[x - 1][y + 1] = 0;

					}
					else {
						// Supress pixel
						imgnm[x][y] = 0;
					}
				}

				// If angle = 90 degrees
				else if (graddir == 160) {
					// Is pixel local maximal
					if (imggrad[x][y] >= imggrad[x - 1][y] && imggrad[x][y] >= imggrad[x + 1][y]) {
						// Write pixel to as max
						imgnm[x][y] = 255;

						// Supress other two
						imgnm[x - 1][y] = 0;
						imgnm[x + 1][y] = 0;

					}
					else {
						// Supress pixel
						imgnm[x][y] = 0;
					}
				}

				// If angle = 135 degrees
				else if (graddir == 240) {
					// Is pixel local maximal
					if (imggrad[x][y] >= imggrad[x - 1][y - 1] && imggrad[x][y] >= imggrad[x + 1][y + 1]) {
						// Write pixel to as max
						imgnm[x][y] = 255;

						// Supress other two
						imgnm[x - 1][y - 1] = 0;
						imgnm[x + 1][y + 1] = 0;

					}
					else {
						// Supress pixel
						imgnm[x][y] = 0;
					}
				}
				
			}
			else {

				// Supress pixel
				imgnm[x][y] = 0;

			}

		}
	}

	// Display time taken to process image
	timetaken = (double)((double)clock() - currenttime) / CLOCKS_PER_SEC;

	std::cout << "- Applying non-maxmimal supression to image took " << timetaken << " seconds\r\n";

	// Output final image
	// Define image to store output
	CImg<unsigned char> imgoutput(width, height, 1, 1, 1);

	// Get each pixel and copy to array
	for (x = 0; x <= width - 1; x++) {
		for (y = 0; y <= height - 1; y++) {
			imgoutput(x, y, 0, 0) = imgnm[x][y];
		}
	}

	// Display image
	CImgDisplay main_disp_output(imgoutput, "output");

	while (!main_disp.is_closed())
		main_disp.wait();
	return 0; 
}
