/* Test case: #12 */

struct test {
  char c1;
  char c2;
  double d;
};


int main(void)
{
  long unsigned size = sizeof(long unsigned);
  struct test t = {
    'N',
    'P',
    3.14,
  };

  size = sizeof(struct test);
  size = sizeof(size);
  size = sizeof(t);

  return 0;
}
