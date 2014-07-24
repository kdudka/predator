/* Test case: #50
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

struct S {
  long l1;
  long l2;

  S() : l1(2048), l2(4096) {};
  S(long val1, long val2 = 1024) : l1(val1), l2(val2) {};
  S(const S &obj) : l1(obj.l1), l2(obj.l2) {};
};

struct S func(struct S obj)   // Call by value - copy constructor used.
{
  obj.l1++;
  obj.l2--;

  return obj;                 // Copy constructor used.
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  struct S s1;                // Implicit constructor.
  struct S s2 = S();          // Explicit call of the implicit constructor.
  struct S s3 = S(256, 512);
  struct S s4 = S(s3);        // Copy constructor used.
  struct S s5 = func(s2);     // Copy constructor used.

  return 0;
}
