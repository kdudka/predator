#!/bin/bash

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 source-file [gcc-options]" >&2
  exit 1
fi

GCC="g++-4.5.0"
SOURCE_FILE="$1"

shift 1

GCC_PARAMS="$*"

if [[ -z "$GCC_PARAMS" ]]; then
  GCC_PARAMS="-std=c++98 -pedantic -Wall -Wextra -g -O0 -c"
fi

$GCC $GCC_PARAMS "$SOURCE_FILE" -o /dev/null

exit $?
