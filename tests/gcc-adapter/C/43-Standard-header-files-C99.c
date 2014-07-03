/* Test case: #43 */

// #include <complex.h> - Tested via a separate test-case.
#include <fenv.h>
#include <inttypes.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
// #include <tgmath.h> - Same as <math.h> and <complex.h> all together.
#include <wchar.h>
#include <wctype.h>

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * NOTE - Many of the examples here were taken from:
 * http://en.cppreference.com/w/c/ *
 */

double fenv_test(double a, double b) {
  const int range_problem = FE_OVERFLOW | FE_UNDERFLOW;
  feclearexcept(range_problem);

  return a * b;
}


int main(int argc, char *argv[])
{
  /* <fenv.h> */
  printf("hypot(%f, %f) = %f\n", 3.0, 4.0, fenv_test(3.0, 4.0));
  printf("hypot(%e, %e) = %e\n", DBL_MAX / 2.0, DBL_MAX / 2.0,
                                 fenv_test(DBL_MAX / 2.0, DBL_MAX / 2.0));

  /* <inttypes.h> */
  printf("%zu\n", sizeof(int64_t));

  /* <iso646.h> ->> Alternative operators spelling and digraphs/trigraphs. */
  if (argc > 1 and argv<:1:> not_eq NULL) <%
    printf("Hello %s!\n", argv<:1:>);
  %>

  /* <stdbool.h> */
  bool truth_value = false;
  
  if (argc > 1) {
    truth_value = true;
  }

  if (truth_value == false) {
    printf("NOPE!\n");
  }
  else {
    printf("YES!\n");
  }

  /* <stdint.h> */
  intmax_t signed_max = (-0x7fffffffffffffff - 1);
  uintmax_t unsigned_max =  0xffffffffffffffff;

  /* <wchar.h> */
  char    string_1[BUFSIZ];
  wchar_t string_2[BUFSIZ];

  fgets(string_1, BUFSIZ, stdin);
  fgetws(string_2, BUFSIZ, stdin);
  mbstowcs(string_2, string_1, BUFSIZ);

  /* <wctype.h> */
  wint_t wc = L'C';

  if (iswctype(wc, wctype("alnum"))) {
    return 0;
  }
  else {
    return 1;
  }
}
