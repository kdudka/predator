/*
 * Treiber stack for two processes (with a garbage list)
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

   int pc4 = 1;
   struct cell* t4 = NULL;
   struct cell* x4 = NULL;

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
               // __VERIFIER_plot("problema");
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
      else // ------------------- Pop 4 -------------------
         switch (pc4) {
            case 1:
               t4 = S;
               pc4 = 2;
               continue;
            case 2:
               if (t4 == NULL) pc4 = 1;
               else pc4 = 3;
               continue;
            case 3:
               x4 = t4->next;
               pc4 = 4;
               continue;
            case 4:
               if (S == t4) {
                  S = x4;
                  t4->next = garbage;
                  garbage = t4;
                  pc4 = 1;
                  x4 = NULL;
                  t4 = NULL;
               } else {
                  pc4 = 1;
                  x4 = NULL;
                  t4 = NULL;
               }
               continue;
        }
   return 0;
}
