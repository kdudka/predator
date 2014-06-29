/* Test case: #11 */

union sub_union {
  char c;
  int i;
  float f;
};

struct sub_struct {
  long unsigned lu;
  long double ld;
};

struct test {
  union sub_union su;
  struct sub_struct ss;
};


int main(void)
{
  struct test t1 = {
    0,    /* Enumerate */

    {
      89,   /* Unsigned long integer in sub structure. */
      3.14, /* Long double in sub structure. */
    },
  };

#ifndef __cplusplus
  /* Tagged structure/union initialization. ISO C++11 still does not support. */
  struct test t2 = {
    .ss = {
      .ld = 2.72,
      .lu = 1337,
    },
    .su = {
      .i = 69,
    },
  };
#endif
  return 0;
}
