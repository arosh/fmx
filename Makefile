.PHONY: all clean

CXXFLAGS=-std=c++11 -Wall -Wextra -Wshadow -O2 -march=native -g -pthread
LDLIBS=-lmsgpack

LINK.o=$(LINK.cpp)

all: test preprocess

test: bitvector.o wavelet_matrix.o gtest/gtest_main.o gtest/gtest-all.o
	$(LINK.o) $^ -o $@

bitvector.o: bitvector.cpp bitvector.h

wavelet_matrix.o: wavelet_matrix.cpp wavelet_matrix.h bitvector.h

preprocess: preprocess.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	$(RM) *.o gtest/*.o test preprocess
