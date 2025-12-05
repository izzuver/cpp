#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

int main()
{
    ofstream f("pi_graph.csv", ios::out);
    if (!f)
    {
        cerr << "Не удалось открыть файл pi_graph.csv\n";
        return 1;
    }

    f << "n,basel,leibniz,nilakantha,bbp" << endl;

    const int maxIter = 200000; 
    const int step    = 100;    

    // Basel
    float basel_sum = 0.0f;

    // Leibniz
    float leibniz_sum  = 0.0f;
    float leibniz_sign = 1.0f;

    // Nilakantha
    float nila_sum  = 3.0f;
    float nila_sign = 1.0f;

    // BBP
    float bbp_sum    = 0.0f;
    float bbp_factor = 1.0f;

    for (int n = 1; n <= maxIter; ++n)
    {
        // Basel: π ≈ sqrt(6 * Σ 1/n^2)
        basel_sum += 1.0f / (n * n);
        float pi_basel = sqrt(6.0f * basel_sum);

        // Leibniz: π ≈ 4 * Σ (-1)^k / (2k+1), k = 0..n-1
        leibniz_sum += leibniz_sign / (2 * n - 1);
        leibniz_sign = -leibniz_sign;
        float pi_leibniz = 4.0f * leibniz_sum;

        // Nilakantha: π ≈ 3 + Σ (-1)^{n+1} * 4 / [(2n)(2n+1)(2n+2)]
        float a = 2.0f * n;
        float term_nila = 4.0f / (a * (a + 1.0f) * (a + 2.0f));
        nila_sum += nila_sign * term_nila;
        nila_sign = -nila_sign;
        float pi_nila = nila_sum;

        // BBP: π ≈ Σ 1/16^k * (4/(8k+1) - 2/(8k+4) - 1/(8k+5) - 1/(8k+6))
        int k = n - 1;
        float term_bbp = 4.0f / (8 * k + 1)
                       - 2.0f / (8 * k + 4)
                       - 1.0f / (8 * k + 5)
                       - 1.0f / (8 * k + 6);
        bbp_sum += bbp_factor * term_bbp;
        float pi_bbp = bbp_sum;
        bbp_factor /= 16.0f;

        if (n % step == 0)
        {
            f << n << ","
              << pi_basel << ","
              << pi_leibniz << ","
              << pi_nila << ","
              << pi_bbp << endl;
        }
    }

    return 0;
}