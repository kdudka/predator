#!/bin/bash
export SELF="$0"
export LC_ALL=C
MAKE="make -j5"

# Config file used by the CL:
# NOTE: Make sure the "$CL_CONFIG_FILE.default" exists!
CL_CONFIG_FILE="./cl/config_cl.h"

# Current supported settings of CL:
CL_SETTINGS=(CL_DEBUG_COLORED_OUTPUT
             CL_DEBUG_CLF
             CL_DEBUG_GCC_GIMPLE
             CL_DEBUG_GCC_TREE
             CL_DEBUG_LOCATION
             CL_DEBUG_LOOP_SCAN
             CL_DEBUG_POINTS_TO
             CL_DEBUG_VAR_KILLER
             CL_EASY_TIMER
             CL_MSG_SQUEEZE_REPEATS
             CLPLUG_SILENT
             DEBUG_CL_FACTORY)

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s ANALYZER [CLANG_HOST OPT_HOST]\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script for checking plug-ins. 

    ANALYZER is analyzer cl Code Listener
                         sl Predator
                         fa Forester 

    CLANG_HOST is the absolute path to clang(1). Default is /usr/bin/clang.
    If you have multiple versions of clang/llvm installed on the system, it 
    can be something like /usr/bin/clang-3.3. You can also provide a local 
    build of clang, e.g. /home/bob/clang+llvm-3.3-x86_64/bin/clang.

    OPT_HOST is the absolute path to opt. Mainly in the same folder as clang.
    Default is /usr/bin/opt.

EOF
    exit 1
}

# Tests if the option is set and updates the value if necessary:
option_set() {
  if [[ -z $1 ]]; then
    return 1;             # Not set.
  fi

  # Obtaining the value of the option:
  eval "OPTION_VALUE=\$$1"
  OPTION_VALUE="$(echo $OPTION_VALUE | tr [:upper:] [:lower:])"

  case $OPTION_VALUE in
    "on" | "true")
      OPTION_VALUE=1
      return 0;;

    "off" | "false")
      OPTION_VALUE=0
      return 0;;

    # NOTE - maximum debug value currently supported is: 4
    "0" | "1" | "2" | "3" | "4" )
      return 0;;

    *)
      return 1;;          # Something unknown ->> ignoring.
  esac
}

# Prepares the CL in correspondence with user's environment settings:
prepare_CL_environment() {
  # Restores the default environment first:
  if [[ -f "$CL_CONFIG_FILE.default" ]]; then
    cp "$CL_CONFIG_FILE.default" "$CL_CONFIG_FILE"
  else
    return
  fi

  # Sets the new environment. If the user's environment is clear, than it does
  # nothing...
  # NOTE: Do not break the sed line below, the regular expression can be highly
  #       error prone, thanks!
  for option in "${CL_SETTINGS[@]}"; do
    if option_set $option; then
      sed -r -i -e 's/(#define[[:space:]]+)('$option')([[:space:]]+)([[:digit:]])/\1\2\3'$OPTION_VALUE'/g' "$CL_CONFIG_FILE"
    fi
  done

  return
}

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

# Prepare the CL environment for the compilation:
prepare_CL_environment

# Turn the NDEBUG off if requested:
# NOTE: '$' is intentionally missing before 'CL_DEBUG'
if option_set CL_DEBUG && test $OPTION_VALUE -eq 1; then
  CL_DEBUG_STRING="-D CL_DEBUG=ON";
fi

if [ "$#" == 3 ]; then

    test cl = "$1" || test sl = "$1" || test fa = "$1" || usage

    HOST="$2"
    OPT_HOST="$3"

    # check the given HOST
    test "/" == "${HOST:0:1}" \
        || die "CLANG_HOST is not an absolute path: $HOST"
    test -x "$HOST" \
        || die "HOST is not an absolute path to an executable file: $HOST"

    # try to run CLANG_HOST
    "$HOST" --version || die "unable to run clang: $HOST --version"

    test "/" == "${OPT_HOST:0:1}" \
        || die "OPT_HOST is not an absolute path: $OPT_HOST"
    test -x "$OPT_HOST" \
        || die "OPT_HOST is not an absolute path to an executable file: $OPT_HOST"

    status_update "Checking whether $1 works"
    $MAKE -C $1 check CMAKE="cmake -D CLANG_HOST='$HOST' -D OPT_HOST='$OPT_HOST' -D ENABLE_LLVM=ON -D GCC_EXEC_PREFIX='timeout 5' $CL_DEBUG_STRING"

else
    test 1 = "$#" || usage

    test cl = "$1" || test sl = "$1" || test fa = "$1" || usage

    status_update "Checking whether $1 works"
    $MAKE -C $1 check CMAKE="cmake -D ENABLE_LLVM=ON -D GCC_EXEC_PREFIX='timeout 5' $CL_DEBUG_STRING"
fi
