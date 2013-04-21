.PHONY: all clean

CC = g++ -O0 -g -std=c++98 -pedantic -W -Wall -Wextra \
	-Wno-variadic-macros -Wno-long-long \
	-I../ -I../../include/ \
	-pthread -lgmpxx -lgmp

all: NumberTest RangeTest MemoryPlaceTest OperandToMemoryPlaceTest UtilityTest

gtest/libgtest.a:
	make -C gtest

NumberTest: NumberTest.cc ../Number.cc gtest/libgtest.a
	${CC} NumberTest.cc ../Number.cc gtest/libgtest.a -o $@

RangeTest: RangeTest.cc ../Range.cc ../Number.cc gtest/libgtest.a
	${CC} RangeTest.cc ../Range.cc ../Number.cc gtest/libgtest.a -o $@

MemoryPlaceTest: MemoryPlaceTest.cc ../MemoryPlace.cc gtest/libgtest.a
	${CC} MemoryPlaceTest.cc ../MemoryPlace.cc gtest/libgtest.a -o $@

OperandToMemoryPlaceTest: OperandToMemoryPlaceTest.cc ../OperandToMemoryPlace.cc gtest/libgtest.a
	${CC} OperandToMemoryPlaceTest.cc ../OperandToMemoryPlace.cc \
		../MemoryPlace.cc gtest/libgtest.a -o $@

UtilityTest: UtilityTest.cc ../Utility.cc ../Number.cc ../Range.cc gtest/libgtest.a
	${CC} UtilityTest.cc ../Utility.cc ../Number.cc ../Range.cc gtest/libgtest.a -o $@

clean:
	make -C gtest clean
	rm -f *.o *Test
