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

    vector<float> floatTransform(const vector<unsigned char>& message);
    vector<unsigned char> encode(const vector<unsigned char>& message);

    void addAWGN(vector<float>& code);

    bool decode(const vector<float>& received,
                vector<unsigned char>& out,
                int maxIters);

    // Generator and Parity check matrices
    Matrix g;
    Matrix h;

    // Vector mapping each variable node to a vector of pairs noting:
    // - the index of a check node it is connected to
    // - the index of the variable node in that check node's lis of connected
    // - variable nodes
    vector<vector<pair<int, int> > > variableNodeEdges;

    // A similar vector for check node edges
    vector<vector<pair<int, int> > > checkNodeEdges;

    // Parameter for noise on the channel
    float EBOverN0;
};


#endif
