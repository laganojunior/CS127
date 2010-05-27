#include <iostream>
#include "Matrix.h"
#include "GenParityCheck.h"

using namespace std;

int main()
{
    Matrix mat(4, 3);
    mat.print(cout);

    genRegLDPCEasy(3, 10, 20, mat);
    mat.print(cout);
}
