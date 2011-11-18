#!/bin/sh
export SELF="$0"
export LC_ALL=C

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s HOST_GCC_EXECUTABLE GCC_PLUGIN_INCLUDE_DIR\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator against an arbitrary build of host
    GCC.  The host GCC needs to be built with the support for GCC plug-ins.  The
    currently supported version of host GCC is 4.6.2, but feel free to use any
    other version of GCC at your own responsibility.

    HOST_GCC_EXECUTABLE is an exectuable of gcc(1).  It can be given either with
    absolute path (e.g. /home/bob/gcc-4.7.0/bin/gcc) or, if it can be reached
    from \$PATH, only the basename is sufficient (e.g. gcc, or gcc-4.6).

    GCC_PLUGIN_INCLUDE_DIR is the path to GCC headers for building GCC plug-ins.
    The path is distribution-specific.  You can guess the path by looking for a
    file named 'gcc-plugin.h'.  Some distributions do not have such an include
    directory installed by default.  A brief summary of experience with
    distributions we have tried follows:

    On Debian, you need to install a package named gcc-4.5-plugin-dev.  Then use
    /usr/lib/x86_64-linux-gnu/gcc/x86_64-linux-gnu/4.5.2/plugin/include as
    GCC_PLUGIN_INCLUDE_DIR.

    On Debian unstable, you need to install a package named gcc-4.6-plugin-dev.
    Then use /usr/lib/gcc/x86_64-linux-gnu/4.6/plugin/include as
    GCC_PLUGIN_INCLUDE_DIR.
 
    On Fedora, you need to install a package named gcc-plugin-devel.  Then use
    /usr/lib/gcc/x86_64-redhat-linux/4.6.2/plugin/include as
    GCC_PLUGIN_INCLUDE_DIR.

    On Gentoo, you only need to have installed >=sys-devel/gcc-4.5.0.  Then use
    /usr/lib64/gcc/x86_64-pc-linux-gnu/4.x.y/plugin/include as
    GCC_PLUGIN_INCLUDE_DIR.

    Corrections and extensions to this list are welcome!

EOF
    exit 1
}

test 2 = "$#" || usage

status_update() {
    printf "\n%s...\n\n" "$*"
}

# try to run gcc
GCC_HOST="$1"
"$GCC_HOST" --version || die "unable to run gcc: $GCC_HOST --version"

# check the include path
GCC_INCDIR="$2"
PLUG_HDR="$GCC_INCDIR/gcc-plugin.h"
test -r "$PLUG_HDR" || die "unable to read: $PLUG_HDR"

ln -fsvT "$GCC_INCDIR" include/gcc || die "failed to create symlink"

status_update "Nuking working directory"
make distclean \
    || die "'make distclean' has failed"

status_update "Trying to build Code Listener"
make -C cl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Code Listener"

status_update "Checking whether Code Listener works"
make -C cl check \
    || die "Code Listener does not work"

status_update "Trying to build Predator"
make -C sl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Predator"

status_update "Checking whether Predator works"
make -C sl check

if test -d .git; then
    die "you are in a git repository --> you need set \$GCC_HOST yourself!"
else
    status_update "Initializing \$GCC_HOST"
    sed "s|GCC_HOST=.*\$|GCC_HOST='$GCC_HOST'|" -i \
        chk-error-label-reachability.sh register-paths.sh sl/probe.sh sl/slgcc
fi
