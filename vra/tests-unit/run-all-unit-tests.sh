#!/bin/bash
#
# Encoding: utf-8
# Author:	Daniela Duricekova, xduric00@stud.fit.vutbr.cz
# Date:		2013-04-13
#
# Runs all the unit tests.
#

# Make sure that all tests are built.
echo "Building the unit tests..."
make

# Run them (show only failures).
for test in NumberTest RangeTest MemoryPlaceTest OperandToMemoryPlaceTest UtilityTest; do
	echo ""
	echo "Running $test..."
	./$test --gtest_color=yes | grep -v "RUN\|OK\|----------\|=========="
done

exit 0
