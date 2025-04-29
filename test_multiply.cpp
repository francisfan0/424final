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

bool verify_results(const std::string& a, const std::string& b,
                    const std::vector<std::string>& results,
                    const std::vector<Algorithm>& algorithms) {
    if (results.empty()) {
        return true; // Nothing to verify if no results
    }

    const std::string& first_result = results[0];
    bool all_match = true;

    for (size_t i = 1; i < results.size(); ++i) {
        if (results[i] != first_result) {
            all_match = false;
            std::cerr << "Verification failed: Result for " << algorithm_to_string(algorithms[i])
                      << " does not match " << algorithm_to_string(algorithms[0]) << ".\n";
        }
    }

    if (a.size() <= 9 && b.size() <= 9 && all_match) {
        try {
            long long a_ll = std::stoll(a);
            long long b_ll = std::stoll(b);
            long long expected = a_ll * b_ll;
            std::string expected_str = std::to_string(expected);
            if (first_result != expected_str) {
                 all_match = false;
                 std::cerr << "Verification failed: Results do not match built-in multiplication.\n";
                 std::cerr << "  Built-in: " << expected_str << std::endl;
                 std::cerr << "  Algorithm Result: " << first_result << std::endl;
            }
        } catch (const std::out_of_range& oor) {

        } catch (const std::invalid_argument& ia) {

        }
    }


    return all_match;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [num_tests] [digits_length] [algorithm1] [algorithm2] ...\n\n"
              << "  num_tests      - Number of test cases (default: 5)\n"
              << "  digits_length  - Length of random numbers (default: 1000)\n"
              << "  algorithm(s)   - One or more algorithms to compare (at least one required):\n"
              << "                   0: naive\n"
              << "                   1: karatsuba sequential\n"
              << "                   2: karatsuba parallel\n"
              << "                   3: toom cook sequential\n"
              << "                   4: toom cook parallel\n"
              << "                   (e.g., '4 1 2' to compare Toom Cook Parallel, Karatsuba Seq, Karatsuba Par)\n";
}

Algorithm parse_algorithm(int choice) {
    switch (choice) {
        case 0: return Algorithm::NAIVE;
        case 1: return Algorithm::KARATSUBA_SEQ;
        case 2: return Algorithm::KARATSUBA_PAR;
        case 3: return Algorithm::TOOM_COOK_SEQ;
        case 4: return Algorithm::TOOM_COOK_PAR;
        default: throw std::invalid_argument("Invalid algorithm choice: " + std::to_string(choice));
    }
}

int main(int argc, char* argv[]) {
    size_t num_tests = 5;
    size_t length = 1000;
    std::vector<Algorithm> algorithms;

    int arg_idx = 1;

    if (arg_idx < argc) {
        if (std::string(argv[arg_idx]) == "-h" || std::string(argv[arg_idx]) == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        try {
            num_tests = std::stoul(argv[arg_idx]);
            arg_idx++;
        } catch (...) {

        }
    }

    if (arg_idx < argc) {
         try {
            length = std::stoul(argv[arg_idx]);
            arg_idx++;
        } catch (...) {

        }
    }

    while (arg_idx < argc) {
        try {
            algorithms.push_back(parse_algorithm(std::stoi(argv[arg_idx])));
            arg_idx++;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: " << e.what() << ". Stopping argument parsing.\n";
            break; // Stop parsing if an invalid algorithm is found
        } catch (...) {
             std::cerr << "Error parsing argument " << argv[arg_idx] << ". Stopping argument parsing.\n";
             break;
        }
    }

    if (algorithms.empty()) {
        std::cout << "No algorithms specified, defaulting to Naive and Karatsuba Parallel.\n";
        algorithms.push_back(Algorithm::NAIVE);
        algorithms.push_back(Algorithm::KARATSUBA_PAR);
    }

    std::cout << "Running " << num_tests << " tests with " << length
              << "-digit operands comparing:";
    for (size_t i = 0; i < algorithms.size(); ++i) {
        std::cout << " " << algorithm_to_string(algorithms[i]) << (i == algorithms.size() - 1 ? "" : ",");
    }
    std::cout << ".\n\n";

    std::vector<double> total_times(algorithms.size(), 0.0);
    bool all_tests_passed = true;

    for (size_t t = 1; t <= num_tests; ++t) {
        std::cout << "Test #" << t << ":\n";

        auto A = random_bigint(length);
        auto B = random_bigint(length);

        std::cout << "  A (" << A.size() << " digits) = "
                  << truncate_display(A) << "\n";
        std::cout << "  B (" << B.size() << " digits) = "
                  << truncate_display(B) << "\n";

        std::vector<std::string> results;
        std::vector<double> times;

        auto run_single_algorithm = [&](Algorithm alg) -> std::pair<std::string, double> {
            std::string result;
            auto start = std::chrono::high_resolution_clock::now();

            switch (alg) {
                case Algorithm::NAIVE:
                    result = naive_mul_string(A, B);
                    break;
                case Algorithm::KARATSUBA_SEQ:
                    result = karatsuba_mul_string(A, B);
                    break;
                case Algorithm::KARATSUBA_PAR:
                    result = par_karatsuba_mul_string(A, B);
                    break;
                case Algorithm::TOOM_COOK_SEQ:
                    result = toom_cook_mul_string(A, B);
                    break;
                case Algorithm::TOOM_COOK_PAR:
                    result = par_toom_cook_mul_string_plib(A, B);
                    break;
                default:
                    result = "Error: Unknown Algorithm";
                    break;
            }

            auto end = std::chrono::high_resolution_clock::now();
            double elapsed_time = std::chrono::duration<double>(end - start).count();
            return {result, elapsed_time};
        };

        for (size_t i = 0; i < algorithms.size(); ++i) {
            auto [result, time] = run_single_algorithm(algorithms[i]);
            results.push_back(result);
            times.push_back(time);
            total_times[i] += time;

            std::cout << "  " << algorithm_to_string(algorithms[i]) << " result (" << result.size() << " digits) = "
                      << truncate_display(result) << "\n";
            std::cout << "  Time: " << std::fixed << std::setprecision(6) << time << " seconds\n";
        }

        bool passed = verify_results(A, B, results, algorithms);
        all_tests_passed &= passed;
        std::cout << "  Verification: " << (passed ? "PASSED" : "FAILED") << "\n";

        if (algorithms.size() > 1 && times[0] > 0) {
             std::cout << "  Speedups (vs " << algorithm_to_string(algorithms[0]) << "):";
             for(size_t i = 1; i < algorithms.size(); ++i) {
                 std::cout << " " << algorithm_to_string(algorithms[i]) << ": "
                           << std::fixed << std::setprecision(2) << (times[0] / times[i]) << "x"
                           << (i == algorithms.size() - 1 ? "" : ",");
             }
             std::cout << "\n";
        }


        std::cout << "\n";
    }

    std::cout << "=== SUMMARY ===\n";
    for (size_t i = 0; i < algorithms.size(); ++i) {
        std::cout << "Average " << algorithm_to_string(algorithms[i]) << " Time: "
                  << std::fixed << std::setprecision(6) << (total_times[i] / num_tests) << " seconds\n";
    }

    if (algorithms.size() > 1 && total_times[0] > 0) {
         std::cout << "Overall Average Speedups (vs " << algorithm_to_string(algorithms[0]) << "):";
         for(size_t i = 1; i < algorithms.size(); ++i) {
             std::cout << " " << algorithm_to_string(algorithms[i]) << ": "
                       << std::fixed << std::setprecision(2) << (total_times[0] / total_times[i]) << "x"
                       << (i == algorithms.size() - 1 ? "" : ",");
         }
         std::cout << "\n";
    }


    std::cout << "All tests " << (all_tests_passed ? "PASSED" : "FAILED") << "\n";

    return 0;
}
