#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>
#include <omp.h>
#include <cmath>

#include "parlaylib/include/parlay/primitives.h"
#include "parlaylib/include/parlay/parallel.h"
#include "parlaylib/include/parlay/sequence.h"
#include "parlaylib/include/parlay/utilities.h"


static constexpr size_t KARATSUBA_THRESHOLD = 64;
static constexpr size_t PARALLEL_THRESHOLD = 10000;

std::vector<long long> par_karatsuba_mul_vector_open(const std::vector<long long>& x, const std::vector<long long>& y) {
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
    
    std::vector<long long> P1, P2, P3;
    
    std::vector<long long> Xlr(k);
    std::vector<long long> Ylr(k);

    #pragma omp parallel
    #pragma omp single nowait
    {
        #pragma omp task shared(P1)
        {
            P1 = par_karatsuba_mul_vector_open(Xl, Yl);
        }

        #pragma omp task shared(P2)
        {
            P2 = par_karatsuba_mul_vector_open(Xr, Yr);
        }

        #pragma omp task shared(Xlr, Ylr)
        {
            if (k >= PARALLEL_THRESHOLD / 4) {
                #pragma omp parallel for
                for (size_t i = 0; i < k; ++i) {
                    Xlr[i] = Xl[i] + Xr[i];
                    Ylr[i] = Yl[i] + Yr[i];
                }
            } else {
                for (size_t i = 0; i < k; ++i) {
                    Xlr[i] = Xl[i] + Xr[i];
                    Ylr[i] = Yl[i] + Yr[i];
                }
            }
        }

        #pragma omp task shared(P3, Xlr, Ylr)
        {
            P3 = par_karatsuba_mul_vector_open(Xlr, Ylr);
        }
    }

    #pragma omp taskwait
    
    if (len >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < P3.size(); ++i) {
            if (i < P1.size()) P3[i] -= P1[i];
            if (i < P2.size()) P3[i] -= P2[i];
        }
    } else {
        for (size_t i = 0; i < P3.size(); ++i) {
            if (i < P1.size()) P3[i] -= P1[i];
            if (i < P2.size()) P3[i] -= P2[i];
        }
    }

    for (size_t i = 0; i < P2.size(); ++i) {
        res[i] += P2[i];
    }

    for (size_t i = 0; i < P3.size(); ++i) {
        if (i + k < res.size()) { 
            res[i + k] += P3[i];
        } else {
        }
    }

    for (size_t i = 0; i < P1.size(); ++i) {
        if (i + len < res.size()) { 
            res[i + len] += P1[i];
        } else {
        }
    }

    return res;
}

std::vector<long long> par_karatsuba_mul_vector_plib(const std::vector<long long>& x, const std::vector<long long>& y) {
    auto len = x.size();    
    std::vector<long long> res(2 * len, 0);
    
    if (len <= KARATSUBA_THRESHOLD) {
        return naive_mul_vector(x, y);
    }
    
    auto k = len / 2;
    
    std::vector<long long> Xr(x.begin(), x.begin() + k);
    std::vector<long long> Xl(x.begin() + k, x.end());
    std::vector<long long> Yr(y.begin(), y.begin() + k);
    std::vector<long long> Yl(y.begin() + k, y.end());

    std::vector<long long> P1, P2, P3;
    std::vector<long long> Xlr(k), Ylr(k);

    if (k >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, k, [&](size_t i) {
            Xlr[i] = Xl[i] + Xr[i];
            Ylr[i] = Yl[i] + Yr[i];
        });
    } else {
        for (size_t i = 0; i < k; ++i) {
            Xlr[i] = Xl[i] + Xr[i];
            Ylr[i] = Yl[i] + Yr[i];
        }
    }

    parlay::par_do(
        [&]() { P1 = par_karatsuba_mul_vector_plib(Xl, Yl); },
        [&]() {parlay::par_do(
            [&]() { P2 = par_karatsuba_mul_vector_plib(Xr, Yr); },
            [&]() { P3 = par_karatsuba_mul_vector_plib(Xlr, Ylr); } // Xlr, Ylr are ready
        );}
    );
    
    size_t max_size = std::max(P1.size(), P2.size());
    if (P3.size() < max_size) {
        P3.resize(max_size, 0);
    }
    
    if (len >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, P3.size(), [&](size_t i) {
            if (i < P1.size()) P3[i] -= P1[i];
            if (i < P2.size()) P3[i] -= P2[i];
        });
    } else {
        for (size_t i = 0; i < P3.size(); ++i) {
            if (i < P1.size()) P3[i] -= P1[i];
            if (i < P2.size()) P3[i] -= P2[i];
        }
    }

    if (res.size() < 2 * len) {
        res.resize(2 * len, 0);
    }
    
    if (len >= PARALLEL_THRESHOLD / 4) {
        parlay::parallel_for(0, P2.size(), [&](size_t i) {
            res[i] += P2[i];
        });

        parlay::parallel_for(0, P3.size(), [&](size_t i) {
            if (i + k < res.size()) {
                res[i + k] += P3[i];
            }
        });

        parlay::parallel_for(0, P1.size(), [&](size_t i) {
            if (i + len < res.size()) {
                res[i + len] += P1[i];
            }
        });
    } else {
        for (size_t i = 0; i < P2.size(); ++i) {
            res[i] += P2[i];
        }

        for (size_t i = 0; i < P3.size(); ++i) {
            if (i + k < res.size()) { 
                res[i + k] += P3[i];
            }
        }

        for (size_t i = 0; i < P1.size(); ++i) {
            if (i + len < res.size()) { 
                res[i + len] += P1[i];
            }
        }
    }

    return res;
}

std::string par_karatsuba_mul_string(const std::string &a, const std::string &b) {
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
    
    std::vector<long long> result_vec = par_karatsuba_mul_vector_open(a_vec, b_vec);
    
    return vector_to_string(result_vec);
}