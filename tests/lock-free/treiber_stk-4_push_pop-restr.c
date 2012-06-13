/*
 * Treiber stack for four processes that randomly choose their role
 * with a garbage list and restricted concurrency
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
           /* ---------- PUSH ---------- */           \
           case 1:                                    \
              x##I = malloc(sizeof(*x##I));           \
           case 2:                                    \
              t##I = S;                               \
              x##I->next = t##I;                      \
              pc##I = 3;                              \
              continue;                               \
           case 3:                                    \
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
           /* ---------- POP ---------- */            \
           case 5:                                    \
              t##I = S;                               \
              if (t##I == NULL) pc##I = PUSH_OR_POP;  \
              else {                                  \
                 x##I = t##I->next;                   \
                 pc##I = 6;                           \
              }                                       \
              continue;                               \
           case 6:                                    \
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
   INIT(3);
   INIT(4);

   while(1)
      // Choose a process that should make a step
      if (__nondet()) 
        if (__nondet())
          PROCESS(1)
	else
	  PROCESS(2)
      else
        if (__nondet())
          PROCESS(3)
	else
	  PROCESS(4)

   return 0;
}
