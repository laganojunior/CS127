#ifndef __CS_MATRIX__
#define __CS_MATRIX__
// A basic matrix class

#include <vector>
#include <iostream>

using namespace std;

struct Matrix
{
    vector<vector<unsigned char> > m;
    unsigned int width, height;

    // Constructor, sets the size of the matrix and sets all the entries
    // to 0
    Matrix(unsigned int width, unsigned int height)
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
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                out << (int)m[i][j] << " ";
            }
            out << endl;
        }
    }    
};


#endif
