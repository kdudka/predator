/* Test case: #18 */

struct test {
  long l;
  double d;
};

int main(void)
{
  char c = 0;
  int i = '0';
  double d = 3.14;

  struct test t1, t2;
  void *v_ptr = 0x0;

  c = (char) i;
  i = (int) d;
  v_ptr = (void *) &t1;
  v_ptr = (void *) &c;
  v_ptr = (void *) &t2;
  v_ptr = (void *) &i;

  return 0;
}
