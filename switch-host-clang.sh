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
    printf "Usage: %s CLANG_HOST\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator and/or Forester against an arbitrary
    build of host Clang/LLVM. The host Clang/LLVM needs library and header files
    for develop native programs that use the LLVM infrastructure. The currently 
    supported version of host Clang/LLVM is 3.3, but feel free to use any other 
    version of Clang at your own responsibility.

    CLANG_HOST is the absolute path to clang(1). The most common location of the system 
    Clang is /usr/bin/clang. If you have multiple versions of clang/llvm installed 
    on the system, it can be something like /usr/bin/clang-3.3. You can also 
    provide a local build of clang, e.g. /home/nika/clang+llvm-3.3-x86_64/bin/clang.

    On some Linux distributions you need to install an optional package (e.g.
    llvm-devel on Fedora) in order to be able to build LLVM pass.

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

test 1 = "$#" || usage

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

# check the given HOST
HOST="$1"
test "/" == "${HOST:0:1}" \
    || die "CLANG_HOST is not an absolute path: $HOST"
test -x "$HOST" \
    || die "CLANG_HOST is not an absolute path to an executable file: $HOST"

# try to run CLANG_HOST
"$HOST" --version || die "unable to run clang: $HOST --version"

# Prepare the environment for the compilation:
prepare_CL_environment

# Turn the NDEBUG off if requested:
# NOTE: '$' is intentionally missing before 'CL_DEBUG'
if option_set CL_DEBUG && test $OPTION_VALUE -eq 1; then
  CL_DEBUG_STRING="-D CL_DEBUG=ON";
fi

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D CLANG_HOST='$HOST' -D ENABLE_LLVM=ON $CL_DEBUG_STRING" \
    || die "failed to build Code Listener"

build_analyzer() {
    test -d $1 || return 0

    status_update "Nuking working directory"
    $MAKE -C $1 distclean || die "'$MAKE -C $1 distclean' has failed"

    status_update "Trying to build $2"
    $MAKE -C $1 CMAKE="cmake -D CLANG_HOST='$HOST' -D ENABLE_LLVM=ON" \
        || return $?

}

build_analyzer fwnull fwnull
build_analyzer sl Predator
build_analyzer fa Forester
build_analyzer vra "Value-Range Analyzer"
