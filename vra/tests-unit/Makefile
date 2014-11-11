.PHONY: all clean

CXX ?= g++
CXXFLAGS ?= -O0 -g -std=c++98 -pedantic -W -Wall -Wextra \
	-Wno-variadic-macros -Wno-long-long \
	-I../ -I../../include/ \
	-pthread -lgmpxx -lgmp

all: NumberTest RangeTest MemoryPlaceTest OperandToMemoryPlaceTest UtilityTest

gtest/libgtest.a:
	make -C gtest

NumberTest: NumberTest.cc ../Number.cc gtest/libgtest.a
	$(CXX) NumberTest.cc ../Number.cc gtest/libgtest.a -o $@ $(CXXFLAGS)

RangeTest: RangeTest.cc ../Range.cc ../Number.cc gtest/libgtest.a
	$(CXX) RangeTest.cc ../Range.cc ../Number.cc gtest/libgtest.a -o $@ $(CXXFLAGS)

MemoryPlaceTest: MemoryPlaceTest.cc ../MemoryPlace.cc gtest/libgtest.a
	$(CXX) MemoryPlaceTest.cc ../MemoryPlace.cc gtest/libgtest.a -o $@ $(CXXFLAGS)

OperandToMemoryPlaceTest: OperandToMemoryPlaceTest.cc ../OperandToMemoryPlace.cc gtest/libgtest.a
	$(CXX) OperandToMemoryPlaceTest.cc ../OperandToMemoryPlace.cc \
		../MemoryPlace.cc gtest/libgtest.a -o $@ $(CXXFLAGS)

UtilityTest: UtilityTest.cc ../Utility.cc ../Number.cc ../Range.cc gtest/libgtest.a
	$(CXX) UtilityTest.cc ../Utility.cc ../Number.cc ../Range.cc gtest/libgtest.a -o $@ $(CXXFLAGS)

clean:
	make -C gtest clean
	rm -f *.o *Test
