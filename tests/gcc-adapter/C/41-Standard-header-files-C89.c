/* Test case: #41
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * However, the example(s) below were taken from (and modified):
 * <http://en.cppreference.com/>
 *
 * Because of it, this file is licensed under GFDL v1.3 and CC BY-SA 3.0
 * licenses as requested at: <http://en.cppreference.com/w/Cppreference:FAQ>
 *
 * You should have receive copies of these licenses in the ~/predator/tests/
 * folder with the predator itself. If not, please, see their online versions:
 * <http://www.gnu.org/copyleft/fdl.html>
 * <https://creativecommons.org/licenses/by-sa/3.0/legalcode>
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
// #include <setjmp.h> ->> tested via a separate test-case.
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void signal_handler(int signal)
{
    printf("Received signal %d\n", signal);
}

void simple_printf(const char* fmt,...)
{
    va_list args;
    va_start(args, fmt);
 
    while (*fmt != '\0') {
        if (*fmt == 'd') {
            int i = va_arg(args, int);
            printf("%d\n", i);
        } else if (*fmt == 'c') {
            int c = va_arg(args, int);
            printf("%c\n", c);
        } else if (*fmt == 'f') {
            double d = va_arg(args, double);
            printf("%f\n", d);
        }
        ++fmt;
    }
 
    va_end(args);
}

int main(int argc, char *argv[])
{
  /* <assert.h> */
  assert(argc != 0);

  /* <ctype.h> */
  char c = toupper('c');

  /* <errno.h> */
  errno = 0;
  printf("log(-1.0) = %f\n", log(-1.0));
  printf("%s\n\n",strerror(errno));

  /* <float.h> */
  printf("FLT_RADIX    = %d\n", FLT_RADIX);
  printf("FLT_MIN      = %e\n", FLT_MIN);
  printf("FLT_MAX      = %e\n", FLT_MAX);
  printf("FLT_EPSILON  = %e\n", FLT_EPSILON);
  printf("FLT_DIG      = %d\n", FLT_DIG);
  printf("FLT_MANT_DIG = %d\n", FLT_MANT_DIG);
  printf("FLT_MIN_EXP  = %d\n", FLT_MIN_EXP);
  printf("\n");

  /* <limits.h> */
  printf("CHAR_BIT   = %d\n", CHAR_BIT);
  printf("MB_LEN_MAX = %d\n", MB_LEN_MAX);
  printf("CHAR_MIN   = %+d\n", CHAR_MIN);
  printf("CHAR_MAX   = %+d\n", CHAR_MAX);
  printf("SCHAR_MIN  = %+d\n", SCHAR_MIN);
  printf("SCHAR_MAX  = %+d\n", SCHAR_MAX);
  printf("UCHAR_MAX  = %u\n", UCHAR_MAX);
  printf("\n");

  /* <locale.h> */
  setlocale(LC_ALL, "");

  /* <math.h> */
  printf("pow(2, 10) = %f\n", pow(2,10));
  printf("pow(2, 0.5) = %f\n", pow(2,0.5));
  printf("pow(-2, -3) = %f\n", pow(-2,-3));

  /* <signal.h> */
  signal(SIGTERM, signal_handler);

  printf("Sending signal %d\n", SIGTERM);
  raise(SIGTERM);
  printf("Exit main()\n");

  /* <stdarg.h> */
  simple_printf("dcff", 3, 'a', 1.999, 42.5);

  /* <string.h> */
  memset(&c, 42, sizeof(c));

  /* <time.h> */
  time_t result = time(NULL);
  printf("%s", asctime(localtime(&result)));

  /* <stdlib.h> */
  if (c == 'c') {
    abort();
  }
  else {
    exit(EXIT_SUCCESS);
  }
}
