# Large Integer Multiplication: Naive Schoolbook Reference

This project provides a simple C++ implementation of the “schoolbook” (naive) multiplication algorithm for arbitrarily large nonnegative integers represented as decimal strings. It also includes a test harness that generates random operands and prints their product, so you can verify more advanced algorithms (Karatsuba, Toom–Cook, FFT‑based) against a known‑correct baseline.

---

## Features

- Generates random big integers of specified digit length (no leading zeros)  
- Multiplies two decimal strings in $O(n^2)$ time using the classic algorithm  
- Command‑line interface to specify number of tests and operand length  
- Plain‑text output: inputs and product for each test case  

---

## Requirements

- C++11 (or newer) compiler  
- Standard library headers:
  - `<iostream>`
  - `<string>`
  - `<vector>`
  - `<random>`
  - `<algorithm>`

No external dependencies.

---

## Building

```bash
g++ -std=c++11 -O2 -o naive naive.cpp
```

---

## Usage

```bash
./naive [NUM_TESTS] [DIGITS_PER_OPERAND]
```

- `NUM_TESTS` (optional): number of random test cases (default: 5)  
- `DIGITS_PER_OPERAND` (optional): length of each operand in digits (default: 1000)  

**Example:**

```bash
./naive 10 500
```

Runs 10 tests multiplying two 500‑digit numbers each.

---

## Output

For each test, prints:

```text
Test #1:
  A = <first random number>
  B = <second random number>
  A * B = <product>
```

Use the printed products to verify correctness of your Karatsuba, Toom–Cook, or FFT‑based implementations.

---

## Extending

- Integrate this harness into unit tests by capturing stdout and comparing results.  
- Adapt `random_bigint` to other bases (e.g., hex) if needed.  
- Increase performance by switching to more advanced algorithms once verified.  

---

## License

MIT License. Feel free to reuse and adapt.  
