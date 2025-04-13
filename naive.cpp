#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

static constexpr size_t TRUNCATE_THRESHOLD = 100;
static constexpr size_t TRUNCATE_HEAD = 50;
static constexpr size_t TRUNCATE_TAIL = 50;

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

std::string naive_mul(const std::string &a, const std::string &b) {
    size_t n = a.size(), m = b.size();
    std::vector<int> prod(n + m, 0);

    for (size_t i = 0; i < n; ++i) {
        int da = a[n - 1 - i] - '0';
        for (size_t j = 0; j < m; ++j) {
            int db = b[m - 1 - j] - '0';
            prod[i + j] += da * db;
        }
    }

    for (size_t k = 0; k + 1 < prod.size(); ++k) {
        prod[k + 1] += prod[k] / 10;
        prod[k] %= 10;
    }

    size_t idx = prod.size();
    while (idx > 1 && prod[idx - 1] == 0) {
        --idx;
    }

    std::string result;
    result.reserve(idx);
    for (size_t i = 0; i < idx; ++i) {
        result.push_back(char('0' + prod[idx - 1 - i]));
    }
    return result;
}

std::string truncate_display(const std::string &s) {
    if (s.size() <= TRUNCATE_THRESHOLD) {
        return s;
    }
    return s.substr(0, TRUNCATE_HEAD)
         + "…"
         + s.substr(s.size() - TRUNCATE_TAIL);
}

int main(int argc, char* argv[]) {
    size_t num_tests = 5;
    size_t length = 1000;
    double total_runtime = 0;

    if (argc >= 2) num_tests = std::stoul(argv[1]);
    if (argc >= 3) length    = std::stoul(argv[2]);

    std::cout << "Running " << num_tests
              << " tests with " << length
              << "-digit operands each.\n\n";

    for (size_t t = 1; t <= num_tests; ++t) {
        auto A = random_bigint(length);
        auto B = random_bigint(length);

        auto start = std::chrono::high_resolution_clock::now();
        auto C     = naive_mul(A, B);
        auto end   = std::chrono::high_resolution_clock::now();

        if (length <= 9) {
            long long a_ll = std::stoll(A);
            long long b_ll = std::stoll(B);
            long long c_ll = a_ll * b_ll;
            std::string expected = std::to_string(c_ll);
        
            if (expected != C) {
                std::cerr << "Mismatch! Expected " << expected << "\n";
            } else {
                std::cout << "Matches C++ multiplication\n";
            }
        }

        std::chrono::duration<double> elapsed = end - start;
        total_runtime += elapsed.count();

        std::cout << "Test #" << t << ":\n";
        std::cout << "  A (" << A.size() << " digits) = "
                  << truncate_display(A) << "\n";
        std::cout << "  B (" << B.size() << " digits) = "
                  << truncate_display(B) << "\n";
        std::cout << "  A * B (" << C.size() << " digits) = "
                  << truncate_display(C) << "\n";
        std::cout << "  Time elapsed: " << elapsed.count() << " seconds\n\n";
    }
    std::cout << "Average Time Elapsed: " << total_runtime / num_tests << " seconds\n\n";

    return 0;
}
