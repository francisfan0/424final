#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

using BigInt = vector<long long>;

static constexpr size_t TOOM_COOK_THRESHOLD = 64;

BigInt add(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));
    for (size_t i = 0; i < res.size(); i++) {
        auto x = i < a.size() ? a[i] : 0;
        auto y = i < b.size() ? b[i] : 0;
        res[i] = x + y;
    }
    return res;
}

BigInt subtract(const BigInt &a, const BigInt &b) {
    BigInt res(max(a.size(), b.size()));
    for (size_t i = 0; i < res.size(); i++) {
        auto x = i < a.size() ? a[i] : 0;
        auto y = i < b.size() ? b[i] : 0;
        res[i] = x - y;
    }
    return res;
}

BigInt multiply_scalar(const BigInt &a, int scalar) {
    BigInt res(a.size());
    for (size_t i = 0; i < res.size(); i++) {
        res[i] = a[i] * scalar;
    }
    return res;
}

BigInt divide_scalar(const BigInt &a, int scalar) {
    BigInt res(a.size());
    for (size_t i = 0; i < res.size(); i++) {
        res[i] = a[i] / scalar;
    }
    return res;
}

// Shifts BigInt by n digits (multiply by 10^n)
BigInt shift(const BigInt &a, int n) {
    BigInt res(n, 0);
    res.insert(res.end(), a.begin(), a.end());
    return res;
}

BigInt toom_cook_mul_vector(const BigInt &x, const BigInt &y) {
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
    BigInt P1 = add(add(X[2], X[1]), X[0]);
    BigInt Pm1 = add(subtract(X[2], X[1]), X[0]);
    BigInt Pm2 = add(subtract(X[0], multiply_scalar(X[1], 2)), multiply_scalar(X[2], 4));
    BigInt Pinf = X[2];

    BigInt Q0 = Y[0];
    BigInt Q1 = add(add(Y[2], Y[1]), Y[0]);
    BigInt Qm1 = add(subtract(Y[2], Y[1]), Y[0]);
    BigInt Qm2 = add(subtract(Y[0], multiply_scalar(Y[1], 2)), multiply_scalar(Y[2], 4));
    BigInt Qinf = Y[2];

    // Pointwise multiplications
    BigInt R0 = toom_cook_mul_vector(P0, Q0);
    BigInt R1 = toom_cook_mul_vector(P1, Q1);
    BigInt Rm1 = toom_cook_mul_vector(Pm1, Qm1);
    BigInt Rm2 = toom_cook_mul_vector(Pm2, Qm2);
    BigInt Rinf = toom_cook_mul_vector(Pinf, Qinf);

    // Interpolation
    BigInt r0 = R0;
    BigInt r4 = Rinf;
    BigInt r3 = divide_scalar(subtract(Rm2, R1), 3);
    BigInt r1 = divide_scalar(subtract(R1, Rm1), 2);
    BigInt r2 = subtract(Rm1, R0);
    r3 = add(divide_scalar(subtract(r2, r3), 2), multiply_scalar(Rinf, 2));
    r2 = subtract(add(r2, r1), r4);
    r1 = subtract(r1, r3);

    // Combine
    BigInt result = r0;
    result = add(result, shift(r1, k));
    result = add(result, shift(r2, 2 * k));
    result = add(result, shift(r3, 3 * k));
    result = add(result, shift(r4, 4 * k));
    
    return result;
}

std::string toom_cook_mul_string(const std::string &a, const std::string &b) {
    if (a.size() <= TOOM_COOK_THRESHOLD || b.size() <= TOOM_COOK_THRESHOLD) {
        return naive_mul_string(a, b);
    }
    
    BigInt a_vec = string_to_vector(a, true);
    BigInt b_vec = string_to_vector(b, true);
    size_t vec_size = std::max(a_vec.size(), b_vec.size());
    a_vec.resize(vec_size, 0);
    b_vec.resize(vec_size, 0);
    BigInt result_vec = toom_cook_mul_vector(a_vec, b_vec);

    return vector_to_string(result_vec);
}