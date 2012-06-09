/*
 * Treiber stack for two processes with a garbage list
 *
 */

#include <stdlib.h>

int __nondet();

struct cell {
   struct cell* next;
};

int main() {

   struct cell* S = NULL;

   int pc1 = 1;
   struct cell* t1 = NULL;
   struct cell* x1 = NULL;

   int pc2 = 1;
   struct cell* t2 = NULL;
   struct cell* x2 = NULL;

   struct cell* garbage = NULL;

   while(1)
      if (__nondet()) // ------------------- Push 1 -------------------
         switch (pc1) {
            case 1:
               x1 = malloc(sizeof(*x1));
               pc1 = 2;
               continue;
            case 2:
               t1 = S;
               pc1 = 3;
               continue;
            case 3:
               x1->next = t1;
               pc1 = 4;
               continue;
            case 4:
               if (S == t1) {
                  S = x1;
                  pc1 = 1;
                  x1 = NULL;
                  t1 = NULL;
               } else {
                  pc1 = 2;
                  t1 = NULL;
               }
               continue;
         }
      else // ------------------- Pop 2 -------------------
         switch (pc2) {
            case 1:
               t2 = S;
               pc2 = 2;
               continue;
            case 2:
               if (t2 == NULL) pc2 = 1;
               else pc2 = 3;
               continue;
            case 3:
               x2 = t2->next;
               pc2 = 4;
               continue;
            case 4:
               if (S == t2) {
                  S = x2;
                  t2->next = garbage;
                  garbage = t2;
                  pc2 = 1;
                  x2 = NULL;
                  t2 = NULL;
               } else {
                  pc2 = 1;
                  x2 = NULL;
                  t2 = NULL;
               }
               continue;
        }
   return 0;
}
