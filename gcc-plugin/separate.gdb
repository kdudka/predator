file ../gcc-install/bin/gcc
set args -std=c99 -pedantic -Wall -W -g -O0 -fPIC -I../gcc-install/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include -fplugin=./separate.so -c ../separate/list.c
set confirm 0
set follow-fork-mode child
set pagination 0
run
echo \nbt:\n
bt
echo \nframe:\n
f 1
echo \ncode:\n
list
echo \n\n
