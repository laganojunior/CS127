#include "LDPC.h"

///////////////////////////////////////////////////////////////////////////
// Sets the generator matrix (g) and parity check matrix (h). This function
// does not try to assert that h is a parity check matrix for g.
///////////////////////////////////////////////////////////////////////////
void LDPC :: setMatrices(const Matrix& g, const Matrix& h)
{

    this->g = g;
    this->g.transpose();

    this->h = h;

    // Set up the tanner graph for h
    checkNodeEdges.resize(h.height); 

    for (int row = 0; row < h.height; row++)
    {
        checkNodeEdges[row].clear();

        for (int col = 0; col < h.width; col++)
        {
            if (h.m[row][col])
                checkNodeEdges[row].push_back(col);
        } 
    }
}

//////////////////////////////////////////////////////////////////////
// Encode a message and transform it into soft bits
//////////////////////////////////////////////////////////////////////
vector<float> LDPC :: encode(const vector<unsigned char>& message)
{
    // Multiply the message by the generator matrix
    Matrix prod = g.binaryMult(message);

    // Transform each of the bits to -1, 1
    vector<float> res;
    res.resize(prod.height);

    for (int i = 0; i < prod.height; i++)    
    {
        if (prod.m[i][0])
            res[i] = -1.0;
        else
            res[i] = 1.0;
    }

    return res;
}
