# Large Integer Multiplication

This project provides C++ implementations of large integer multiplication algorithms using decimal strings. It includes both the “schoolbook” (naive) algorithm and an optimized Karatsuba method, along with a benchmark/test harness to compare their performance and correctness.

---

## Features

- Randomly generates large integers of a specified digit length (no leading zeros)  
- Multiplies two decimal strings using:
  - Naive (grade-school) algorithm: $O(n^2)$  
  - Karatsuba multiplication: $O(n^{\log_2 3}) \approx O(n^{1.585})$  
- Compares and verifies both algorithms for correctness  
- Reports detailed performance statistics and speedup  
- Command-line interface to configure number of tests and operand size  

---

## Requirements

- C++17 or newer  
- Standard library headers:
  - `<iostream>`
  - `<string>`
  - `<vector>`
  - `<chrono>`
  - `<iomanip>`
  - `<random>`
  - `<algorithm>`

No external dependencies.

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
./multiply_test [NUM_TESTS] [DIGITS_PER_OPERAND]
```

- `NUM_TESTS` (optional): number of test cases to run (default: 5)  
- `DIGITS_PER_OPERAND` (optional): length of each operand (default: 1000 digits)  

**Example:**

```bash
./multiply_test 10 500
```

Runs 10 tests of multiplying two randomly generated 500-digit integers.

---

## Output

For each test case, the program prints:

- Truncated representations of input operands  
- Results from both the naive and Karatsuba algorithms  
- Whether results match (either directly or via small-size verification)  
- Timing for each algorithm and the calculated speedup  

**Example output:**

```
Test #1:
  A (500 digits) = 39492...
  B (500 digits) = 20348...
  Naive result (999 digits) = 80325...
  Karatsuba result (999 digits) = 80325...
  Verification: PASSED
  Naive time: 0.312 seconds
  Karatsuba time: 0.087 seconds
  Speedup: 3.58x
```

---

## License

MIT License — feel free to reuse, modify, and share.
