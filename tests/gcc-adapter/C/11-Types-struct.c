/* Test case: #11
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator. If not, see <http://www.gnu.org/licenses/>.
 */

union sub_union {
  char c;
  int i;
  float f;
};

struct sub_struct {
  long unsigned lu;
  long double ld;
};

struct test {
  union sub_union su;
  struct sub_struct ss;
};


int main(void)
{
  struct test t1 = {
    0,    /* Enumerate */

    {
      89,   /* Unsigned long integer in sub structure. */
      3.14, /* Long double in sub structure. */
    },
  };

#ifndef __cplusplus
  /* Tagged structure/union initialization. ISO C++11 still does not support. */
  struct test t2 = {
    .ss = {
      .ld = 2.72,
      .lu = 1337,
    },
    .su = {
      .i = 69,
    },
  };
#endif
  return 0;
}
