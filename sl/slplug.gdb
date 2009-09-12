file ../gcc-install/bin/gcc
set args -fplugin=./slplug.so -std=c99 -pedantic -Wall -W -g -O0 -c data/dll.c
set confirm 0
set follow-fork-mode child
set pagination 0
set print pretty
set unwindonsignal
run
echo \nbt:\n
bt
echo \nframe:\n
f 1
echo \ncode:\n
list
echo \n\n
