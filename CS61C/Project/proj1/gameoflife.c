/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"



//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule) {
	Color *newColor = (Color *)malloc(sizeof(Color));
    if (newColor == NULL) {
        return NULL;
    }

    Color oldColor = image->image[row][col];

    // the positions of the eight neighboring cells are precomputed
    Color *neighbors[8];
    int nidx = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;

            int nr = row + dx;
			if (nr < 0) {
				nr = image->rows - 1;
			}
        	if (nr == image->rows) {
        		nr = 0;
        	}

            int nc = col + dy;
        	if (nc < 0) {
        		nc = image->cols - 1;
        	}
        	if (nc == image->cols) {
        		nc = 0;
        	}
            neighbors[nidx++] = &(image->image[nr][nc]);
        }
    }

    /*
    The live neighbors are counted by checking the same bit in the neighboring cells' red channels.
    The new state of the bit is determined using the rule and combined into the new red value.
	The same process is repeated for the green and blue channels.
	*/
    uint8_t newR = 0;
    uint8_t newG = 0;
    uint8_t newB = 0;
    for (int bit = 0; bit < 8; bit++) {
        int current_bit_R = (oldColor.R >> bit) & 1;
        int current_bit_G = (oldColor.G >> bit) & 1;
        int current_bit_B = (oldColor.B >> bit) & 1;

        int count_R = 0;
        int count_G = 0;
        int count_B = 0;
        for (int i = 0; i < 8; i++) {
            Color neighbor = *neighbors[i];
            int neighbor_bit_R = (neighbor.R >> bit) & 1;
            count_R += neighbor_bit_R;

			int neighbor_bit_G = (neighbor.G >> bit) & 1;
            count_G += neighbor_bit_G;

			int neighbor_bit_B = (neighbor.B >> bit) & 1;
            count_B += neighbor_bit_B;
        }

        int new_bit_R;
        int new_bit_G;
        int new_bit_B;
        if (current_bit_R) {
            new_bit_R = (rule >> (count_R + 9)) & 1;
        } else {
            new_bit_R = (rule >> count_R) & 1;
        }
        newR |= (new_bit_R << bit);

		if (current_bit_G) {
            new_bit_G = (rule >> (count_G + 9)) & 1;
        } else {
            new_bit_G = (rule >> count_G) & 1;
        }
        newG |= (new_bit_G << bit);

       if (current_bit_B) {
            new_bit_B = (rule >> (count_B + 9)) & 1;
        } else {
            new_bit_B = (rule >> count_B) & 1;
        }
        newB |= (new_bit_B << bit);
    }

    newColor->R = newR;
    newColor->G = newG;
    newColor->B = newB;

    return newColor;
}



//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule) {
	Image* newImage = malloc(sizeof(Image));
	newImage->image = malloc(image->rows * sizeof(Color*));
	newImage->rows = image->rows;
	newImage->cols = image->cols;
	for (int i = 0; i < image->rows; i++) {
		newImage->image[i] = malloc(image->cols * sizeof(Color));
		for (int j = 0; j < image->cols; j++) {
			Color* newColor = evaluateOneCell(image, i, j, rule);
			newImage->image[i][j].R = newColor->R;
			newImage->image[i][j].G = newColor->G;
			newImage->image[i][j].B = newColor->B;
			free(newColor);
		}
	}
	return newImage;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv) {
	if (argc != 3) {
		printf("usage: ./gameOfLife filename rule\nfilename is an ASCII PPM file (type P3) "
		 "with maximum value 255.\nrule is a hex number beginning with 0x; Life is 0x1808.\n");
		return -1;
	}
	Image* image = readData(argv[1]);
	char *endptr;
	uint32_t rule = strtol(argv[2], &endptr, 16);

	if (*endptr != '\0' || rule < 0x00000 || rule > 0x3FFFF) {
		printf("Invalid rule. Rule should be a hex number between 0x00000 and 0x3FFFF.\n");
		return -1;
	}
	Image* newImage = life(image, rule);
	freeImage(image);
	writeData(newImage);
	freeImage(newImage);
	return 0;

}
