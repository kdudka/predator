/*
 * Treiber stack for two processes that randomly choose their role
 * (with a garbage list)
 *
 */

#include <stdlib.h>

int __nondet();

struct cell {
   struct cell* next;
};

#define PUSH_OR_POP ((__nondet())?1:5)

#define INIT(I)                    \
        int pc##I = PUSH_OR_POP;   \
        struct cell* t##I = NULL;  \
        struct cell* x##I = NULL   \

#define PROCESS(I)                                    \
        switch (pc##I) {                              \
           /* ------------ Push ------------ */       \
           case 1:                                    \
              x##I = malloc(sizeof(*x##I));           \
              pc##I = 2;                              \
              continue;                               \
           case 2:                                    \
              t##I = S;                               \
              pc##I = 3;                              \
              continue;                               \
           case 3:                                    \
              x##I->next = t##I;                      \
              pc##I = 4;                              \
              continue;                               \
           case 4:                                    \
              if (S == t##I) {                        \
                 S = x##I;                            \
                 pc##I = PUSH_OR_POP;                 \
                 x##I = NULL;                         \
                 t##I = NULL;                         \
              } else {                                \
                 pc##I = 2;                           \
                 t##I = NULL;                         \
              }                                       \
              continue;                               \
           /* ------------ Pop ------------ */        \
           case 5:                                    \
              t##I = S;                               \
              pc##I = 6;                              \
              continue;                               \
           case 6:                                    \
              if (t##I == NULL) pc##I = PUSH_OR_POP;  \
              else pc##I = 7;                         \
              continue;                               \
           case 7:                                    \
              x##I = t##I->next;                      \
              pc##I = 8;                              \
              continue;                               \
           case 8:                                    \
              if (S == t##I) {                        \
                 S = x##I;                            \
                 t##I->next = garbage;                \
                 garbage = t##I;                      \
                 pc##I = PUSH_OR_POP;                 \
                 x##I = NULL;                         \
                 t##I = NULL;                         \
              } else {                                \
                 pc##I = 5;                           \
                 x##I = NULL;                         \
                 t##I = NULL;                         \
              }                                       \
              continue;                               \
        }                                             \

int main() {

   struct cell* S = NULL;
   struct cell* garbage = NULL;

   INIT(1);
   INIT(2);

   while(1)
      // Choose a process that should make a step
      if (__nondet()) 
        PROCESS(1)
      else
        PROCESS(2)

   return 0;
}