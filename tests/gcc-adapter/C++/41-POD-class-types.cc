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

// NOTE: Static data members are not covered in this test.

#include <new>

enum basic_enum {
  ZERO = 0,
  FIRST,
  SECOND,
  THIRD
};

struct POD_S {
  char c;
  wchar_t wc;
  bool b;
  unsigned u;
  long l;
  double d;
  double d_array[42];

  enum basic_enum e;

  void *void_ptr;
  struct POD_S *this_ptr;
  char POD_S::* c_ptr;
  void (*func_ptr)(...);

  struct POD_S func()
  {
    return *this_ptr;
  }
};

union POD_U {
  char c;
  wchar_t wc;
  bool b;
  unsigned u;
  long l;
  double d;
  double d_array[42];

  enum basic_enum e;

  void *void_ptr;
  union POD_U *this_ptr;
  char POD_U::* c_ptr;
  void (*func_ptr)(...);

  union POD_U func()
  {
    return *this_ptr;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // Not initialized:
  POD_S *S_ptr = new POD_S;
  POD_U *U_ptr = new POD_U;

  delete S_ptr;
  delete U_ptr;

  // Default-initialized:
  S_ptr = new POD_S();
  U_ptr = new POD_U();

  delete S_ptr;
  delete U_ptr;

  return 0;
}
