/* Test case: #33 */

struct test {
  unsigned x;
  char array[3];
} GLOBAL_STRUCTURE;

/* 
 * Taken from:
 * https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Compound-Literals.html
 */
struct foo {
  int a;
  char b[2];
};


int main(void)
{
  GLOBAL_STRUCTURE = ((struct test) {42, {'N', 'P', '\0'}});

  char **array_of_strings = (char *[]) {"a", "b", "c"};

#if !defined __cplusplus && defined _GNU_SOURCE
  /* GNU extension, ISO C99 forbids this. Taken from the link above. */
  static struct foo x = (struct foo) {1, {'a', 'b'}};
  static int y[] = (int []) {1, 2, 3};
  static int z[] = (int [3]) {1};
#else
  static struct foo x = (struct foo) {1, {'a', 'b'}};

  /* Size of array has to be supplied, otherwise the compiler fails. */
  static int y[3] = (int []) {1, 2, 3};
  static int z[3] = (int [3]) {1, 0, 0};
#endif

  return 0;
}
