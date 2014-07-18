/* Test case: #29
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

extern unsigned EXTERN_GLOBAL;
unsigned GLOBAL;

struct test1;

struct test2 {
  signed i;
  struct test1 *ptr_other;
};

struct test1 {
  unsigned j;
  struct test2 *ptr_other;
};

extern double func_extern();      /* Declaration. */
int func_anonym(double);          /* Anonymous specification. */


int main(void)
{
  int func_complete(double x);    /* Complete functional prototype. */

  double parameter = 3.14;

  return func_anonym(parameter) * func_complete(parameter);
}


int func_anonym(double input)
{
  (void) input;
  return 0;
}

int func_complete(double input)
{
  (void) input;
  return 1;
}
