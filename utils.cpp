#include "bigint_multiply.h"
#include <random>

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