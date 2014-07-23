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
    printf "Usage: %s GCC_HOST\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator and/or Forester against an arbitrary
    build of host GCC.  The host GCC needs to be built with the support for GCC
    plug-ins.  The recommended version of host GCC is 4.9.0 but Predator can be
    loaded also into older versions of GCC (plug-ins are supported since 4.5.0).
    For host GCC 4.6.x and older, please use the compatibility patches in the
    build-aux directory.

    GCC_HOST is a gcc(1) executable file that is built with the support for
    GCC plug-ins.  The most common location of the system GCC is /usr/bin/gcc.
    If you have multiple versions of gcc installed on the system, it can be
    something like /usr/bin/gcc-4.9.0.  You can also provide a local build of
    GCC, e.g.  /home/bob/gcc-4.9.0/bin/gcc.  Please avoid setting GCC_HOST to
    a ccache, distcc, or another GCC wrapper.  Such setups are not supported
    yet.

    On some Linux distributions you need to install an optional package (e.g.
    gcc-plugin-devel on Fedora) in order to be able to build GCC plug-ins.

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

# check the given GCC_HOST
GCC_HOST="$1"
if test "/" != "${GCC_HOST:0:1}"; then
    if echo "$GCC_HOST" | grep / >/dev/null; then
        # assume a relative path to GCC_HOST
        GCC_HOST="$(readlink -f "$GCC_HOST")"
    else
        # assume an executable in $PATH
        GCC_HOST="$(command -v "$GCC_HOST")"
    fi
fi
test -x "$GCC_HOST" || die "GCC_HOST is not an executable file: $1"

# try to run GCC_HOST
"$GCC_HOST" --version || die "unable to run gcc: $GCC_HOST --version"

status_update "Nuking working directory"
$MAKE distclean \
    || die "'$MAKE distclean' has failed"

# Prepare the CL environment for the compilation:
prepare_CL_environment

# Turn the NDEBUG off if requested:
# NOTE: '$' is intentionally missing before 'CL_DEBUG'
if option_set CL_DEBUG && test $OPTION_VALUE -eq 1; then
  CL_DEBUG_STRING="-D CL_DEBUG=ON";
fi

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D GCC_HOST='$GCC_HOST' $CL_DEBUG_STRING" \
    || die "failed to build Code Listener"

status_update "Checking whether Code Listener works"
$MAKE -C cl check \
    || die "Code Listener does not work"

build_analyzer() {
    test -d $1 || return 0

    status_update "Trying to build $2"
    $MAKE -C $1 CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
        || return $?

    status_update "Checking whether $2 works"
    $MAKE -C $1 check CTEST='ctest -j9'
}

build_analyzer fwnull fwnull
build_analyzer sl Predator
build_analyzer fa Forester
build_analyzer vra "Value-Range Analyzer"
