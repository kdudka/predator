file ./slsparse
set args -Wno-decl data/dll.c
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
