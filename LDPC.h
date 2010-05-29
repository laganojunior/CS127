#ifndef __CS_LDPC__
#define __CS_LDPC__

#include <vector>
#include "Matrix.h"

using namespace std;

// This class implements the LPDC encoding/decoding
struct LDPC
{
    void setMatrices(const Matrix& g, const Matrix& h);
    void setEBOverN0(float EBOverN0);

    vector<float> encode(const vector<unsigned char>& message);

    void addAWGN(vector<float>& code);

    // Generator and Parity check matrices
    Matrix g;
    Matrix h;

    // Index for each check node to which message nodes it connects to in
    // the tanner graph
    vector<vector<int> > checkNodeEdges;

    // Parameter for noise on the channel
    float EBOverN0;
};


#endif
