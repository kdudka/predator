/* Test case: #43
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
  int S::* i_ptr;
  int i;

  S() : i_ptr(0), i('#')
  {
    return;
  }

  S(const int i) : i_ptr(0), i(i)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  S s = S('$');               // C++98 initialization.

  s.i_ptr = &S::i;            // Set the pointer to have offset of 'i'.

  int &i_ref = s.*s.i_ptr;    // Try to get address of 'i' of object 's'.

  // Pointer arithmetics should be now used to obtain the address of 'c' via
  // 'x', so it can be changed. We're making sure 2 pointers are not added
  // together, which is syntax error. If so, predator/forester should be able to
  // catch this problem and OFFSET_TYPE handling would have to be fixed.
  // FIXME: Find a way to test this possible issue right in the Code Listener.
  i_ref = '@';

  return i_ref;
}
