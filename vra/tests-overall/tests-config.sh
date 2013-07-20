#
# Encoding: utf-8
# Author:   Daniela Duricekova, xduric00@stud.fit.vutbr.cz
# Date:     2013-04-13
#
# A configuration file for the tests.
#

# Path to the tests (without ending '/').
TESTS_PATH="./"

# Use $GCC_HOST from environment if set, local GCC build otherwise.
test -n "$GCC_HOST" || GCC_HOST="../../gcc-install/bin/gcc"

# Command that runs the analysis. The tests are run by using
# `$ANALYSIS_CMD $TEST_FILE`.
ANALYSIS_CMD="$GCC_HOST -fplugin=../../vra_build/libvra.so"

# Regular expression describing test files (format is that of `find`).
TEST_REGEXP="test-[0-9]*.c"

# Number of columns after which the status message (OK/FAIL) should be emitted.
INDENT=20
