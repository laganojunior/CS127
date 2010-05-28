#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"
#include <cstdlib>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
    if (argc != 6)
    {
        cout << "Usage: autogenmat gFilename hFilename wc wr N\n"
             << "\tGenerates a pair of LDPC parity check matrix and a\n"
             << "\tcorresponding systematic generator matrix\n\n";

        cout << "\thFilename, gFilename - the files to write the parity\n"
             << "\tcheck matrix and generator matrix to respectively\n\n";

        cout << "\twc, wr - The maximum number of ones per column and row\n"
             << "\tresp.\n\n";

        cout << "\tN is the width of the parity check matrix. The height is\n"
             << "\tN * wr / wc (as the parity check matrix is generated from\n"
             << "\ta regular matrix)\n"; 

        exit(0) ;
    } 

    string gFilename = argv[1];
    string hFilename = argv[2];
    int wc = atoi(argv[3]);
    int wr = atoi(argv[4]);
    int  N = atoi(argv[5]);
        
    srand(time(0));

    // Generate some regular parity check matrix
    Matrix h;
    genRegParity(wc, wr, N, h);
    cout << "Original Parity\n";
    h.print(cout);
    cout << endl;

    // Remove cycles of length 4
    removeShortCycles(h); 
    cout << "After Short Cycles\n";
    h.print(cout);
    cout << endl;

    // Permute the columns of the parity check to enable an easy
    // derivation of a systematic encoder
    if (!permuteForSystematic(h, h.width * 2))
    {
        cout << "Error in systematic permute\n";
        exit(1);
    }

    cout << "After systematic permute\n";
    h.print(cout);
    cout << endl;

    // Generate a systematic generator matrix
    Matrix g;
    if (!genSystematicGenerator(h, g))
    {
        cout << "Error in systematic generation\n";
        exit(1);
    }
    
    cout << "After systematic generator matrix generation\n";
    cout << "H\n";
    h.print(cout);
    cout << endl;

    cout << "G\n";
    g.print(cout);
    cout << endl; 

    // Test if g is a valid generator matrix
    Matrix tH = h; tH.transpose();
    Matrix prod = g.binaryMult(tH);

    if (!prod.isZero())
    {
        cout << "G is not a valid generator!\n";
        cout << "GH ^ T is\n";
        prod.print(cout);
        cout << endl;
    }

    // Save the matrices
    ofstream gout(gFilename.c_str());
    g.print(gout);
    gout.close();

    ofstream hout(hFilename.c_str());
    h.print(hout);
    hout.close();
    return 0;
}
