.PHONY: all clean

CXXFLAGS=-std=c++11 -Wall -Wextra -Wno-char-subscripts -O2 -g

LINK.o=$(LINK.cc)

all: main

main: main.o wat_array/wat_array.o wat_array/bit_array.o

wattest: wattest.o wat_array/wat_array.o wat_array/bit_array.o
