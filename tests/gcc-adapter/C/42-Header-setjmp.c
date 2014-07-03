/* Test case: #42 */

#include <setjmp.h>

void exception_handling(jmp_buf env)
{
  static unsigned i = 0;

  for ( ; i >= 0; i++) {
    longjmp(env, i);
  }
}

int main(void)
{
  int retval;
  jmp_buf env;

  retval = setjmp(env);

  if (retval < 3) {
    exception_handling(env);
  }

  return 0;
}
