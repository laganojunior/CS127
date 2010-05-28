#ifndef __CS_MATRIX__
#define __CS_MATRIX__
// A basic matrix class

#include <vector>
#include <iostream>
#include <assert.h>

using namespace std;

struct Matrix
{
    vector<vector<unsigned char> > m;
    unsigned int width, height;

    Matrix() {}

    // Constructor, sets the size of the matrix and sets all the entries
    // to 0
    Matrix(unsigned int width, unsigned int height)
    {
        setDim(width, height); 
    }

    // Function to set width and height, and setting all elements to 0
    void setDim(unsigned int width, unsigned int height)
    {
        this->width = width;
        this->height = height;    
        
        m.resize(height);
        
        for (int i = 0; i < height; i++)
        {
            m[i].resize(width);
            for (int j = 0; j < width; j++)
                m[i][j] = 0;
        }
    }

    // Print the matrix to some output stream
    void print(ostream& out)
    {
        out << height << " " << width << endl;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                out << (int)m[i][j] << " ";
            }
            out << endl;
        }
    } 

    // Load from an input stream
    void load(istream& in)
    {
        in >> height >> width;

        setDim(width, height);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int val;
                in >> val;
                m[i][j] = val;
            }
        }
    }

    // Return the result of a binary matrix multiply
    Matrix binaryMult(const Matrix& mat)
    {
        assert(width == mat.height);

        Matrix prod(mat.width, height);

        for (int i = 0; i < height; i ++)
        {
            for (int j = 0; j < mat.width; j++)
            {
                int sum = 0;
                for (int k = 0; k < mat.height; k++)
                {
                    // Product in binary is and, addition in binary is xor
                    sum ^= m[i][k] & mat.m[k][j];
                }
                prod.m[i][j] = sum;
            }
        }  
        return prod;
    }

    // Transpose this matrix
    void transpose()
    {
        vector<vector< unsigned char> > old = m;
        setDim(height, width);

        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                m[row][col] = old[col][row];
            }
        }
    } 

    // Check if all the entries are 0
    bool isZero()
    {

        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                if (m[row][col])
                    return false;
            }
        }

        return true;
    }

};


#endif
