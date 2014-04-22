#!/bin/bash
export SELF="$0"
export LC_ALL=C
MAKE="make -j5"

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s GCC_HOST\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator and/or Forester against an arbitrary
    build of host GCC.  The host GCC needs to be built with the support for GCC
    plug-ins.  The currently supported version of host GCC is 4.8.2, but feel
    free to use any other version of GCC at your own responsibility.

    GCC_HOST is the absolute path to gcc(1) that is built with the support for
    GCC plug-ins.  The most common location of the system GCC is /usr/bin/gcc.
    If you have multiple versions of gcc installed on the system, it can be
    something like /usr/bin/gcc-4.8.2.  You can also provide a local build of
    GCC, e.g.  /home/bob/gcc-4.8.2/bin/gcc.  Please avoid setting GCC_HOST to
    a ccache, distcc, or another GCC wrapper.  Such setups are not supported
    yet.

    On some Linux distributions you need to install an optional package (e.g.
    gcc-plugin-devel on Fedora) in order to be able to build GCC plug-ins.

EOF
    exit 1
}

test 1 = "$#" || usage

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

# Completing the path of given GCC, if necessary:
GCC="$1"

if [[ "${GCC:0:1}" == "/" ]]; then
  # Absolute path can also lead to symlink:
  if [[ -L "$GCC" ]]; then
    GCC_HOST="$(readlink -f $GCC)"
  else
    GCC_HOST="$GCC"
  fi
else
  # Testing if the given GCC can be located via $PATH:
  WHICH_RESULT="$(which $GCC)"

  # Not in $PATH:
  if [[ "$WHICH_RESULT" == "" || "${WHICH_RESULT:0:1}" == "." ]]; then
    GCC_HOST="$(realpath $GCC)"
  # Possible symlink used by many sys-admins:
  elif [[ -L "$WHICH_RESULT" ]]; then
    GCC_HOST="$(readlink -f $WHICH_RESULT)"
  else
    GCC_HOST="$WHICH_RESULT"
  fi
fi

test -x "$GCC_HOST" \
    || die "GCC_HOST is not an executable file: $GCC"

# try to run GCC_HOST
"$GCC_HOST" --version || die "unable to run gcc: $GCC_HOST --version"

status_update "Nuking working directory"
$MAKE distclean \
    || die "'$MAKE distclean' has failed"

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
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
    $MAKE -C $1 check
}

build_analyzer fwnull fwnull
build_analyzer sl Predator
build_analyzer fa Forester
build_analyzer vra "Value-Range Analyzer"
