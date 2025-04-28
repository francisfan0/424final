#include "bigint_multiply.h"
#include <vector>
#include <string>
#include <cmath> // For ceil/log2
#include <algorithm> // For max
#include <omp.h> // Include OpenMP header

std::string random_bigint(size_t len) {
    static std::mt19937_64 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> digit_dist(0, 9);

    std::string s;
    s.reserve(len);
    s.push_back(char('1' + digit_dist(rng) % 9));
    for (size_t i = 1; i < len; ++i) {
        s.push_back(char('0' + digit_dist(rng)));
    }
    return s;
}

std::string truncate_display(const std::string &s, size_t head, size_t tail) {
    static constexpr size_t TRUNCATE_THRESHOLD = 100;
    
    if (s.size() <= TRUNCATE_THRESHOLD) {
        return s;
    }
    return s.substr(0, head) + "..." + s.substr(s.size() - tail);
}

std::vector<long long> string_to_vector(const std::string& s, bool pad_to_power_of_2) {
    size_t original_len = s.size();
    size_t len = original_len;

    if (pad_to_power_of_2) {
        if (len > 0) {
             len = 1;
             while (len < original_len) len *= 2;
        } else {
             len = 0;
        }
    }

    std::vector<long long> result(len, 0);

    #pragma omp parallel for
    for (size_t i = 0; i < original_len; ++i) {
        size_t result_idx = original_len - 1 - i;
        if (result_idx < len) {
            result[result_idx] = s[i] - '0';
        }
    }

    return result;
}

std::string vector_to_string(const std::vector<long long>& v) {
    std::vector<int> normalized(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        normalized[i] = v[i];
    }
    
    for (size_t i = 0; i < normalized.size() - 1; ++i) {
        if (normalized[i] >= 10) {
            normalized[i+1] += normalized[i] / 10;
            normalized[i] %= 10;
        }
    }

    size_t idx = normalized.size();
    while (idx > 1 && normalized[idx - 1] == 0) {
        --idx;
    }
    
    std::string result;
    result.reserve(idx);
    for (int i = idx - 1; i >= 0; --i) {
        result.push_back('0' + normalized[i]);
    }
    
    return result;
}