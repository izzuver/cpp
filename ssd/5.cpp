#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
using namespace std;

const float PI_TRUE = 3.14159265358979323846f;

double get_time()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(
               steady_clock::now().time_since_epoch()
           ).count() / 1e6;
}
int pi_basel_to_eps(float eps, int max_iter, float &result)
{
    float sum = 0.0f;
    for (int n = 1; n <= max_iter; ++n)
    {
        sum += 1.0f / (n * n);
        float pi_approx = sqrt(6.0f * sum);
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return n;
        }
    }
    result = sqrt(6.0f * sum);
    return -1;
}
int pi_wallis_to_eps(float eps, int max_iter, float &result)
{
    float prod = 1.0f;
    for (int n = 1; n <= max_iter; ++n)
    {
        float k = (float)n;
        float num = 2.0f * k;
        float factor = (num / (num - 1.0f)) * (num / (num + 1.0f));
        prod *= factor;
        float pi_approx = 2.0f * prod;
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return n;
        }
    }
    result = 2.0f * prod;
    return -1;
}
int pi_vieta_to_eps(float eps, int max_iter, float &result)
{
    float a    = sqrt(0.5f);
    float prod = a;

    for (int n = 1; n <= max_iter; ++n)
    {
        if (n > 1)
        {
            a = sqrt(0.5f + 0.5f * a);
            prod *= a;
        }
        float pi_approx = 2.0f / prod;
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return n;
        }
    }

    result = 2.0f / prod;
    return -1;
}
int pi_machin_to_eps(float eps, int max_iter, float &result)
{
    float sum1  = 0.0f;
    float pow1  = 1.0f / 5.0f;
    float sign1 = 1.0f;

    float sum2  = 0.0f;
    float pow2  = 1.0f / 239.0f;
    float sign2 = 1.0f;

    for (int k = 0; k < max_iter; ++k)
    {
        sum1 += sign1 * pow1 / (2 * k + 1);
        pow1 *= (1.0f / 5.0f) * (1.0f / 5.0f);
        sign1 = -sign1;
        sum2 += sign2 * pow2 / (2 * k + 1);
        pow2 *= (1.0f / 239.0f) * (1.0f / 239.0f);
        sign2 = -sign2;

        float pi_approx = 16.0f * sum1 - 4.0f * sum2;

        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return k + 1;
        }
    }

    result = 16.0f * sum1 - 4.0f * sum2;
    return -1;
}

int main()
{
    ofstream f("pi_time.csv", ios::out);
    if (!f)
    {
        cerr << "Не удалось открыть файл pi_time.csv\n";
        return 1;
    }

    f << "digits,formula,avg_time_sec,avg_iterations" << endl;

    const int REPEAT = 20;

    const int MAX_ITER_BASEL  = 2000000;
    const int MAX_ITER_WALLIS = 2000000;
    const int MAX_ITER_VIETA  = 40;
    const int MAX_ITER_MACHIN = 2000;

    for (int digits = 1; digits <= 10; ++digits)
    {
        // eps ~ 0.5 * 10^(-digits)
        float eps = 0.5f;
        for (int i = 0; i < digits; ++i)
            eps /= 10.0f;
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_basel_to_eps(eps, MAX_ITER_BASEL, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",basel,"
                  << (total_time / ok) << ","
                  << (total_iter / (double)ok) << endl;
            else
                f << digits << ",basel,-1,-1" << endl;
        }
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_wallis_to_eps(eps, MAX_ITER_WALLIS, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",wallis,"
                  << (total_time / ok) << ","
                  << (total_iter / (double)ok) << endl;
            else
                f << digits << ",wallis,-1,-1" << endl;
        }
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_vieta_to_eps(eps, MAX_ITER_VIETA, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",vieta,"
                  << (total_time / ok) << ","
                  << (total_iter / (double)ok) << endl;
            else
                f << digits << ",vieta,-1,-1" << endl;
        }
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_machin_to_eps(eps, MAX_ITER_MACHIN, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",machin,"
                  << (total_time / ok) << ","
                  << (total_iter / (double)ok) << endl;
            else
                f << digits << ",machin,-1,-1" << endl;
        }
    }

    return 0;
}