
Simple test of lists from glib library (glib is the base for GTK+)

This README is for version glib-2.30.1 from
http://ftp.gnome.org/pub/gnome/sources/glib/

all headers from glib-VERSION/glib/ copied into ./glib
added glib-VERSION/config.h

Edits (marked with // style comments):
 - allocation macros replaced to use standard malloc
 - some functions from other glib modules added to common-functions.c
   which is included into glist.c and gslist.c

 - #include gslist.c to test code in slist.c
 - simplification: remove some unhandled parts of slist.c

 - ditto for glist.c and list.c


