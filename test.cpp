#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"

using namespace std;

int main()
{
    Matrix mat(4, 3);
    mat.print(cout);

    genRegParity(3, 4, 12, mat);
    mat.print(cout);

    Matrix mat2(1, 12);
    mat2.m[1][0] = 1;
    mat2.m[5][0] = 1;
    mat2.m[7][0] = 1;
    mat2.m[9][0] = 1;
    mat2.m[11][0] = 1;
    mat2.print(cout);

    cout << endl;

    Matrix prod = mat.binaryMult(mat2);
    prod.print(cout);
}
