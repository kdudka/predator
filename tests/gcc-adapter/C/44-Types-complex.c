/* Test case: #44
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

#include <complex.h> #include <stdio.h>

int main(int argc, char *argv[]) { printf("%2zu\n", sizeof(float complex));
printf("%2zu\n", sizeof(double complex)); printf("%2zu\n", sizeof(long double
complex));

  printf("%4.1f  + %4.1fi\n", creal(_Complex_I), cimag(_Complex_I));
  printf("%4.1f  + %4.1fi\n", creal(I), cimag(I));

  printf("%4.1f  + %4.1fi\n", creal(1.0 * _Complex_I * 1.0 * _Complex_I),
  cimag(1.0 * _Complex_I * 1.0 * _Complex_I)); printf("%4.1f  + %4.1fi\n",
  creal(1.0 * I * 1.0 * I), cimag(1.0 * I * 1.0 * I));

  double complex z1 = 1.0 + 2.0 * I; double complex z2 = 2.0 + 4.0 * _Complex_I;

  double complex z3 = z1 + z2; printf("%4.1f  + %4.1fi\n", creal(z3),
  cimag(z3));

  #undef I #define J _Complex_I z1 = 1.5 + 2.5 * J;

  printf("%4.1f  + %4.1fi\n", creal(z1), cimag(z1));

  return 0; }
