#include "bigint_multiply.h"
#include <vector>
#include <string>

std::string naive_mul_string(const std::string &a, const std::string &b) {
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

std::vector<long long> naive_mul_vector(const std::vector<long long>& x, const std::vector<long long>& y) {
    size_t n = x.size();
    std::vector<long long> res(2 * n, 0);
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            res[i + j] += x[i] * y[j];
        }
    }
    
    return res;
}