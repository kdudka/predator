/* Test case: #14 */

char *char_ptr = 0x0;
int *int_ptr = 0x0;
float *float_ptr = 0x0;

typedef union test_union {
  long l;
  double d;
} union_t;

typedef struct test_struct {
  char c;
  union_t u;
  struct test_struct *struct_ptr;
} struct_t;


int main(void)
{
  char c = '$';
  int i = 42;
  float f = 3.14;
  
  char_ptr = &c;
  int_ptr = &i;
  float_ptr = &f;

  union_t u = {
    89,
  };

  struct_t s = {
    '0',
    { 2.72, },
    0x0,
  };

  union_t *union_t_ptr = &u;
  struct_t *struct_t_ptr = &s;

  return 0;
}
