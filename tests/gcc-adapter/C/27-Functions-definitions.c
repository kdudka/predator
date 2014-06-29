/* Test case: #27 */

int GLOBAL_RESULT;


void func_foo(void)
{
  int i = 42, j = 89;

  GLOBAL_RESULT =  i + j;

  return;
}

int func_bar(int input)
{
  int result = GLOBAL_RESULT;

  return result + input;
}

double func_foobar(char *formal_parameters, ...)
{
  double retval = 0;

  retval++;

  return retval * 3.14159265359F;
}

int (*func_bar_pointer)(int) = func_bar;

double (*array_of_func_pointers[42])(char *formal_parameters, ...);


int main(void)
{
  return 0;
}
