/* Test case: #41
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

  U() : p_i(NULL)
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
  struct S *p_s = new (std::nothrow) S();
  union U *p_u = new (std::nothrow) U(89);

  if (p_s == NULL || p_u == NULL) {
    return 1;       // We're ignoring memory leaks.
  }

  struct S *p_s_array = new (std::nothrow) S[10];
  union U *p_u_array = new (std::nothrow) U[5];

  if (p_s_array == NULL || p_u_array == NULL) {
    return 2;       // We're ignoring memory leaks.
  }

  delete p_s;
  delete p_u;

  delete[] p_s_array;
  delete[] p_u_array;

  return 0;
}
