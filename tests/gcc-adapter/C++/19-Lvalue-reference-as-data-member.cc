/* Test case: #19
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

long LONG_GLOBAL = 42;

struct S {
  long &long_lref;
  
  S() : long_lref(LONG_GLOBAL)
  {
    long_lref++;
    return;
  }

  S(long &lref) : long_lref(lref)
  {
    long_lref--;
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  long long_local = 89;

  S s1;
  S s2(long_local);

  return 0;
}
