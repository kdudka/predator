/* Test case: #13 */

typedef char char_t;
typedef int int_t;
typedef float float_t;

typedef union union_test {
  long l;
  long long ll;
} union_t;

typedef struct struct_test {
  long unsigned lu;
  long double ld;
} struct_t;


int main(void)
{
  char_t c = '0';
  int_t i = 42;
  float_t f = 3.14;

  union_t test1 = {
    64,
  };

  struct_t test2 = {
    128,
    256,
  };

  return 0;
}
