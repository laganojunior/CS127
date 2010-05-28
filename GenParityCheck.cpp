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
        for (int col = 0; col < width; col++)
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

/////////////////////////////////////////////////////////////////////////
// This function eliminates 1's from a parity check matrix so that
// cycles of length 4 are eliminated in the corresponding tanner graph
/////////////////////////////////////////////////////////////////////////
void removeShortCycles(Matrix& mat)
{
    for (int i = 0; i < mat.width; i++)
    {
        for (int j = i + 1; j < mat.width; j++)
        {
            vector<int> same;
            for (int k = 0; k < mat.height; k++)
                if (mat.m[k][i] & mat.m[k][j])
                    same.push_back(k);
            
            if (same.size() > 1)
            {
                // Remove ones to only allow 1 pair per pair of columns
                int keep = rand() % same.size();
                 
                for (int k = 0; k < keep; k++)
                {
                    int choice = rand() % 2;
                    mat.m[same[k]][choice ? i : j] = 0;
                }

                for (int k = keep + 1; k < same.size(); k++)
                {
                    int choice = rand() % 2;
                    mat.m[same[k]][choice ? i : j] = 0;
                }
            }
        }
    }

    mat.height = mat.m.size();
}