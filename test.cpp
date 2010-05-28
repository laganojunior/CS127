#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"
#include <cstdlib>

using namespace std;

int main()
{
    srand(time(0));

    // Generate some regular parity check matrix
    Matrix h(1, 1);
    genRegParity(3, 4, 16, h);
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
    if (!permuteForSystematic(h, 20))
    {
        cout << "Error in systematic permute\n";
        exit(1);
    }

    cout << "After systematic permute\n";
    h.print(cout);
    cout << endl;

    // Generate a systematic generator matrix
    Matrix g(1, 1);
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

}
