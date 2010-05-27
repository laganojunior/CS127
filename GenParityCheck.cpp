#include <vector>
#include <assert.h>
#include <cstdlib>
#include "Matrix.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
// This function makes a random parity check matrix that has
// wc 1's in every column, wr 1's in every row, with the
// respective width (the height is then wc * width / wr).
//
// This function needs wr | width
///////////////////////////////////////////////////////////////////
void genRegParity(unsigned int wc, unsigned int wr,
                  unsigned int width, Matrix& mat)
{
    // Make sure the constraints on the inputs are satisfied
    assert(width % wr == 0);

    // Resize the matrix. Note this also zeros out the matrix
    int height = wc * width / wr;
    mat.setDim(width, height);

    // Separate the rows in wc parts, and in these parts only
    // allow 1 one per column. Clearly this achieves wc ones in
    // every column in the whole matrix
    for (int rowPart = 0; rowPart < wc; rowPart++)
    {
        // For each part, decide where the 1 goes for each column.
        // Limit to wr 1's per row 
        int numRows = width / wr;
        vector<unsigned int> rowCount;
        rowCount.resize(numRows);
        for (int i = 0; i < numRows; i++)
            rowCount[i] = 0;

        // Fill out the ones
        for (int col = (rowPart + 1) * numRows ; col < width; col++)
        {
            // Choose a random row to place the one.
            int choice = rand() % numRows;
            while (rowCount[choice] >= wr)
                choice = rand() % numRows;

            mat.m[rowPart * numRows + choice][col] = 1;
            rowCount[choice]++;
        }
    } 
}
