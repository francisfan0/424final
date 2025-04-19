#include "bigint_multiply.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

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

int main(int argc, char* argv[]) {
    size_t num_tests = 5;
    size_t length = 1000;
    
    if (argc >= 2) num_tests = std::stoul(argv[1]);
    if (argc >= 3) length = std::stoul(argv[2]);
    
    std::cout << "Running " << num_tests << " tests with " << length 
              << "-digit operands each.\n\n";
    
    double total_naive_time = 0;
    double total_karatsuba_time = 0;
    bool all_passed = true;
    
    for (size_t t = 1; t <= num_tests; ++t) {
        std::cout << "Test #" << t << ":\n";
        
        auto A = random_bigint(length);
        auto B = random_bigint(length);
        
        std::cout << "  A (" << A.size() << " digits) = " 
                  << truncate_display(A) << "\n";
        std::cout << "  B (" << B.size() << " digits) = " 
                  << truncate_display(B) << "\n";
        
        auto naive_start = std::chrono::high_resolution_clock::now();
        auto naive_result = naive_mul_string(A, B);
        auto naive_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> naive_elapsed = naive_end - naive_start;
        total_naive_time += naive_elapsed.count();
        
        auto karatsuba_start = std::chrono::high_resolution_clock::now();
        auto karatsuba_result = karatsuba_mul_string(A, B);
        auto karatsuba_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> karatsuba_elapsed = karatsuba_end - karatsuba_start;
        total_karatsuba_time += karatsuba_elapsed.count();
        
        bool test_passed = verify_result(A, B, naive_result, karatsuba_result);
        all_passed &= test_passed;
        
        std::cout << "  Naive result (" << naive_result.size() << " digits) = " 
                  << truncate_display(naive_result) << "\n";
        std::cout << "  Karatsuba result (" << karatsuba_result.size() << " digits) = " 
                  << truncate_display(karatsuba_result) << "\n";
        std::cout << "  Verification: " << (test_passed ? "PASSED" : "FAILED") << "\n";
        std::cout << "  Naive time: " << naive_elapsed.count() << " seconds\n";
        std::cout << "  Karatsuba time: " << karatsuba_elapsed.count() << " seconds\n";
        std::cout << "  Speedup: " << std::fixed << std::setprecision(2) 
                  << (naive_elapsed.count() / karatsuba_elapsed.count()) << "x\n\n";
    }
    
    std::cout << "Average Naive Time: " << (total_naive_time / num_tests) << " seconds\n";
    std::cout << "Average Karatsuba Time: " << (total_karatsuba_time / num_tests) << " seconds\n";
    std::cout << "Average Speedup: " << std::fixed << std::setprecision(2) 
              << (total_naive_time / total_karatsuba_time) << "x\n";
    std::cout << "All tests " << (all_passed ? "PASSED" : "FAILED") << "\n";
    
    return 0;
}