#include <iostream>
using namespace std;

void printBinary(unsigned int x)
{
    const int bits = sizeof(unsigned int) * 8;
    for (int i = bits - 1; i >= 0; --i)
    {
        unsigned int mask = 1u << i;
        cout << ((x & mask) ? '1' : '0');
        if (i == 31 || i == 23)
            cout << ' ';
    }
    cout << endl;
}

union FloatUInt
{
    float f;
    unsigned int u;
};

int main()
{
    FloatUInt x;
    cin >> x.f;
    printBinary(x.u);
    return 0;
}