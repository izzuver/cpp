#include <iostream>
using namespace std;

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
    unsigned int value;
    cin >> value;
    printBinary(value);
    return 0;
}