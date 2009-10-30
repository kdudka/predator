file ./slsparse
set args -g -O0 -Wno-decl -fPIC -std=c99 -pedantic -Wall -W -Werror -I.. -I../gcc -iquote. -I. data/slplug.c
set confirm 0
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
target record
