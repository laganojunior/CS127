#include "Matrix.h"
#include "LDPC.h"
#include "WeightDist.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>

using namespace std;

WeightDist dist;

//////////////////////////////////////////////////////////////////////////
// Estimate the mean of errors at some EB/N0 point such that the standard
// deviation on the estimated error rate is no more than 10% of the
// estimate.
//
// Returns true if successful. Returns false if the condition is not
// met when the number of tests reaches the maximum desired.
//////////////////////////////////////////////////////////////////////////
bool testPoint(float EBOverN0, LDPC& ldpc, int maxTests,
               int maxItersPerTest, double& errorRate, double& symErrorRate)
{
    int numErrors = 0;
    unsigned int bitTests = 0;
    unsigned int messageBitErrors = 0;

    // Set the energy ratio
    ldpc.setEBOverN0(EBOverN0);
    for (int testI = 1; testI < maxTests; testI++)
    {
        // Create some random message. Note that internally LDPC transposes
        // the G matrix, so the message length is the width of G
        vector<unsigned char> message;
        for (int i = 0; i < ldpc.g.width; i++)
        {
            message.push_back(rand()%2);
        }

        // Encode the message 
        vector<unsigned char> codeBits = ldpc.encode(message);
       
        // Store the weight information of the message
        dist.addCodeword(codeBits);
       
        // Transform to soft floating bits 
        vector<float> code = ldpc.floatTransform(codeBits);
        
        // Add some noise
        ldpc.addAWGN(code);
        
        // Attempt to decode
        vector<unsigned char> decoded;
        if (ldpc.decode(code, decoded, maxItersPerTest))
        {
            // Check if the decoded codeword is the one sent on the channel
            // and count the wrong bits in the message payload
            bool matches = true;
            for (int i = 0; i < message.size(); i++)
            {
                if (decoded[i] != message[i])
                {
                    matches = false;    
                    break;
                }
            }

            if (!matches)
            {
                numErrors++;
            }
        }
        else
        {
            // Count a decoding failure the same as undetected error
            // codeword 
            numErrors++; 
        } 

        // Count up the wrong bits in case the decoder decides to use the
        // codeword regardless
        bitTests += ldpc.g.width;
        for (int i = 0; i < ldpc.g.width; i++)
        {
            if (decoded[i] != message[i])
                messageBitErrors++;
        }

        // Make the estimates on the error rate and see if the termination
        // condition succeeded
        errorRate = ((double) numErrors)   / testI;
        symErrorRate = ((double) messageBitErrors) / bitTests;

        if (testI % 10000 == 0)
            cout << testI << " " << numErrors << " "
                 << errorRate << " " << symErrorRate << " " << endl;

        if (numErrors >= 100)
        {
            cout << "Ended in: " << testI << endl;
            cerr << "DMin: " << dist.currMinDist
                 << " K: " << dist.minCodes.size() << endl;
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Output some plot points linearly on the EB/N0 scale to some output stream
/////////////////////////////////////////////////////////////////////////////
void testLinear(LDPC& ldpc, ostream& out, float min, float max, float step,
                int maxTests, int maxIters)
{
    double curr = min;

    dist.init(ldpc.g.height);

    while (curr < max)
    {
        double errorRate;
        double symErrorRate;
        bool success = testPoint(curr, ldpc, maxTests, maxIters,
                                 errorRate, symErrorRate);

        cout << curr << " " << errorRate << " " << symErrorRate << " ";
        out << curr << " " << errorRate << " " << symErrorRate << " ";

        if (!success)
        {
            cout << " FAILURE\n";
            out << " FAILURE\n";
        }
        else
        {
            cout << endl;
            out << endl;
        }

        curr += step;

        // Dump some stats on the weight distribution
        dist.dumpStats();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Output some plot points multiplicatively on the EB/N0 scale (such that in
// a log-log plot, the points are linearly sampled) to some output stream
//////////////////////////////////////////////////////////////////////////////
void testLogLinear(LDPC& ldpc, ostream& out, float min, float max, float step,
                   int maxTests, int maxIters)
{
    double curr = min;

    dist.init(ldpc.g.height);

    while (curr < max)
    {
        double errorRate;
        double symErrorRate;
        bool success = testPoint(curr, ldpc, maxTests, maxIters,
                                 errorRate, symErrorRate);

        cout << curr << " " << errorRate << " " << symErrorRate << " ";
        out << curr << " " << errorRate << " " << symErrorRate << " ";

        if (!success)
        {
            cout << " FAILURE\n";
            out << " FAILURE\n";
        }
        else
        {
            cout << endl;
            out << endl;
        }

        curr *= step;

        // Dump some stats on the weight distribution
        dist.dumpStats();
    }
}


int main(int argc, char ** argv)
{
    if (argc != 10)
    {
        cout << "Usage: test gFilename hFilename out dist ...\n"
             << "               min max mult maxIters maxIterDecode\n\n"
             << "\tGenerates a series of plot points of errors\n\n";

        cout << "\thFilename, gFilename - the files to read the parity\n"
             << "\t                       check matrix and generator matrix\n"
             << "\t                       to respectively\n\n";

        cout << "\tout   - the file to write results to. The lines are:\n"
                "\t        EB/NO wordErrorRate symbolErrorRate\n"; 
        cout << "\tdist  - the file to write codeword distribution results to.\n";

        cout << "\tmin, max - The minimum/maximum values of EB/NO to try\n";
        cout << "\tmult - the amount to multiplicatively step. This makes\n"
                "\t       uniform step in a log scale plot\n\n";

        cout << "\tmaxIters - the max number of iterations to do per\n"
                "\t           plotpoint\n";
        cout << "\tmaxIterDecode - the max number of iterations to do per\n"
                "\t                decoding\n";
        exit(0) ;
    }

    int seed = time(0); 
    srand(seed);

    string gFilename = argv[1];
    string hFilename = argv[2];
    string outFilename = argv[3];
    string distFilename = argv[4];
    double min = atof(argv[5]);
    double max = atof(argv[6]);
    double mult = atof(argv[7]);
    int maxIters = atoi(argv[8]);
    int maxIterDecode = atoi(argv[9]);

    cout << "Seed: " << seed << endl;
    cout << "Loading G\n";
    cout.flush();
    ifstream gin(gFilename.c_str());
    Matrix g;
    g.load(gin);
    gin.close();

    cout << "Loading H\n";
    cout.flush();
    ifstream hin(hFilename.c_str());
    Matrix h;
    h.load(hin);
    hin.close();

    LDPC ldpc;
    ldpc.setMatrices(g, h);
  
    cout << "Starting\n"; 
    ofstream out(outFilename.c_str()); 
    ofstream statDump(distFilename.c_str());
    dist.setDumpStream(statDump);
    testLogLinear(ldpc, out, min, max, mult, maxIters, maxIterDecode);

    statDump.close(); 
    out.close(); 
}
