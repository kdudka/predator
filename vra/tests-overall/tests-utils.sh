#
# Encoding: utf-8
# Author:	Daniela Duricekova, xduric00@stud.fit.vutbr.cz
# Date:		2013-04-13
#
# Utilits for the tests.
#

. tests-config.sh

##
## Emits a proper number of spaces so a status message can be emitted after it.
##
## 1 string argument is needed:
##	 $1 test name
##
## If $1 is empty, no spaces are emitted.
emit_indent() {
	# Check the number of arguments.
	if [ "$#" != "1" ]; then
		return
	fi

	# Is the test name empty?
	if [ -z "$1" ]; then
		return
	fi

	test_name_len=`expr length $1`
	spaces=$(echo "$INDENT-$test_name_len" | bc)
	for i in `seq 1 $spaces`; do
		echo -n " "
	done
}

##
## Performs a diff of the two given files emits the result on standard output.
##
## 4 string arguments are needed:
##	 $1 first file	(out)
##	 $2 second file (ref)
##	 $3 test name
##
## Returns 0 if the diff is empty, 1 otherwise.
##
evaluate_result() {
	# Check the number of arguments.
	if [ "$#" != "3" ]; then
		return 1
	fi

	emit_indent $3

	# Check that the files we're diffing exist.
	if [ ! -e "$1" ]; then
		echo_colored "[ERROR] .out file does not exist" "red"
		return 1
	fi
	if [ ! -e "$2" ]; then
		echo_colored "[ERROR] .ref file does not exist" "red"
		return 1
	fi

	diff=`diff "$1" "$2"`
	if [ -z "$diff" ]; then
		echo_colored "[OK]" "green"
		return 0
	else
		echo_colored "[FAIL]" "red"
		echo ""
		echo "$diff" | sed 's/^/  /'
		return 1
	fi
}

##
## Emits a colored version of the given message to the standard output.
##
##
## 2 string argument are needed:
##	 $1 message to be colored
##	 $2 color (red, green, yellow)
##
## If the color is unknown, this function emits just $1. No new line is
## appended.
##
echo_colored() {
	# Check the number of arguments.
	if [ "$#" != "2" ]; then
		return
	fi

	case $2 in
	"red")
		printf "\033[22;31m$1\033[0m"
		;;
	"green")
		printf "\033[22;32m$1\033[0m"
		;;
	"yellow")
		printf "\033[01;33m$1\033[0m"
		;;
	*)
		printf "$1\n"
		;;
	esac
}
