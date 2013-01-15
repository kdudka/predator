#!/bin/bash

################################# CONSTANTS ##################################

# default timeout (can be overriden from command line)
timeout=30;

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

METHODS=( 
  "forester"
  "predator"
)
################################# FUNCTIONS ##################################

# Function that terminates the script with a message
function die {
  echo "$1";
  exit -1;
}

# Function that runs a single test
# takes:   <source file>  <method>   <file to write time>
function runone {
  ulimit -St "${timeout}";
  ${SCRIPTPATH}/ver_wrapper.sh "$1" "$2" > "$3"
  local ret=$?

  return $ret
}

# Function that runs all tests for one source file
function runall {

  # Set trap for terminating the script
  trap killscripts INT TERM EXIT

  local res="?";
  local i;

  # run all methods
  for (( i=0; i < ${#METHODS[*]}; i++ )); do
    runone "${METHODS[$i]}" "$1" "${tmpTime[$i]}" &
    pid[$i]=$!;
  done;

  # evaluate all methods
  for (( i=0; i < ${#METHODS[*]}; i++ )); do
    wait ${pid[$i]};
    local ret=$?;
    pid[$i]=0;
    [ "$ret" == "0" ] || die "verification failed! ($ret)";

    local running_time=`cat "${tmpTime[$i]}"`;
    local time_diff=`echo "$timeout - $running_time" | bc | xargs printf "%1.0f"`

    if [ "$time_diff" == "0" ] ; then
      printcolumn "-"
    else
      num=$(printf "%.2f" "${running_time}")
      printcolumn "${num}";
    fi
  done;
  printf "\n"
}

# Prints a file column into the result table
function printfilecolumn {
  printf "%-30s;" "$1"
}

# Prints a column into the result table
function printcolumn {
  printf "%10s;" "$1"
}

function killscripts {
  local i

#  for i in $(ps | grep "\(vata\|sfta\)" | cut -d' ' -f1); do
#    kill ${i}
#  done
}

################################## PROGRAM ###################################

# Check the number of command-line arguments
if [ \( "$#" -eq 0 \) -o \( "$#" -gt 2 \) ] ; then
  die "usage: $0 <dir> [timeout]"
fi 

# If timeout is specified, use it instead of the default
if [ "$2" != "" ]; then
  timeout="$2";
fi;

VER_DIR="$1"

# If the directory doesn't exist, terminate
if [ ! -d "${VER_DIR}" ]; then
  die "directory '${VER_DIR}' does not exist!"
fi

echo "======== Verifying programs ======="
echo "Programs directory:   ${1}"
echo "Timeout:              ${timeout} s"

# Create temporary files for storing the output
for (( i=0; i < ${#METHODS[*]}; i++ )); do
  tmpTime[$i]=`mktemp`;
done;

echo "-----------------------------------------------"
printfilecolumn "file"

for (( i=0; i < ${#METHODS[*]}; i++ )); do
  printcolumn "${METHODS[$i]}"
done;

printf "\n"

for i in ${VER_DIR}/* ; do
  printfilecolumn "`basename ${i}`"
  runall "${i}"
done;

# Remove temporary files
for (( i=0; i < ${#METHODS[*]}; i++ )); do
  rm ${tmpTime[$i]}
done;
