#include <iostream>
#include <iomanip>
using namespace std;

union FloatUInt
{
    float f;
    unsigned int u;
};

void printBinary(unsigned int x)
{
    const int bits = sizeof(unsigned int) * 8;
    for (int i = bits - 1; i >= 0; --i)
    {
        unsigned int mask = 1u << i;
        cout << ((x & mask) ? '1' : '0');
    }
    cout << endl;
}

int main()
{
    cout << fixed;
    cout.precision(2);

    FloatUInt v;
    v.f = 10.0f;

    for (int i = 1; i <= 20; ++i)
    {
        cout << "10^" << i << " = " << v.f << "   ";
        printBinary(v.u);
        v.f *= 10.0f;
    }

    return 0;
}