#include <iostream>
#include <iomanip>
using namespace std;

int main()
{
    cout << fixed;

    float start = 1e38f;
    float end = start + 1000.0f;

    if (start == start + 1.0f)
        cout << "!" << endl;

    float x = start;
    int steps = 0;

    while (x < end)
    {
        float old = x;
        x += 1.0f;
        ++steps;

        if (x == old)
        {
            break;
        }
    }
}