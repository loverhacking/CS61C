/************************************************************************
**
** NAME:        steganography.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**				Justin Yokota - Starter Code
**				YOUR NAME HERE
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

//Determines what color the cell at the given row/col should be. This should not affect Image, and should allocate space for a new Color.
Color *evaluateOnePixel(Image *image, int row, int col) {
	Color* newColor = malloc(sizeof(Color));
	if (image->image[row][col].B % 2 == 0) {
		// black (0, 0, 0)
		newColor->R = 0;
		newColor->G = 0;
		newColor->B = 0;
	} else {
		// while (255, 255, 255)
		newColor->R = 255;
		newColor->G = 255;
		newColor->B = 255;
	}
	return newColor;

}

//Given an image, creates a new image extracting the LSB of the B channel.
Image *steganography(Image *image)
{
	Image* newImage = malloc(sizeof(Image));
	newImage->image = malloc(image->rows * sizeof(Color*));
	newImage->rows = image->rows;
	newImage->cols = image->cols;
	for (int i = 0; i < image->rows; i++) {
		newImage->image[i] = malloc(image->cols * sizeof(Color));
		for (int j = 0; j < image->cols; j++) {
			Color* newColor = evaluateOnePixel(image, i, j);
			newImage->image[i][j].R = newColor->R;
			newImage->image[i][j].G = newColor->G;
			newImage->image[i][j].B = newColor->B;
			free(newColor);
		}
	}
	return newImage;

}

/*
Loads a file of ppm P3 format from a file, and prints to stdout (e.g. with printf) a new image, 
where each pixel is black if the LSB of the B channel is 0, 
and white if the LSB of the B channel is 1.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a file of ppm P3 format (not necessarily with .ppm file extension).
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!
*/
int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "wrong number of arguments\n");
		return -1;
	}
	Image* image = readData(argv[1]);
	Image* newImage = steganography(image);
	freeImage(image);
	writeData(newImage);
	freeImage(newImage);
	return 0;
}
