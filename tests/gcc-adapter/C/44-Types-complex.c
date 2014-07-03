/* Test case: #44 */

#include <complex.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  /*
   * <complex.h>
   * Example taken from: http://en.cppreference.com/w/c/numeric/complex
   */
  printf("%2zu\n", sizeof(float complex));
  printf("%2zu\n", sizeof(double complex));
  printf("%2zu\n", sizeof(long double complex));

  printf("%4.1f  + %4.1fi\n", creal(_Complex_I), cimag(_Complex_I));
  printf("%4.1f  + %4.1fi\n", creal(I), cimag(I));

  printf("%4.1f  + %4.1fi\n", creal(1.0 * _Complex_I * 1.0 * _Complex_I),
                              cimag(1.0 * _Complex_I * 1.0 * _Complex_I));
  printf("%4.1f  + %4.1fi\n", creal(1.0 * I * 1.0 * I),
                              cimag(1.0 * I * 1.0 * I));

  double complex z1 = 1.0 + 2.0 * I;
  double complex z2 = 2.0 + 4.0 * _Complex_I;

  double complex z3 = z1 + z2;
  printf("%4.1f  + %4.1fi\n", creal(z3), cimag(z3));

  #undef I
  #define J _Complex_I
  z1 = 1.5 + 2.5 * J;

  printf("%4.1f  + %4.1fi\n", creal(z1), cimag(z1));

  return 0;
}
