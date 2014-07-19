/* Test case: #15
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

typedef long &lvalue_ref_t;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  long variable;

// Only C++11 (C++0X) supports reference collapsing.
#if __cplusplus <= 199711L
  lvalue_ref_t variable_ref = variable;     // Tests only typedef.
#else
  lvalue_ref_t &variable_ref = variable;    // Typedef & reference collapsing.
#endif

  variable_ref = 42;
  variable_ref++;

  return 0;
}
