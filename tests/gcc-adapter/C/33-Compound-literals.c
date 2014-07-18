/* Test case: #33
 *
 * Copyright NOTE - this file is part of predator's test suite.
 *
 * However, this example was taken (and modified) from:
 * <https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Compound-Literals.html>
 *
 * Therefore the copyright belongs to Free Software Foundation, Inc. (FSF):
 * (c) Free Software Foundation, Inc. (FSF)
 */

struct test {
  unsigned x;
  char array[3];
} GLOBAL_STRUCTURE;

struct foo {
  int a;
  char b[2];
};


int main(void)
{
  GLOBAL_STRUCTURE = ((struct test) {42, {'N', 'P', '\0'}});

  const char *array_of_strings[] = {"a", "b", "c"};

#if !defined __cplusplus && defined _GNU_SOURCE
  /* GNU extension, ISO C99 forbids this. Taken from the link above. */
  static struct foo x = (struct foo) {1, {'a', 'b'}};
  static int y[] = (int []) {1, 2, 3};
  static int z[] = (int [3]) {1};
#else
  static struct foo x = (struct foo) {1, {'a', 'b'}};

  /* Size of array has to be supplied, otherwise the compiler fails. */
  static int y[3] = {1, 2, 3};
  static int z[3] = {1, 0, 0};
#endif

  return 0;
}
