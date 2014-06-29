/* Test case: #29 */

extern unsigned EXTERN_GLOBAL;
unsigned GLOBAL;

struct test1;

struct test2 {
  signed i;
  struct test1 *ptr_other;
};

struct test1 {
  unsigned j;
  struct test2 *ptr_other;
};

extern double func_extern();      /* Declaration. */
int func_anonym(double);          /* Anonymous specification. */


int main(void)
{
  int func_complete(double x);    /* Complete functional prototype. */

  double parameter = 3.14;

  return func_anonym(parameter) * func_complete(parameter);
}


int func_anonym(double input)
{
  (void) input;
  return 0;
}

int func_complete(double input)
{
  (void) input;
  return 1;
}
