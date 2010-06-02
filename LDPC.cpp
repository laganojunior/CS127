#include "LDPC.h"
#include <cstdlib>
#include <math.h>

#undef DEBUG_OUT
#undef USE_MIN_RULE

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
    variableNodeEdges.resize(h.width);
    for (int row = 0; row < h.height; row++)
    {
        checkNodeEdges[row].clear();

        for (int col = 0; col < h.width; col++)
        {
            if (h.m[row][col])
            {
                checkNodeEdges[row].push_back(pair<int, int>(col, 0));

                variableNodeEdges[col].push_back(pair<int, int>
                                       (row, checkNodeEdges[row].size() - 1));

                checkNodeEdges[row].back().second
                    = variableNodeEdges[col].size() - 1;
            }
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
// Encode a message
//////////////////////////////////////////////////////////////////////
vector<unsigned char> LDPC :: encode(const vector<unsigned char>& message)
{
    // Multiply the message by the generator matrix
    Matrix prod = g.binaryMult(message);

    vector<unsigned char> res;

    res.resize(prod.height);

    for (int i = 0; i < prod.height; i++)    
        res[i] = prod.m[i][0];

    return res;
}

/////////////////////////////////////////////////////////////////////
// Transform a set of bits into soft floating point bits
/////////////////////////////////////////////////////////////////////
vector<float> LDPC :: floatTransform(const vector<unsigned char>& message)
{
    vector<float> res;

    res.resize(message.size());

    for (int i = 0; i < message.size(); i++)
    {
        if (message[i])
            res[i] = -1;
        else
            res[i] = 1;
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

//////////////////////////////////////////////////////////////////////
// Attempt to decode a soft vector into the original hard codeword it
// came from. 
//
// Returns true on success in which case the decoded message is written
// onto the "out" input. Otherwise, this returns false.
//////////////////////////////////////////////////////////////////////
bool LDPC :: decode(const vector<float>& received,
                    vector<unsigned char>& out,
                    int maxIters)
{
    float rate = ((float)g.width) / g.height;
    float var  = 1.0 / (2 * rate * EBOverN0);

    // Initialize the values of the "intrinsic" values of the variable nodes.
    // These do not change per iteration
    #ifdef DEBUG_OUT
    cout << "Original Intrs: ";
    #endif
    vector<float> constVarVals;
    constVarVals.resize(received.size());
    for (int i = 0; i < received.size(); i++)
    {
        constVarVals[i] = 2.0 / var * received[i];

        #ifdef DEBUG_OUT
        cout << constVarVals[i] << " ";
        #endif
    }

    #ifdef DEBUG_OUT
    cout << endl;
    #endif

    // Initialize the values of the messages sent by variable nodes to the
    // intrinsic values
    vector<vector<float> > variableMessages;
    variableMessages.resize(variableNodeEdges.size());
    for (int i = 0; i < variableNodeEdges.size(); i++)
    {
        variableMessages[i].resize(variableNodeEdges[i].size());
        variableMessages[i].assign(variableNodeEdges[i].size(),
                                   constVarVals[i]);
    }

    // Make a vector for each check node sending messages to the variable
    // nodes, with initial values at 0.
    vector<vector<float> > checkMessages;
    checkMessages.resize(checkNodeEdges.size());
    for (int i = 0; i < checkNodeEdges.size(); i++)
    {
        checkMessages[i].resize(checkNodeEdges[i].size());
        checkMessages[i].assign(checkNodeEdges[i].size(), 0);
    }
   
    // Keep some storage for the argtangent calculations 
    vector<float> variableValTanh;
    variableValTanh.resize(variableMessages.size());
   
    // Allocate space for the temporary decision 
    vector<unsigned char> tempWord;
    tempWord.resize(variableMessages.size());

    int numIters; 
    for (numIters = 0; numIters < maxIters; numIters++)
    {
        // Calculate all check node messages
        for (int checkNodeI = 0; checkNodeI < checkNodeEdges.size();
                                 checkNodeI++)
        {

            // Figure out the related arctangents of all variable node
            // messages connected to this node
            for (int i = 0; i < checkNodeEdges[checkNodeI].size(); i++)
            {
                pair<int, int> p = checkNodeEdges[checkNodeI][i];
                int varI        = p.first;
                int varMessageI = p.second;

                #ifdef DEBUG_OUT
                cout << "Received: " << checkNodeI << " " << i << " "
                     << variableMessages[varI][varMessageI] << endl;
                #endif

                variableValTanh[i] = tanh(variableMessages[varI][varMessageI]
                                          / 2.0);
            }
            
            // Send out messages to each variable node, removing their own
            // contribution to the product
            
            for (int i = 0; i < checkNodeEdges[checkNodeI].size(); i++)
            {
                #ifdef USE_MIN_RULE
                float min = 100000.0;
                int sign = 1;
                for (int j = 0; j < i; j++)
                {
                    
                    pair<int, int> p = checkNodeEdges[checkNodeI][j];
                    int varI        = p.first;
                    int varMessageI = p.second;
                   
                    float m = variableMessages[varI][varMessageI];
                    #ifdef DEBUG_OUT 
                    cout << "Received: " << m << endl;
                    #endif
                    if (fabs(m) < min)
                    {
                        min = fabs(m);
                    }

                    if (m < 0)
                        sign *= -1;
                }

                for (int j = i + 1; j < checkNodeEdges[checkNodeI].size(); j++)
                {
                    
                    pair<int, int> p = checkNodeEdges[checkNodeI][j];
                    int varI        = p.first;
                    int varMessageI = p.second;
                    
                    float m = variableMessages[varI][varMessageI];
                    #ifdef DEBUG_OUT 
                    cout << "Received: " << m << endl;
                    #endif
                    if (fabs(m) < min)
                    {
                        min = fabs(m);
                    }

                    if (m < 0)
                        sign *= -1;
                } 

                checkMessages[checkNodeI][i] = sign * min;
                
                #else

                float prod = 1.0;

                for (int j = 0; j < i; j++)
                {
                    prod *= variableValTanh[j];
                }

                for (int j = i + 1; j < checkNodeEdges[checkNodeI].size(); j++)
                {
                    prod *= variableValTanh[j];
                }
               
                checkMessages[checkNodeI][i] = 2.0 * atanh(prod);

                #endif
                                   
                #ifdef DEBUG_OUT
                cout << "Check Message " << checkNodeI << " " << i << ": "
                     << checkMessages[checkNodeI][i] << endl; 
                #endif 
            }
        }
       
        
        // Figure out all variable node messages
        for (int varI = 0; varI < variableNodeEdges.size(); varI++)
        {
            // Go over the nodes to send to
            for (int i = 0; i < variableNodeEdges[varI].size(); i++)
            {
                // Add up the contributions from every other node connected
                float sum = constVarVals[varI];

                for (int j = 0; j < i; j++)
                {
                    pair<int, int> p = variableNodeEdges[varI][j];
                    int checkI        = p.first;
                    int checkMessageI = p.second;

                    sum += checkMessages[checkI][checkMessageI];

                    #ifdef DEBUG_OUT
                    cout << "Var received " << varI << " " << j << " "
                         << checkMessages[checkI][checkMessageI] << endl;
                    #endif

                }

                for (int j = i + 1; j < variableNodeEdges[varI].size(); j++)
                {
                    pair<int, int> p = variableNodeEdges[varI][j];
                    int checkI        = p.first;
                    int checkMessageI = p.second;

                    sum += checkMessages[checkI][checkMessageI];

                    #ifdef DEBUG_OUT
                    cout << "Var received " << varI << " " << j << " "
                         << checkMessages[checkI][checkMessageI] << endl;
                    #endif
                }

                variableMessages[varI][i] = sum;
                #ifdef DEBUG_OUT
                cout << "Var message " << varI << " " << i << " "
                     << sum << endl;
                #endif
            }
        }

        // Make a tentative decision

        #ifdef DEBUG_OUT
        cout << "Temp is: ";
        #endif
        for (int varI = 0; varI < variableNodeEdges.size(); varI++)
        {
            float sum = constVarVals[varI];

            for (int i = 0; i < variableNodeEdges[varI].size(); i++)
            {
                pair<int, int> p = variableNodeEdges[varI][i];
                int checkI        = p.first;
                int checkMessageI = p.second;

                sum += checkMessages[checkI][checkMessageI];
            }
   
            if (sum < 0) 
                tempWord[varI] = 1;
            else
                tempWord[varI] = 0;

            #ifdef DEBUG_OUT
            cout << (int)tempWord[varI] << " " << sum << endl;
            #endif
        }
        #ifdef DEBUG_OUT
        cout << endl;
        #endif

        // Check to see if the decision is a codeword by checking
        // each parity constraint
        bool found = true;
        for (int checkNodeI = 0; checkNodeI < checkNodeEdges.size();
                                 checkNodeI++)
        {
            int sum = 0;
            for (int i = 0; i < checkNodeEdges[checkNodeI].size(); i++)
            {
                sum ^= tempWord[checkNodeEdges[checkNodeI][i].first];
            }

            if (sum != 0) // This parity constraint failed
            {
                found = false;  
                break;
            }
        }
         
        // If all the parity check constraints are satisfied, terminate
        // early and return the found codeword 

        #ifdef DEBUG_OUT
        cout << "Done Iter " << numIters << endl;
        #endif
        out = tempWord;
        if (found)
            break;

        
    }
    #ifdef DEBUG_OUT  
    cout << "In: " << numIters << endl;
    #endif
    return (numIters != maxIters); // Early break means a codeword was detected
}
