/*
 * Treiber stack for two processes that randomly choose their role
 * (with a garbage list)
 *
 */

#include <stdlib.h>

#define PUSH_OR_POP ((__nondet())?1:5)

int __nondet();

struct cell {
   struct cell* next;
};

int main() {

   struct cell* S = NULL;

   int pc1 = PUSH_OR_POP;
   struct cell* t1 = NULL;
   struct cell* x1 = NULL;

   int pc2 = PUSH_OR_POP;
   struct cell* t2 = NULL;
   struct cell* x2 = NULL;

   struct cell* garbage = NULL;

   while(1)
      // Choose a process that should make a step
      if (__nondet())
         // Process 1 will run
         switch (pc1) {
            // ------------------- Push 1 -------------------
            case 1:
               x1 = malloc(sizeof(*x1));
               goto L1;
            case 2:
               L1: t1 = S;
               pc1 = 3;
               continue;
            case 3:
               x1->next = t1;
               if (S == t1) {
                  S = x1;
                  pc1 = PUSH_OR_POP;
                  x1 = NULL;
                  t1 = NULL;
               } else {
                  pc1 = 2;
                  t1 = NULL;
               }
               continue;
            // ------------------- Pop 1 -------------------
            case 5:
               t1 = S;
               pc1 = 6;
               continue;
            case 6:
               if (t1 == NULL) pc1 = PUSH_OR_POP;
               else { 
                  x1 = t1->next;
                  if (S == t1) {
                     S = x1;
                     t1->next = garbage;
                     garbage = t1;
                     pc1 = PUSH_OR_POP;
                     x1 = NULL;
                     t1 = NULL;
                  } else {
                     pc1 = 5;
                     x1 = NULL;
                     t1 = NULL;
                  }
               }
               continue;
         }
      else
         // Process 2 will run
         switch (pc2) {
            // ------------------- Push 2 -------------------
            case 1:
               x2 = malloc(sizeof(*x2));
               goto L2;
            case 2:
               L2: t2 = S;
               pc2 = 3;
               continue;
            case 3:
               x2->next = t2;
               if (S == t2) {
                  S = x2;
                  pc2 = PUSH_OR_POP;
                  x2 = NULL;
                  t2 = NULL;
               } else {
                  pc2 = 2;
                  t2 = NULL;
               }
               continue;
            // ------------------- Pop 2 -------------------
            case 5:
               t2 = S;
               pc2 = 6;
               continue;
            case 6:
               if (t2 == NULL) pc2 = PUSH_OR_POP;
               else {
                  x2 = t2->next;
                  if (S == t2) {
                     S = x2;
                     t2->next = garbage;
                     garbage = t2;
                     pc2 = PUSH_OR_POP;
                     x2 = NULL;
                     t2 = NULL;
                  } else {
                     pc2 = 5;
                     x2 = NULL;
                     t2 = NULL;
                  }
               }
               continue;
         }

   return 0;
}
