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

    h.print(cout);
    cout << endl;

    LDPC ldpc;
    ldpc.setMatrices(g, h);
    ldpc.setEBOverN0(1.3);

    vector<unsigned char> message;
    for (int i = 0; i < g.height; i++)
        message.push_back(rand()%2);

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

    // Attempt to decode
    vector<unsigned char> decoded;
    if (ldpc.decode(code, decoded, 100000000))
    {
        cout << "Decoding Success\n";

        cout << "Decoded Message: ";
        for (int i = 0; i < decoded.size(); i++)
        {
            cout << (int)decoded[i] << " ";
        }
        cout << endl;

        bool matches = true;
        for (int i = 0; i < message.size(); i++)
        {
            if (decoded[i] != message[i])
            {
                matches = false;    
                break;
            }
        }

        if (matches)
            cout << "Yay!\n";
        else
            cout << "Aww!\n";
    }
    else
        cout << "Decoding Failure\n";

        return 0;
}
