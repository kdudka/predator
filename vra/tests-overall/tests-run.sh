#!/bin/bash
#
# Encoding: utf-8
# Author:	Daniela Duricekova, xduric00@stud.fit.vutbr.cz
# Date:		2013-04-13
#
# Runs the tests.
#

. tests-config.sh
. tests-utils.sh

# Remove all .out files before we start testing.
find "$TESTS_PATH" -name "*.out.*" -exec rm -f {} \;

# Number of passed tests.
num_of_passed=0

# Number of failed tests.
num_of_failed=0

# Run the analyzer on all existing tests to obtain outputs, which will be then
# compared to the reference outputs.
echo "Running the analyzer on tests in '$TESTS_PATH'..."
echo ""
for test_file in $(find "$TESTS_PATH" -name "$TEST_REGEXP" | sort); do
	# Generate and emit the test's name.
	test_name=`basename ${test_file##$TESTS_PATH}`
	echo_colored "$test_name" "yellow"

	# Generate the reference and output file names.
	ref_file="$test_file"".ref"
	out_file="$test_file"".out"

	# Run the analyzer to obtain the output.
	$ANALYSIS_CMD $test_file 2> /dev/null > $out_file
	if [ ! -e "$out_file" ]; then
		# There was an error during the analysis, so skip the comparison to the
		# reference output.
		emit_indent $test_name
		echo_colored "[ERROR] (no output)" "red"
		echo ""
		continue
	fi

	# Check whether the obtained output matches the reference output.
	evaluate_result "$out_file" "$ref_file" "$test_name"
	if [ $? -eq 0 ]; then
		num_of_passed=`expr $num_of_passed + 1`
	else
		num_of_failed=`expr $num_of_failed + 1`
	fi

	echo ""
done
echo ""

# Emit statistics.
total=`expr $num_of_passed + $num_of_failed`
echo "  Passed: $num_of_passed/$total"
echo "  Failed: $num_of_failed/$total"
echo ""

exit 0
