#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>

static constexpr size_t KARATSUBA_THRESHOLD = 64;

std::vector<long long> karatsuba_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y) {
    auto len = x.size();    
    std::vector<long long> res(2 * len);
    
    if (len <= KARATSUBA_THRESHOLD) {
        return naive_mul_vector(x, y);
    }
    
    auto k = len / 2;
    
    std::vector<long long> Xr(x.begin(), x.begin() + k);
    std::vector<long long> Xl(x.begin() + k, x.end());
    std::vector<long long> Yr(y.begin(), y.begin() + k);
    std::vector<long long> Yl(y.begin() + k, y.end());
    
    std::vector<long long> P1 = karatsuba_mul_vector(Xl, Yl);
    std::vector<long long> P2 = karatsuba_mul_vector(Xr, Yr);
    
    std::vector<long long> Xlr(k);
    std::vector<long long> Ylr(k);
    
    for (auto i = 0; i < k; ++i) {
        Xlr[i] = Xl[i] + Xr[i];
        Ylr[i] = Yl[i] + Yr[i];
    }
    
    std::vector<long long> P3 = karatsuba_mul_vector(Xlr, Ylr);
    
    for (auto i = 0; i < len; ++i) {
        P3[i] -= P2[i] + P1[i];
    }
    for (auto i = 0; i < len; ++i) {
        res[i] = P2[i];
    }

    for (auto i = len; i < 2 * len; ++i) {
        res[i] = P1[i - len];
    }

    for (auto i = k; i < len + k; ++i) {
        res[i] += P3[i - k];
    }
    
    return res;
}

std::string karatsuba_mul_string(const std::string &a, const std::string &b) {
    if (a.size() <= KARATSUBA_THRESHOLD || b.size() <= KARATSUBA_THRESHOLD) {
        return naive_mul_string(a, b);
    }
    
    size_t max_len = std::max(a.size(), b.size());
    size_t pow2 = 1;
    while (pow2 < max_len) pow2 *= 2;
    
    std::vector<long long> a_vec = string_to_vector(a, true);
    std::vector<long long> b_vec = string_to_vector(b, true);
    size_t vec_size = std::max(a_vec.size(), b_vec.size());
    a_vec.resize(vec_size, 0);
    b_vec.resize(vec_size, 0);
    std::vector<long long> result_vec = karatsuba_mul_vector(a_vec, b_vec);
    
    return vector_to_string(result_vec);
}