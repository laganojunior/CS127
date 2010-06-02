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
// This function swaps 1's around on a parity check matrix so that
// cycles of length 4 are minimized in the corresponding tanner graph
/////////////////////////////////////////////////////////////////////////
void removeShortCycles(Matrix& mat)
{
    int count = 0;
    // Go through each pair of columns
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
                // There is a cycle of length 4. Attempt to find another
                // column to swap a one out to
                // Remove ones to only allow 1 pair per pair of columns
                int keep = rand() % same.size();
                 
                for (int k = 0; k < keep; k++)
                {
                    int choice = rand() % 2;
                    mat.m[same[k]][choice ? i : j] = 0;
                    count++;
                }

                for (int k = keep + 1; k < same.size(); k++)
                {
                    int choice = rand() % 2;
                    mat.m[same[k]][choice ? i : j] = 0;
                    count++;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////
// This function guassian eliminates such that the right <height> columns
// form the identity.
//
// Returns true on success, false otherwise
//
// NOTE: Regardless of success or not, this function modifies the input
///////////////////////////////////////////////////////////////////////// 
bool guassianRightEliminate(Matrix& t)
{
    bool good = true;
    // Go through each row
    for (int row = 0; row < t.height; row++)
    {
        int col = t.width - t.height + row;

        // Find some row with a 1 in the desired column
        int row2;
        for (row2 = row; row2 < t.height &&
                         t.m[row2][col] == 0;
                         row2++);

        if (row2 == t.height)
        {
            good = false; // Unable to continue guassian elimination
            break; 
        }

        // If a row was found, swap it for this row
        if (row != row2)
        {
            vector<unsigned char> temp = t.m[row];
            t.m[row] = t.m[row2];
            t.m[row2] = temp;
        }

        // Eliminate all the ones in the column in other rows by adding
        // this row to those columns
        
        for (int r = 0; r < row; r++)
        {
            if (t.m[r][col])
            {
                for (int rowIter = 0; rowIter < t.width; rowIter++)
                {
                    t.m[r][rowIter] ^= t.m[row][rowIter];
                }
            }
        }

        for (int r = row + 1; r < t.height; r++)
        {
            if (t.m[r][col])
            {
                for (int rowIter = 0; rowIter < t.width; rowIter++)
                {
                    t.m[r][rowIter] ^= t.m[row][rowIter];
                }
            }
        }
    }

    return good;
}


////////////////////////////////////////////////////////////////////////
// Attempt to permute the columns of the input parity check matrix to
// make proper for systematic encoding. Specifically, the square matrix
// at the right (the part for the parity bits) must be invertible.
//
// Returns true if success, false otherwise.
////////////////////////////////////////////////////////////////////////
bool permuteForSystematic(Matrix& mat, int maxIters)
{
    int numIters;
    for (numIters = 0; numIters < maxIters; numIters++)
    {
        // Check if the last height columns form an invertible matrix using
        // Guassian elimination
        Matrix t = mat;
        bool good = guassianRightEliminate(t);
 
        // If the result was not found, permute the columns of the parity
        // matrix
        if (!good)
        {
            int col1 = rand() % (mat.width - mat.height);
            int col2 = (rand() % mat.height) + mat.width - mat.height;

            for (int row = 0; row < mat.height; row++)
            {
                unsigned char val = mat.m[row][col1];
                mat.m[row][col1] = mat.m[row][col2];
                mat.m[row][col2] = val;
            }
        }
        else
        {
            // Otherwise, the parity check matrix as is should be fine
            break; 
        } 
    }

    if (numIters == maxIters)
        return false;
    else
        return true;
}

////////////////////////////////////////////////////////////////////////////
// This function generates a systematic generator matrix for the given
// parity check matrix. 
//
// Returns true on success, false otherwise. The only reason for known failure
// is a failure on guassian right elimination
///////////////////////////////////////////////////////////////////////////
bool genSystematicGenerator(const Matrix& parity, Matrix& gen)
{
    Matrix t = parity;

    // Attempt to get the identity matrix at the right end
    if (!guassianRightEliminate(t))
        return false;
   
    // Set the generator to the appropriate size. Note that this zeros
    // the matrix
    int genHeight = parity.width - parity.height;
    gen.setDim(parity.width, genHeight);

    // Fill in the identity at the left side
    for (int i = 0; i < genHeight; i++)
    {
        gen.m[i][i] = 1;
    }

    // Fill in the rest as the transpose of the left side of the parity
    // check matrix
    for (int row = 0; row < genHeight; row++)
    {
        for (int col = 0; col < parity.height; col++)
        {
            gen.m[row][col + genHeight] = t.m[col][row];
        }
    }

    return true;
}
