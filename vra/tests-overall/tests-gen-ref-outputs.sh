#!/bin/bash
#
# Encoding: utf-8
# Author:   Daniela Duricekova, xduric00@stud.fit.vutbr.cz
# Date:     2013-04-13
#
# Generates reference test outputs.
#

. tests-config.sh
. tests-utils.sh

# Number of tests.
num_of_tests=0

# Run the analyzer on all existing tests and generate the reference outputs.
echo "Generating the reference outputs for tests in '$TESTS_PATH'..."
echo ""
for test_file in $(find "$TESTS_PATH" -name "$TEST_REGEXP" | sort); do
	# Generate and emit the test's name.
	test_name=`basename ${test_file##$TESTS_PATH}`
	echo_colored "$test_name" "yellow"

	# Generate the reference file name.
	ref_file="$test_file"".ref"

	# Run the analyzer to obtain the reference output.
	$ANALYSIS_CMD $test_file 2> /dev/null > $ref_file
	if [ ! -e "$ref_file" ]; then
		# There was an error during the analysis.
		emit_indent $test_name
		echo_colored "[ERROR] (no output)" "red"
		echo ""
		continue
	fi

	emit_indent $test_name
	echo_colored "[OK]" "green"

	num_of_tests=`expr $num_of_tests + 1`

	echo ""
done
echo ""

# Emit statistics.
echo "Generated reference outputs for $num_of_tests tests."

exit 0
