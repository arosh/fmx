.PHONY: all clean

CXXFLAGS=-std=c++11 -Wall -Wextra -Wshadow -O2 -g

LINK.o=$(LINK.cc)

all: main bwt

main: main.o wat_array/wat_array.o wat_array/bit_array.o

wattest: wattest.o wat_array/wat_array.o wat_array/bit_array.o

bwt: bwt.o wat_array/wat_array.o wat_array/bit_array.o

clean:
	$(RM) *.o wat_array/*.o main wattest bwt
