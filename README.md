# Large Integer Multiplication

This project provides C++ implementations of large integer multiplication algorithms that can multiply decimal strings. It includes the "schoolbook" (naive) algorithm, a sequential Karatsuba implementation, a parallel Karatsuba implementation, a sequential 3-way Toom-Cook implementation, and a parallel 3-way Toom-Cook implementation. The project also includes a benchmark/test harness to compare the algorithms' performance and correctness.

---

## Features

- Randomly generates large integers of a specified digit length (no leading zeros)  
- Multiplies two decimal strings using:
  - Naive (grade-school) algorithm: $O(n^2)$  
  - Sequential Karatsuba multiplication: $O(n^{\log_2 3}) \approx O(n^{1.585})$
  - Parallel Karatsuba multiplication: Parallelized version using OpenMP / ParlayLib
  - Sequential 3-way Toom-Cook multiplication: $O(n^{\log_3 5}) \approx O(n^{1.465})$
  - Parallel 3-way Toom-Cook multiplication: Parallelized version using OpenMP / ParlayLib
- Compares and verifies algorithms for correctness  
- Reports detailed performance statistics and speedup  
- Command-line interface to configure number of tests, operand size, and algorithm selection

---

## Requirements

- C++17 or newer  
- OpenMP for parallel implementation
- Standard library headers:
  - `<iostream>`
  - `<string>`
  - `<vector>`
  - `<chrono>`
  - `<iomanip>`
  - `<random>`
  - `<algorithm>`
  - `<omp.h>` (for parallel implementation)

---

## Building

Use the included `Makefile`:

```bash
make
```

This compiles all sources and produces an executable named `multiply_test`.

You can also clean up build artifacts with:

```bash
make clean
```

---

## Usage

```bash
./multiply_test [NUM_TESTS] [DIGITS_PER_OPERAND] [ALGORITHM]
```

- `NUM_TESTS` (optional): number of test cases to run (default: 5)  
- `DIGITS_PER_OPERAND` (optional): length of each operand (default: 1000 digits)  
- `ALGORITHM` (optional): algorithm to use for multiplication:
  - `0`: Naive algorithm
  - `1`: Sequential Karatsuba
  - `2`: Parallel Karatsuba
  - `3`: Sequential Toom-Cook
  - `4`: Parallel Toom-Cook

**Examples:**

```bash
./multiply_test 10 500 0 1 2 3 4
```

Runs 10 tests of multiplying two randomly generated 500-digit integers using all algorithms.

```bash
./multiply_test 3 2000 0 2
```

Runs 3 tests of multiplying two randomly generated 2000-digit integers while comparing naive to the parallel Karatsuba algorithm.

```bash
./multiply_test -h
```

Displays help information.

---

## Output

For each test case, the program prints:

- Truncated representations of input operands  
- Results from the selected algorithm(s)  
- Result correctness
- Average execution time for each algorithm and the calculated speedup  

**Example output with all algorithms:**

```
Test #1:
  A (500 digits) = 39492...
  B (500 digits) = 20348...
  Naive result (999 digits) = 80325...
  Karatsuba sequential result (999 digits) = 80325...
  Sequential verification: PASSED
  Sequential speedup: 3.58x
  Using 8 threads for parallel execution
  Karatsuba parallel result (999 digits) = 80325...
  Parallel vs Naive verification: PASSED
  Parallel vs Naive speedup: 5.12x
  Parallel vs Sequential verification: PASSED
  Parallel vs Sequential speedup: 1.43x
```

The program also outputs a summary of average times and speedups after all tests are completed.

---

## License

MIT License — feel free to reuse, modify, and share.