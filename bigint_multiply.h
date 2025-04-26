#ifndef BIGINT_MULTIPLY_H
#define BIGINT_MULTIPLY_H

#include <string>
#include <vector>
#include <algorithm>
#include <iostream> 
#include <iomanip>
#include <omp.h>
#include <random>
#include <cmath>
#include <limits>

std::string naive_mul_string(const std::string &a, const std::string &b);
std::string karatsuba_mul_string(const std::string &a, const std::string &b);
std::string par_karatsuba_mul_string(const std::string &a, const std::string &b);
std::string toom_cook_mul_string(const std::string &a, const std::string &b);

std::vector<long long> naive_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y);
std::vector<long long> karatsuba_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y);
std::vector<long long> par_karatsuba_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y);
std::vector<long long> toom_cook_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y);

std::vector<long long> string_to_vector(const std::string& s, bool pad_to_power_of_2 = false);
std::string vector_to_string(const std::vector<long long>& v);

std::string random_bigint(size_t len);
std::string truncate_display(const std::string &s, size_t head = 50, size_t tail = 50);

#endif // BIGINT_MULTIPLY_H