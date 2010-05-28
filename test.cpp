#include "Matrix.h"
#include "LDPC.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{

    ifstream gin("G.txt");
    Matrix g;
    g.load(gin);
    gin.close();

    ifstream hin("H.txt");
    Matrix h;
    h.load(hin);
    hin.close();

    LDPC ldpc;
    ldpc.setMatrices(g, h);

    vector<unsigned char> message;
    message.push_back(0);
    message.push_back(0);
    message.push_back(1);
    message.push_back(0);
    message.push_back(1);
    message.push_back(0);

    vector<float> encode = ldpc.encode(message);
    for (int i = 0; i < encode.size(); i++)
    {
        cout << encode[i] << " ";
    }
    cout << endl;
    return 0;
}
