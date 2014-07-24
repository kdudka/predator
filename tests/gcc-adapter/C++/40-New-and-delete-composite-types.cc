/* Test case: #40
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

#include <new>

struct S {
  char c;
  char *p_c;

  S() : c(42), p_c(&c)
  {
    (*p_c)++;
    return;
  }
};

union U {
  int i;
  int *p_i;

  U() : p_i(0x0)
  {
    return;
  }

  U(int value) : i(value)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  struct S *p_s = new S();
  union U *p_u1 = new U();
  union U *p_u2 = new U(89);

  delete p_s;
  delete p_u1;
  delete p_u2;

  return 0;
}
