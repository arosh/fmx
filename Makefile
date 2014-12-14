.PHONY: all clean

CXXFLAGS=-std=c++11 -Wall -Wextra -Wshadow -O2 -march=native -g -pthread

LINK.o=$(LINK.cpp)

all: main mainmain bwt test

main: main.o wat_array/wat_array.o wat_array/bit_array.o

mainmain: mainmain.o wat_array/wat_array.o wat_array/bit_array.o module.o

bwt: bwt.o wat_array/wat_array.o wat_array/bit_array.o

test: bitvector.o wavelet_matrix.o gtest/gtest_main.o gtest/gtest-all.o
	$(LINK.o) $^ -o $@

bitvector.o: bitvector.cpp bitvector.h

wavelet_matrix.o: wavelet_matrix.cpp wavelet_matrix.h bitvector.h

clean:
	$(RM) *.o wat_array/*.o gtest/*.o main bwt test
