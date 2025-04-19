CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

SOURCES = naive.cpp seq_karatsuba.cpp utils.cpp test_multiply.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = multiply_test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp bigint_multiply.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean