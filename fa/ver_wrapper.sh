#!/bin/bash

################################# CONSTANTS ##################################

TOOL=$1
FILE=$2

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f $0`

# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=`dirname $SCRIPT`

# Forester executable
FORESTER="${SCRIPTPATH}/../fa_build/fagcc"

# Predator executable
PREDATOR="${SCRIPTPATH}/../sl_build/slgcc"

################################# FUNCTIONS ##################################

# Function that terminates the script with a message
function die {
  echo "$1";
  exit -1;
}

################################## PROGRAM ###################################

if [ "$#" -ne 2 ]; then
  die "usage: $0 <method> <file>"
fi

RETVAL="?"

EXEC_STRING="<unknown>"

case "${TOOL}" in
  forester)
    EXEC_STRING="${FORESTER} \"${FILE}\""
    ;;
  predator)
    EXEC_STRING="${PREDATOR} \"${FILE}\""
    ;;
  *) die "Invalid option ${TOOL}"
    ;;
esac

TIMEFORMAT="%U"
RUNNING_TIME=`eval "time ${EXEC_STRING} >/dev/null 2>&1" 2>&1`
RETVAL="$?"

echo "${RUNNING_TIME}"

exit ${RETVAL}
