#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"
#include <cstdlib>

using namespace std;

int main()
{
    srand(10000);

    // Generate some regular parity check matrix
    Matrix h(1, 1);
    genRegParity(3, 8, 24, h);
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
        cout << "Error in systematic permute\n";

    cout << "After systematic permute\n";
    h.print(cout);
    cout << endl;

    // Generate a systematic generator matrix
    Matrix g(1, 1);
    if (!genSystematicGenerator(h, g))
        cout << "Error in systematic generation\n";
    
    cout << "After systematic generator matrix generation\n";
    cout << "H\n";
    h.print(cout);
    cout << endl;

    cout << "G\n";
    g.print(cout);
    cout << endl; 
}
