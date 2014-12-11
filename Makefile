.PHONY: all clean

CXXFLAGS=-std=c++11 -Wall -Wextra -Wshadow -O2 -g

LINK.o=$(LINK.cpp)

all: main bwt bitvector

main: main.o wat_array/wat_array.o wat_array/bit_array.o

wattest: wattest.o wat_array/wat_array.o wat_array/bit_array.o

bwt: bwt.o wat_array/wat_array.o wat_array/bit_array.o

bitvector: bitvector.o gtest/gtest_main.o gtest/gtest-all.o

bitvector.o: bitvector.cpp bitvector.h

clean:
	$(RM) *.o wat_array/*.o main wattest bwt bitvector gtest/*.o
