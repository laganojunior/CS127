#include "Matrix.h"
#include "LDPC.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

int main()
{
    srand(time(0));

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
    ldpc.setEBOverN0(2.0);

    vector<unsigned char> message;
    message.push_back(0);
    message.push_back(0);
    message.push_back(1);
    message.push_back(0);
    message.push_back(1);
    message.push_back(0);

    cout << "Original Message: ";
    for (int i = 0; i < message.size(); i++)
    {
        cout << (int)message[i] << " ";
    }
    cout << endl;

    // Encode the message 
    vector<float> code = ldpc.encode(message);
    cout << "Encoded Message: ";
    for (int i = 0; i < code.size(); i++)
    {
        cout << code[i] << " ";
    }
    cout << endl;

    // Add some noise
    ldpc.addAWGN(code);
    cout << "Sent Message: ";
    for (int i = 0; i < code.size(); i++)
    {
        cout << code[i] << " ";
    }
    cout << endl;

    return 0;
}
