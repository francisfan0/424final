# Example Makefile update
CC = g++
CFLAGS = -O3 -std=c++17 -Wall -Wextra -fopenmp
OBJECTS = naive.o seq_karatsuba.o utils.o test_multiply.o par_karatsuba.o seq_toom_cook.o par_toom_cook.o par_toom_cook_plib.o

multiply_test: $(OBJECTS)
	$(CC) $(CFLAGS) -o multiply_test $(OBJECTS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o multiply_test naive