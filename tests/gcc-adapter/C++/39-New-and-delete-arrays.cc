/* Test case: #39
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

const unsigned ARRAY_SIZE = 10;

struct S {
  void *ptr;

  S() : ptr(NULL)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  char *p_c = new char[ARRAY_SIZE];
  int *p_i = new int[ARRAY_SIZE * 2];
  long *p_l = new long[ARRAY_SIZE * 3];
  double *p_d = new double[ARRAY_SIZE * 4];

  // Should call constructors automatically:
  struct S *p_s = new struct S[ARRAY_SIZE * 5];

  delete[] p_c;
  delete[] p_i;
  delete[] p_l;
  delete[] p_d;
  delete[] p_s;

  return 0;
}
