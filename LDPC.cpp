#include "LDPC.h"
#include <cstdlib>
#include <math.h>

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
// Set the EB/N0 parameter that determines the variance of the channel
/////////////////////////////////////////////////////////////////////
void LDPC :: setEBOverN0(float EBOverN0)
{
    this->EBOverN0 = EBOverN0;
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

//////////////////////////////////////////////////////////////////////
// Use box-muller transform for gaussian sampling
//////////////////////////////////////////////////////////////////////
float gaussSample(float mean, float var)
{
    float U1 = ((float)rand()) / RAND_MAX;
    float U2 = ((float)rand()) / RAND_MAX;

    return mean + sqrt(var) * sqrt(-2.0 * log(U1)) * cos(2 * M_PI * U2);
}

//////////////////////////////////////////////////////////////////////
// Add some additive white guassian noise to some codeword
//////////////////////////////////////////////////////////////////////
void LDPC :: addAWGN(vector<float>& code)
{
    float rate = ((float)g.width) / g.height;
    float var  = 1.0 / (2 * rate * EBOverN0);

    for (int i = 0; i < code.size(); i++)
    {
        code[i] = gaussSample(code[i], var);
    }

}
