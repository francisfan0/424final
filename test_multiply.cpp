#include "bigint_multiply.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>
#include <omp.h>

enum class Algorithm {
    NAIVE = 0,
    KARATSUBA_SEQ = 1,
    KARATSUBA_PAR = 2,
    TOOM_COOK_SEQ = 3,
    TOOM_COOK_PAR = 4,
};

bool verify_result(const std::string& a, const std::string& b, 
                  const std::string& result1, const std::string& result2) {
    if (result1 == result2) {
        return true;
    }

    if (a.size() <= 9 && b.size() <= 9) {
        long long a_ll = std::stoll(a);
        long long b_ll = std::stoll(b);
        long long expected = a_ll * b_ll;
        std::string expected_str = std::to_string(expected);

        std::cout << "Built-in: " << expected_str << std::endl;
        std::cout << "Result 1: " << result1 << std::endl;
        std::cout << "Result 2: " << result2 << std::endl;

        return (expected_str == result1) && (expected_str == result2);
    }

    return false;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [num_tests] [digits_length] [algorithm1] [algorithm2]\n\n"
              << "  num_tests      - Number of test cases (default: 5)\n"
              << "  digits_length  - Length of random numbers (default: 1000)\n"
              << "  algorithm1/2   - Algorithms to compare:\n"
              << "                   0: naive\n"
              << "                   1: karatsuba sequential\n"
              << "                   2: karatsuba parallel\n"
              << "                   3: toom cook sequential\n"
              << "                   4: toom cook parallel\n";
}

Algorithm parse_algorithm(int choice) {
    switch (choice) {
        case 0: return Algorithm::NAIVE;
        case 1: return Algorithm::KARATSUBA_SEQ;
        case 2: return Algorithm::KARATSUBA_PAR;
        case 3: return Algorithm::TOOM_COOK_SEQ;
        case 4: return Algorithm::TOOM_COOK_PAR;
        default: throw std::invalid_argument("Invalid algorithm choice");
    }
}

std::string algorithm_to_string(Algorithm alg) {
    switch (alg) {
        case Algorithm::NAIVE: return "Naive";
        case Algorithm::KARATSUBA_SEQ: return "Karatsuba Sequential";
        case Algorithm::KARATSUBA_PAR: return "Karatsuba Parallel";
        case Algorithm::TOOM_COOK_SEQ: return "Toom Cook Sequential";
        case Algorithm::TOOM_COOK_PAR: return "Toom Cook Parallel";
        default: return "Unknown";
    }
}

int main(int argc, char* argv[]) {
    size_t num_tests = 5;
    size_t length = 1000;
    Algorithm alg1 = Algorithm::NAIVE;
    Algorithm alg2 = Algorithm::KARATSUBA_PAR;

    if (argc >= 2) {
        if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        num_tests = std::stoul(argv[1]);
    }

    if (argc >= 3) length = std::stoul(argv[2]);

    if (argc >= 4) {
        try {
            alg1 = parse_algorithm(std::stoi(argv[3]));
        } catch (...) {
            std::cerr << "Invalid first algorithm, defaulting to Naive.\n";
        }
    }

    if (argc >= 5) {
        try {
            alg2 = parse_algorithm(std::stoi(argv[4]));
        } catch (...) {
            std::cerr << "Invalid second algorithm, defaulting to Karatsuba Parallel.\n";
        }
    }

    std::cout << "Running " << num_tests << " tests with " << length 
              << "-digit operands comparing " << algorithm_to_string(alg1) 
              << " and " << algorithm_to_string(alg2) << ".\n\n";

    double total_time1 = 0;
    double total_time2 = 0;
    bool all_passed = true;

    for (size_t t = 1; t <= num_tests; ++t) {
        std::cout << "Test #" << t << ":\n";

        auto A = random_bigint(length);
        auto B = random_bigint(length);

        std::cout << "  A (" << A.size() << " digits) = " 
                  << truncate_display(A) << "\n";
        std::cout << "  B (" << B.size() << " digits) = " 
                  << truncate_display(B) << "\n";

        std::string result1, result2;
        double time1 = 0, time2 = 0;

        auto run_algorithm = [&](Algorithm alg, std::string& result, double& elapsed_time) {
            auto start = std::chrono::high_resolution_clock::now();

            if (alg == Algorithm::NAIVE) {
                result = naive_mul_string(A, B);
            } else {
                if (alg == Algorithm::KARATSUBA_SEQ) {
                    // omp_set_num_threads(1);
                    result = karatsuba_mul_string(A, B);
                }
                if (alg == Algorithm::KARATSUBA_PAR) {
                    // std::cout << "  Using " << omp_get_max_threads() << " threads\n";
                    result = par_karatsuba_mul_string(A, B);
                }
                if (alg == Algorithm::TOOM_COOK_SEQ) {
                    result = toom_cook_mul_string(A, B);
                }
                if (alg == Algorithm::TOOM_COOK_PAR) {
                    result = par_toom_cook_mul_string(A, B);
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            elapsed_time = std::chrono::duration<double>(end - start).count();
        };

        run_algorithm(alg1, result1, time1);
        total_time1 += time1;
        std::cout << "  " << algorithm_to_string(alg1) << " result (" << result1.size() << " digits) = " 
                  << truncate_display(result1) << "\n";
        std::cout << "  Time: " << time1 << " seconds\n";

        run_algorithm(alg2, result2, time2);
        total_time2 += time2;
        std::cout << "  " << algorithm_to_string(alg2) << " result (" << result2.size() << " digits) = " 
                  << truncate_display(result2) << "\n";
        std::cout << "  Time: " << time2 << " seconds\n";

        bool passed = verify_result(A, B, result1, result2);
        all_passed &= passed;
        std::cout << "  Verification: " << (passed ? "PASSED" : "FAILED") << "\n";
        std::cout << "  Speedup: " << std::fixed << std::setprecision(2)
                  << (time1 / time2) << "x (" << algorithm_to_string(alg1) 
                  << " vs " << algorithm_to_string(alg2) << ")\n";

        std::cout << "\n";
    }

    std::cout << "=== SUMMARY ===\n";
    std::cout << "Average " << algorithm_to_string(alg1) << " Time: " << (total_time1 / num_tests) << " seconds\n";
    std::cout << "Average " << algorithm_to_string(alg2) << " Time: " << (total_time2 / num_tests) << " seconds\n";
    std::cout << "Average Speedup: " << std::fixed << std::setprecision(2)
              << (total_time1 / total_time2) << "x\n";
    std::cout << "All tests " << (all_passed ? "PASSED" : "FAILED") << "\n";

    return 0;
}
