#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <algorithm>
#include <omp.h>
#include <cmath>

static constexpr size_t KARATSUBA_THRESHOLD = 64;
static constexpr size_t PARALLEL_THRESHOLD = 1024;

std::vector<long long> par_karatsuba_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y) {
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
    
    if (len >= PARALLEL_THRESHOLD) {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                P1 = par_karatsuba_mul_vector(Xl, Yl);
            }
            
            #pragma omp section
            {
                P2 = par_karatsuba_mul_vector(Xr, Yr);
            }
        }
    } else {
        P1 = par_karatsuba_mul_vector(Xl, Yl);
        P2 = par_karatsuba_mul_vector(Xr, Yr);
    }
    
    std::vector<long long> Xlr(k);
    std::vector<long long> Ylr(k);
    
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
    
    P3 = par_karatsuba_mul_vector(Xlr, Ylr);
    
    if (len >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel for
        for (size_t i = 0; i < len; ++i) {
            P3[i] -= P2[i] + P1[i];
        }
    } else {
        for (size_t i = 0; i < len; ++i) {
            P3[i] -= P2[i] + P1[i];
        }
    }
    
    if (len >= PARALLEL_THRESHOLD / 4) {
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                for (size_t i = 0; i < len; ++i) {
                    res[i] = P2[i];
                }
            }
            
            #pragma omp section
            {
                for (size_t i = len; i < 2 * len; ++i) {
                    res[i] = P1[i - len];
                }
            }
            
            #pragma omp section
            {
                for (size_t i = k; i < len + k; ++i) {
                    res[i] += P3[i - k];
                }
            }
        }
    } else {
        for (size_t i = 0; i < len; ++i) {
            res[i] = P2[i];
        }
        for (size_t i = len; i < 2 * len; ++i) {
            res[i] = P1[i - len];
        }
        for (size_t i = k; i < len + k; ++i) {
            res[i] += P3[i - k];
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
    
    int max_threads = omp_get_max_threads();
    int num_threads = std::min(max_threads, static_cast<int>(std::log2(vec_size)));
    if (num_threads > 1) {
        omp_set_num_threads(num_threads);
    }
    
    std::vector<long long> result_vec = karatsuba_mul_vector(a_vec, b_vec);
    
    return vector_to_string(result_vec);
}

// #include "bigint_multiply.h"
// #include <vector>
// #include <string>
// #include <algorithm>
// #include <omp.h>
// #include <cmath>
// #include <iostream>

// static constexpr size_t KARATSUBA_THRESHOLD = 64;
// static constexpr size_t PARALLEL_TASK_THRESHOLD = 512;

// std::vector<long long> par_karatsuba_recursive(const std::vector<long long>& x, const std::vector<long long>& y);

// std::vector<long long> par_karatsuba_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y) {
//     if (x.empty() || y.empty()) {
//         return {};
//     }
//      if (x.size() != y.size()) {
//          std::cerr << "Warning: par_karatsuba_mul_vector called with vectors of different sizes ("
//                    << x.size() << " vs " << y.size() << "). Results may be incorrect." << std::endl;
//     }

//     std::vector<long long> result;
//     #pragma omp parallel
//     {
//         #pragma omp single
//         {
//             result = par_karatsuba_recursive(x, y);
//         }
//     }
//     return result;
// }


// std::vector<long long> par_karatsuba_recursive(const std::vector<long long>& x, const std::vector<long long>& y) {
//     size_t len = x.size();

//     if (len <= KARATSUBA_THRESHOLD) {
//         return naive_mul_vector(x, y);
//     }

//     size_t k = len / 2;
//     size_t k_rem = len - k;

//     std::vector<long long> Xr(x.begin(), x.begin() + k);
//     std::vector<long long> Xl(x.begin() + k, x.begin() + k + k_rem);
//     std::vector<long long> Yr(y.begin(), y.begin() + k);
//     std::vector<long long> Yl(y.begin() + k, y.begin() + k + k_rem);

//     std::vector<long long> P1, P2, P3;
//     std::vector<long long> Xlr(k_rem);
//     std::vector<long long> Ylr(k_rem);

//     #pragma omp task shared(P2, Xr, Yr) if(len >= PARALLEL_TASK_THRESHOLD)
//     {
//         P2 = par_karatsuba_recursive(Xr, Yr);
//     }

//     #pragma omp task shared(P1, Xl, Yl) if(len >= PARALLEL_TASK_THRESHOLD)
//     {
//         P1 = par_karatsuba_recursive(Xl, Yl);
//     }

//     if (k != k_rem) {
//          std::cerr << "Error: Uneven split detected in par_karatsuba_recursive ("
//                    << k << " vs " << k_rem << "). Padding might be incorrect." << std::endl;
//     }

//     for (size_t i = 0; i < k; ++i) {
//          Xlr[i] = Xl[i] + Xr[i];
//          Ylr[i] = Yl[i] + Yr[i];
//     }

//     #pragma omp task shared(P3, Xlr, Ylr) if(len >= PARALLEL_TASK_THRESHOLD)
//     {
//         P3 = par_karatsuba_recursive(Xlr, Ylr);
//     }

//     #pragma omp taskwait

//     #pragma omp taskloop shared(P3, P1, P2) grainsize(256) if(len >= PARALLEL_TASK_THRESHOLD)
//     for (size_t i = 0; i < P1.size(); ++i) { // P1 and P2 have size ~len
//         P3[i] -= (P1[i] + P2[i]);
//     }
//     #pragma omp taskwait
//     std::vector<long long> res(2 * len);

//     for (size_t i = 0; i < P2.size(); ++i) {
//         res[i] = P2[i];
//     }
//     for (size_t i = 0; i < P1.size(); ++i) {
//         res[i + len] = P1[i];
//     }
//     for (size_t i = 0; i < P3.size(); ++i) {
//         res[i + k] += P3[i];
//     }

//     return res;
// }


// std::string par_karatsuba_mul_string(const std::string &a, const std::string &b) {
//      if (a.empty() || b.empty()) return "0";
//      if (a == "0" || b == "0") return "0";
//     if (a.size() <= KARATSUBA_THRESHOLD + 10 || b.size() <= KARATSUBA_THRESHOLD + 10) {
//         return karatsuba_mul_string(a, b);
//     }

//     size_t max_len = std::max(a.size(), b.size());
//     size_t vec_size = 1;
//     while (vec_size < max_len) {
//         vec_size *= 2;
//     }
//     std::vector<long long> a_vec = string_to_vector(a); // Don't pad here yet
//     std::vector<long long> b_vec = string_to_vector(b); // Don't pad here yet

//     a_vec.resize(vec_size, 0);
//     b_vec.resize(vec_size, 0);

//     std::vector<long long> result_vec = par_karatsuba_mul_vector(a_vec, b_vec);

//     return vector_to_string(result_vec);
// }