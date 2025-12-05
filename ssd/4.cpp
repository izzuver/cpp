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

    const int maxIter = 200000;
    const int step    = 100;

    //Базель
    float basel_sum = 0.0f;

    //Уоллис
    float wallis_prod = 1.0f;

    //Виета
    float vieta_a = sqrt(0.5f);
    float vieta_prod = vieta_a;

    //Мачин
    // pi = 16*atan(1/5) - 4*atan(1/239)
    float m_sum1  = 0.0f;
    float m_pow1  = 1.0f / 5.0f;    // x^(2k+1) для x = 1/5
    float m_sign1 = 1.0f;

    float m_sum2  = 0.0f;
    float m_pow2  = 1.0f / 239.0f;  // x^(2k+1) для x = 1/239
    float m_sign2 = 1.0f;

    //Рамануджан
    const float ram_factor = 2.0f * sqrt(2.0f) / 9801.0f;
    const float inv3964 = 1.0f / (396.0f * 396.0f * 396.0f * 396.0f);
    float ram_sum  = 0.0f;
    float ram_term = 1103.0f;
    int   ram_k    = 0;
    const int ram_max_terms = 15;
    float pi_ram = 0.0f;

    for (int n = 1; n <= maxIter; ++n)
    {
        //Базель
        basel_sum += 1.0f / (n * n);
        float pi_basel = sqrt(6.0f * basel_sum);

        //Уоллис
        float k = (float)n;
        float num1 = 2.0f * k;
        float wallis_factor =
            (num1 / (num1 - 1.0f)) * (num1 / (num1 + 1.0f));
        wallis_prod *= wallis_factor;
        float pi_wallis = 2.0f * wallis_prod;

        //Виета
        if (n > 1)
        {
            vieta_a = sqrt(0.5f + 0.5f * vieta_a);
            vieta_prod *= vieta_a;
        }
        float pi_vieta = 2.0f / vieta_prod;

        //Мачин
        int m_k = n - 1;
        // arctan(1/5)
        m_sum1 += m_sign1 * m_pow1 / (2 * m_k + 1);
        m_pow1 *= (1.0f / 5.0f) * (1.0f / 5.0f);
        m_sign1 = -m_sign1;
        m_sum2 += m_sign2 * m_pow2 / (2 * m_k + 1);
        m_pow2 *= (1.0f / 239.0f) * (1.0f / 239.0f);
        m_sign2 = -m_sign2;

        float pi_machin = 16.0f * m_sum1 - 4.0f * m_sum2;

        //Рамануджан
        if (n <= ram_max_terms)
        {
            ram_sum += ram_term;
            float kf = (float)ram_k;
            float num = (4.0f * kf + 1.0f) * (4.0f * kf + 2.0f)
                      * (4.0f * kf + 3.0f) * (4.0f * kf + 4.0f);
            float den = (kf + 1.0f) * (kf + 1.0f)
                      * (kf + 1.0f) * (kf + 1.0f);
            float ratio = num / den;

            float num2 = 1103.0f + 26390.0f * (kf + 1.0f);
            float den2 = 1103.0f + 26390.0f * kf;
            ratio *= num2 / den2;
            ratio *= inv3964;

            ram_term *= ratio;
            ram_k++;
        }

        if (ram_sum != 0.0f)
            pi_ram = 1.0f / (ram_factor * ram_sum);
        if (n % step == 0)
        {
            f << n << ","
              << pi_basel << ","
              << pi_wallis << ","
              << pi_vieta << ","
              << pi_machin << ","
              << pi_ram << endl;
        }
    }

    return 0;
}