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

int pi_leibniz_to_eps(float eps, int max_iter, float &result)
{
    float sum  = 0.0f;
    float sign = 1.0f;
    for (int n = 0; n < max_iter; ++n)
    {
        sum += sign / (2 * n + 1);
        sign = -sign;
        float pi_approx = 4.0f * sum;
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return n + 1;
        }
    }
    result = 4.0f * sum;
    return -1;
}

int pi_nilakantha_to_eps(float eps, int max_iter, float &result)
{
    float pi_approx = 3.0f;
    float sign      = 1.0f;
    for (int n = 1; n <= max_iter; ++n)
    {
        float a    = 2.0f * n;
        float term = 4.0f / (a * (a + 1.0f) * (a + 2.0f));
        pi_approx += sign * term;
        sign = -sign;
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return n;
        }
    }
    result = pi_approx;
    return -1;
}

int pi_bbp_to_eps(float eps, int max_iter, float &result)
{
    float sum    = 0.0f;
    float factor = 1.0f;
    for (int k = 0; k < max_iter; ++k)
    {
        float term = 4.0f / (8 * k + 1)
                   - 2.0f / (8 * k + 4)
                   - 1.0f / (8 * k + 5)
                   - 1.0f / (8 * k + 6);
        sum += factor * term;
        float pi_approx = sum;
        if (fabs(pi_approx - PI_TRUE) < eps)
        {
            result = pi_approx;
            return k + 1;
        }
        factor /= 16.0f;
    }
    result = sum;
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
    const int MAX_ITER_BASEL    = 5000000;
    const int MAX_ITER_LEIBNIZ  = 5000000;
    const int MAX_ITER_NILA     = 2000000;
    const int MAX_ITER_BBP      = 1000;

    for (int digits = 1; digits <= 10; ++digits)
    {
        float eps = 0.5f;
        for (int i = 0; i < digits; ++i)
            eps /= 10.0f;

        // Basel
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
                f << digits << ",basel," << (total_time / ok) << "," << (total_iter / (double)ok) << endl;
            else
                f << digits << ",basel,-1,-1" << endl;
        }

        // Leibniz
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_leibniz_to_eps(eps, MAX_ITER_LEIBNIZ, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",leibniz," << (total_time / ok) << "," << (total_iter / (double)ok) << endl;
            else
                f << digits << ",leibniz,-1,-1" << endl;
        }

        // Nilakantha
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_nilakantha_to_eps(eps, MAX_ITER_NILA, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",nilakantha," << (total_time / ok) << "," << (total_iter / (double)ok) << endl;
            else
                f << digits << ",nilakantha,-1,-1" << endl;
        }

        // BBP
        {
            double total_time = 0.0;
            long long total_iter = 0;
            int ok = 0;

            for (int r = 0; r < REPEAT; ++r)
            {
                float res;
                double t0 = get_time();
                int it = pi_bbp_to_eps(eps, MAX_ITER_BBP, res);
                double t1 = get_time();

                if (it < 0) break;
                total_time += (t1 - t0);
                total_iter += it;
                ++ok;
            }

            if (ok > 0)
                f << digits << ",bbp," << (total_time / ok) << "," << (total_iter / (double)ok) << endl;
            else
                f << digits << ",bbp,-1,-1" << endl;
        }
    }
}