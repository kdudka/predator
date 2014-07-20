/* Test case: #26
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

// TODO: This test is missing testing of 'core constant expressions' (with
//       constexpr) of C++11. Implement when needed!

const unsigned ARRAY_SIZE = 10U;
const double PI = 3.14F;
const char STRING[] = "Hello, Code Listener!";
const char AMPERSAND = '&';

char ARRAY[ARRAY_SIZE];

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  for (unsigned i = 0; i < ARRAY_SIZE; i++) {
    ARRAY[i] = STRING[i];
  }
  
  double PI_multiplied = PI * AMPERSAND;

  return (int) PI_multiplied;
}
