#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"
#include <cstdlib>

using namespace std;

int main()
{
    srand(10000);
    Matrix mat(4, 3);
    genRegParity(3, 8, 24, mat);
    cout << "Original Parity\n";
    mat.print(cout);
    
    cout << endl;

    removeShortCycles(mat); 
    cout << "After Short Cycles\n";
    mat.print(cout);
    cout << endl;

    if (!permuteForSystematic(mat, 20))
        cout << "Error in systematic permute\n";

    cout << "After systematic permute\n";
    mat.print(cout);
    cout << endl;
}
