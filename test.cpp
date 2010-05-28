#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"
#include <cstdlib>

using namespace std;

int main()
{
    srand(10000);
    Matrix mat(4, 3);
    genRegParity(3, 4, 16, mat);
    cout << "Original Parity\n";
    mat.print(cout);
    
    cout << endl;

    removeShortCycles(mat); 
    cout << "After Short Cycles\n";
    mat.print(cout);
    cout << endl;
}
