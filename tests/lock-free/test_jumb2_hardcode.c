/*
 * Singly linked list with head pointers
 *
 * boxes:
 */

#include <stdlib.h>

  int __nondet();
  struct cell { 
    int data;
    struct cell* next;
  };

 int main() {
  
 struct cell* t1 = NULL;
 struct cell* x1 = NULL;
 struct cell* t2 = NULL;
 struct cell* x2 = NULL;
 struct cell* t3 = NULL;
 struct cell* x3 = NULL;
 struct cell* t4 = NULL;
 struct cell* x4 = NULL;
 struct cell* x = NULL;
 struct cell* garbage = NULL;
 int v = 4;
 int pc1 = 1;
 int pc2 = 1; 
 int pc3 = 1;
 int pc4 = 1;  
 int res3;
 int res4;        
 int result;

 struct cell* S = NULL;
 while(1)
 {   
   if(__nondet())
   
     {     
     //--------------------------------Push 1----------------------------------------
           if (pc1==1){
               x1 = malloc(sizeof(*x1)); 
               x1->data = 0; 
               x1->next = NULL;
               pc1 = 2;

               continue;
           }

           if (pc1 == 2){
               pc1 = 3;
               x1->data = v;

               continue;
           }
            if(pc1 == 3){
               t1 = S;
               pc1 = 4;
               continue;
             }

             if(pc1 == 4){
               x1->next = t1;
               pc1 = 5;
               continue;
             }

             if(pc1 == 5){
               if (S == t1) 
                 { S = x1;
                   pc1 = 6;}
               else
                  {pc1 = 3;}
               continue;
             }
          if(pc1 == 6){
            pc1 = 1;
            continue;
         }
    }
   //--------------------------------Pop 4----------------------------------------
if(__nondet())
  {
       if(pc4 == 1){
          t4 = S;
          pc4 = 2;
          continue;
       }
       if(pc4 == 2){
          if(t4 == NULL)
         {pc4 = 1;}
          else
          pc4 = 3;
          continue;
           }
       
       if(pc4 == 3){
          x4 = t4->next;
          pc4 = 4;
          continue;
          }

       if(pc4 == 4){
          if (S == t4) 
             { S = x4;
               pc4 = 5; 
             }
          else
             { pc4 = 1; }
          continue;      
       }

       if(pc4 == 5 ){
         res4 = t4->data;
         t4->next = garbage;
         garbage = t4;
         pc4 = 1;
         continue;
       } 
  }
 }  
while (garbage != NULL) {
		x = garbage;
		garbage = garbage->next;
		free(x);
	}
     return 0;
}

             