#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>
#include <omp.h>

using namespace std;

using BigInt = vector<long long>;

static constexpr size_t TOOM_COOK_THRESHOLD = 64;
static constexpr size_t PARALLEL_THRESHOLD = 1024;

BigInt par_add(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));
    
    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x + y;
        }
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x + y;
        }
    }
    return res;
}

BigInt par_subtract(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x - y;
        }
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x - y;
        }
    }
    return res;
}

BigInt par_multiply_scalar(const BigInt &a, int scalar) {
    BigInt res(a.size());

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] * scalar;
        }
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] * scalar;
        }
    }
    return res;
}

BigInt par_divide_scalar(const BigInt &a, int scalar) {
    BigInt res(a.size());

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] / scalar;
        }
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] / scalar;
        }
    }
    return res;
}

// par_shifts BigInt by n digits (multiply by 10^n)
BigInt par_shift(const BigInt &a, int n) {
    BigInt res(n, 0);
    res.insert(res.end(), a.begin(), a.end());
    return res;
}

BigInt par_toom_cook_mul_vector(const BigInt &x, const BigInt &y) {
    auto len = x.size();    
    
    if (len <= TOOM_COOK_THRESHOLD) {
        return naive_mul_vector(x, y);
    }
    
    int k = (len + 2) / 3;

    // Split x and y into 3 parts each
    auto split = [&](const BigInt &num) -> array<BigInt, 3> {
        array<BigInt, 3> parts = {};
        for (int i = 0; i < 3; i++) {
            int start = i * k;
            int end = min((i + 1) * k, (int)num.size());
            parts[i] = BigInt(num.begin() + start, num.begin() + end);
        }
        return parts;
    };

    auto X = split(x);
    auto Y = split(y);

    // Evaluate at 5 points
    BigInt P0 = X[0];
    BigInt P1 = par_add(par_add(X[2], X[1]), X[0]);
    BigInt Pm1 = par_add(par_subtract(X[2], X[1]), X[0]);
    BigInt Pm2 = par_add(par_subtract(X[0], par_multiply_scalar(X[1], 2)), par_multiply_scalar(X[2], 4));
    BigInt Pinf = X[2];

    BigInt Q0 = Y[0];
    BigInt Q1 = par_add(par_add(Y[2], Y[1]), Y[0]);
    BigInt Qm1 = par_add(par_subtract(Y[2], Y[1]), Y[0]);
    BigInt Qm2 = par_add(par_subtract(Y[0], par_multiply_scalar(Y[1], 2)), par_multiply_scalar(Y[2], 4));
    BigInt Qinf = Y[2];

    // Pointwise multiplications
    BigInt R0, R1, Rm1, Rm2, Rinf;

    if (len >= PARALLEL_THRESHOLD) {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                R0 = toom_cook_mul_vector(P0, Q0);
            }
            #pragma omp section
            {
                R1 = toom_cook_mul_vector(P1, Q1);
            }
            #pragma omp section
            {
                Rm1 = toom_cook_mul_vector(Pm1, Qm1);
            }
            #pragma omp section
            {
                Rm2 = toom_cook_mul_vector(Pm2, Qm2);
            }
            #pragma omp section
            {
                Rinf = toom_cook_mul_vector(Pinf, Qinf);
            }
        }
    } else {
        R0 = toom_cook_mul_vector(P0, Q0);
        R1 = toom_cook_mul_vector(P1, Q1);
        Rm1 = toom_cook_mul_vector(Pm1, Qm1);
        Rm2 = toom_cook_mul_vector(Pm2, Qm2);
        Rinf = toom_cook_mul_vector(Pinf, Qinf);
    }

    // Interpolation
    BigInt r0 = R0;
    BigInt r4 = Rinf;
    BigInt r3 = par_divide_scalar(par_subtract(Rm2, R1), 3);
    BigInt r1 = par_divide_scalar(par_subtract(R1, Rm1), 2);
    BigInt r2 = par_subtract(Rm1, R0);
    r3 = par_add(par_divide_scalar(par_subtract(r2, r3), 2), par_multiply_scalar(Rinf, 2));
    r2 = par_subtract(par_add(r2, r1), r4);
    r1 = par_subtract(r1, r3);

    // Combine
    BigInt result = r0;
    result = par_add(result, par_shift(r1, k));
    result = par_add(result, par_shift(r2, 2 * k));
    result = par_add(result, par_shift(r3, 3 * k));
    result = par_add(result, par_shift(r4, 4 * k));
    
    return result;
}

std::string par_toom_cook_mul_string(const std::string &a, const std::string &b) {
    if (a.size() <= TOOM_COOK_THRESHOLD || b.size() <= TOOM_COOK_THRESHOLD) {
        return naive_mul_string(a, b);
    }
    
    BigInt a_vec = string_to_vector(a, true);
    BigInt b_vec = string_to_vector(b, true);
    size_t vec_size = std::max(a_vec.size(), b_vec.size());
    a_vec.resize(vec_size, 0);
    b_vec.resize(vec_size, 0);
    BigInt result_vec = par_toom_cook_mul_vector(a_vec, b_vec);

    return vector_to_string(result_vec);
}