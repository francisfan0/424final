#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>

#include "parlaylib/include/parlay/primitives.h"
#include "parlaylib/include/parlay/parallel.h"
#include "parlaylib/include/parlay/sequence.h"
#include "parlaylib/include/parlay/utilities.h"


using namespace std;

using BigInt = vector<long long>;

static constexpr size_t TOOM_COOK_THRESHOLD = 64;
static constexpr size_t PARALLEL_THRESHOLD = 10000;

BigInt par_add_plib(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));
    
    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, res.size(), [&](size_t i) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x + y;
        });
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x + y;
        }
    }
    return res;
}

BigInt par_subtract_plib(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, res.size(), [&](size_t i) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x - y;
        });
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            auto x = i < a.size() ? a[i] : 0;
            auto y = i < b.size() ? b[i] : 0;
            res[i] = x - y;
        }
    }
    return res;
}

BigInt par_multiply_scalar_plib(const BigInt &a, int scalar) {
    BigInt res(a.size());

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, res.size(), [&](size_t i) {
            res[i] = a[i] * scalar;
        });
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] * scalar;
        }
    }
    return res;
}

BigInt par_divide_scalar_plib(const BigInt &a, int scalar) {
    BigInt res(a.size());

    if (res.size() >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, res.size(), [&](size_t i) {
            res[i] = a[i] / scalar;
        });
    } else {
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = a[i] / scalar;
        }
    }
    return res;
}

// par_shift_plibs BigInt by n digits (multiply by 10^n)
BigInt par_shift_plib(const BigInt &a, int n) {
    BigInt res(n, 0);
    res.insert(res.end(), a.begin(), a.end());
    return res;
}

BigInt par_toom_cook_mul_vector_plib(const BigInt &x, const BigInt &y) {
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
    BigInt P1 = par_add_plib(par_add_plib(X[2], X[1]), X[0]);
    BigInt Pm1 = par_add_plib(par_subtract_plib(X[2], X[1]), X[0]);
    BigInt Pm2 = par_add_plib(par_subtract_plib(X[0], par_multiply_scalar_plib(X[1], 2)), par_multiply_scalar_plib(X[2], 4));
    BigInt Pinf = X[2];

    BigInt Q0 = Y[0];
    BigInt Q1 = par_add_plib(par_add_plib(Y[2], Y[1]), Y[0]);
    BigInt Qm1 = par_add_plib(par_subtract_plib(Y[2], Y[1]), Y[0]);
    BigInt Qm2 = par_add_plib(par_subtract_plib(Y[0], par_multiply_scalar_plib(Y[1], 2)), par_multiply_scalar_plib(Y[2], 4));
    BigInt Qinf = Y[2];

    // Pointwise multiplications
    BigInt R0, R1, Rm1, Rm2, Rinf;

    if (len >= PARALLEL_THRESHOLD) {
        parlay::par_do(
            [&]() { R0 = par_toom_cook_mul_vector_plib(P0, Q0); },
            [&]() {parlay::par_do(
                [&]() { R1 = par_toom_cook_mul_vector_plib(P1, Q1); },
                [&]() {parlay::par_do(
                    [&]() { Rm1 = par_toom_cook_mul_vector_plib(Pm1, Qm1); },
                    [&]() {parlay::par_do(
                        [&]() { Rm2 = par_toom_cook_mul_vector_plib(Pm2, Qm2); },
                        [&]() { Rinf = par_toom_cook_mul_vector_plib(Pinf, Qinf); }
                    );}
                );}
            );}
        );
    } else {
        R0 = par_toom_cook_mul_vector_plib(P0, Q0);
        R1 = par_toom_cook_mul_vector_plib(P1, Q1);
        Rm1 = par_toom_cook_mul_vector_plib(Pm1, Qm1);
        Rm2 = par_toom_cook_mul_vector_plib(Pm2, Qm2);
        Rinf = par_toom_cook_mul_vector_plib(Pinf, Qinf);
    }

    // Interpolation
    BigInt r0 = R0;
    BigInt r4 = Rinf;
    BigInt r3 = par_divide_scalar_plib(par_subtract_plib(Rm2, R1), 3);
    BigInt r1 = par_divide_scalar_plib(par_subtract_plib(R1, Rm1), 2);
    BigInt r2 = par_subtract_plib(Rm1, R0);
    r3 = par_add_plib(par_divide_scalar_plib(par_subtract_plib(r2, r3), 2), par_multiply_scalar_plib(Rinf, 2));
    r2 = par_subtract_plib(par_add_plib(r2, r1), r4);
    r1 = par_subtract_plib(r1, r3);

    // Combine
    BigInt result = r0;
    result = par_add_plib(result, par_shift_plib(r1, k));
    result = par_add_plib(result, par_shift_plib(r2, 2 * k));
    result = par_add_plib(result, par_shift_plib(r3, 3 * k));
    result = par_add_plib(result, par_shift_plib(r4, 4 * k));
    
    return result;
}

std::string par_toom_cook_mul_string_plib(const std::string &a, const std::string &b) {
    if (a.size() <= TOOM_COOK_THRESHOLD || b.size() <= TOOM_COOK_THRESHOLD) {
        return naive_mul_string(a, b);
    }
    
    BigInt a_vec = string_to_vector(a, true);
    BigInt b_vec = string_to_vector(b, true);
    size_t vec_size = std::max(a_vec.size(), b_vec.size());
    a_vec.resize(vec_size, 0);
    b_vec.resize(vec_size, 0);
    BigInt result_vec = par_toom_cook_mul_vector_plib(a_vec, b_vec);

    return vector_to_string(result_vec);
}