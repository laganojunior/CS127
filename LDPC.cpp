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
    cout << "Original Intrs: ";
    vector<float> constVarVals;
    constVarVals.resize(received.size());
    for (int i = 0; i < received.size(); i++)
    {
        constVarVals[i] = 2.0 / var * received[i];
        cout << constVarVals[i] << " ";
    }
    cout << endl;

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

    // Storage for total lambda calculations
    vector<float> variableLambdaSum;
    vector<float> oldVariableLambdaSum;
    variableLambdaSum.resize(variableMessages.size());
    oldVariableLambdaSum.resize(variableMessages.size());
    oldVariableLambdaSum.assign(constVarVals.begin(),
                                constVarVals.end());

    int numIters; 
    bool earlyBreak = false;
    for (numIters = 0; numIters < maxIters; numIters++)
    {
        
        // Calculate all check node messages
        for (int checkNodeI = 0; checkNodeI < checkNodeEdges.size();
                                 checkNodeI++)
        {

            // Figure out the related arctangents of all variable node
            // messages connected to this node
            float prod = 1.0;
            for (int i = 0; i < checkNodeEdges[checkNodeI].size(); i++)
            {
                pair<int, int> p = checkNodeEdges[checkNodeI][i];
                int varI        = p.first;
                int varMessageI = p.second;

            //    cout << "Received: " << checkNodeI << " " << i << " "
            //         << variableMessages[varI][varMessageI] << endl;

                variableValTanh[i] = tanh(variableMessages[varI][varMessageI]
                                          / 2.0);

                prod *= variableValTanh[i];
            }

//            cout << "Check Prod: " << checkNodeI << " " << prod << endl;

            // Send out messages to each variable node, removing their own
            // contribution to the product
            for (int i = 0; i < checkNodeEdges[checkNodeI].size(); i++)
            {
                checkMessages[checkNodeI][i] = 2.0 * atanh(
                                               prod / variableValTanh[i]);
 //               cout << "Check Message " << checkNodeI << " " << i << ": "
 //                    << checkMessages[checkNodeI][i] << endl; 
            } 
        }
       
        
        // Figure out all variable node messages
        for (int varI = 0; varI < variableNodeEdges.size(); varI++)
        {
            // Sum up the total lambdas, which is the intrinsic value plus
            // incoming messages
            variableLambdaSum[varI] = constVarVals[varI];

            for (int i = 0; i < variableNodeEdges[varI].size(); i++)
            {
                pair<int, int> p = variableNodeEdges[varI][i];
                int checkI        = p.first;
                int checkMessageI = p.second;

                variableLambdaSum[varI] += checkMessages[checkI][checkMessageI];
            //    cout << "Var received " << varI << " " << i << " "
            //         << checkMessages[checkI][checkMessageI] << endl;
            }

            // Now send messages to each connected check node, removing its
            // contribution to the sums
            for (int i = 0; i < variableNodeEdges[varI].size(); i++)
            {
                pair<int, int> p = variableNodeEdges[varI][i];
                int checkI        = p.first;
                int checkMessageI = p.second;
            
                variableMessages[varI][i] 
                      = variableLambdaSum[varI]
                      - checkMessages[checkI][checkMessageI];

        //        cout << "Variable Message " << varI << " " << i << ": "
        //            << variableMessages[varI][i] << endl; 
            }
        }

        // Make a tentative decision
        vector<unsigned char> tempWord;
       // cout << "Temp is: ";
        tempWord.resize(variableLambdaSum.size());
        for (int i = 0; i < variableLambdaSum.size(); i++)
        {
            if (variableLambdaSum[i] < 0)
                tempWord[i] = 1;
            else
                tempWord[i] = 0;
    
     //       cout << (int)tempWord[i] << " " << variableLambdaSum[i] << endl;
        }
      //  cout << endl;

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
        if (found)
        {
            out = tempWord;
            break;
        }
        
        cout << "Iter " << numIters << ": " << endl;
     /*   // Print the values of the current iteration
        for (int i = 0; i < variableLambdaSum.size(); i++)
        {
            cout << variableLambdaSum[i] << " ";
        } 

        cout << endl; */
        
        // Check if the lambdas changed
        float dist = 0;
        for (int i = 0; i < variableLambdaSum.size(); i++)
        {
            float diff = variableLambdaSum[i] - oldVariableLambdaSum[i];
            dist += diff * diff;
        }

        if (dist < .01)
            return false;

        oldVariableLambdaSum.assign(variableLambdaSum.begin(),
                                    variableLambdaSum.end());
    }
    
    cout << "In: " << numIters << endl;
    return (numIters != maxIters); // Early break means a codeword was detected
}
