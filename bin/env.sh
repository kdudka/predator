#!/bin/sh
source /etc/profile
unset err
echo "--- TOOLS_ROOT = $TOOLS_ROOT"

BIN_DIR="`readlink -f $TOOLS_ROOT/bin`"                 || err=yes
GCC_DIR="`readlink -f $TOOLS_ROOT/gcc-install/bin`"     || err=yes
LIB_DIR="`readlink -f $TOOLS_ROOT/sl`"                  || err=yes

echo "--- BIN_DIR = $BIN_DIR"
echo "--- GCC_DIR = $GCC_DIR"
echo "--- LIB_DIR = $LIB_DIR"

test -x "$BIN_DIR/gcc"                                  || err=yes
test -x "$GCC_DIR/gcc"                                  || err=yes
test -x "$LIB_DIR/slplug.so"                            || err=yes
test -f "$LIB_DIR/gcc.gdb"                              || err=yes

if test -z "$err"; then
    echo ">>> setting environment variables"
    export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
    export PATH="$BIN_DIR:$PATH"
    export GCC_PATH="$GCC_DIR:$PATH"
    export GDB_SLPLUG="$LIB_DIR/gcc.gdb"
    echo "--- All OK!"
else
    echo "!!! error occurred while looking for dependencies"
fi
