/* Test case: #10 */

enum subtest_enum {
  FIRST = 1,
  THIRD = 3,
  FIFTH = 5,
};

union subtest {
  enum subtest_enum st_enum;
  long unsigned lu;
  long double ld;
};

union test {
  char c;
  int i;
  float f;
  union subtest st;
};


int main(void)
{
  union test t = {
    42,
  };

  return 0;
}
