/* Test case: #10
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

enum subtest_enum {
  FIRST = 1,
  THIRD = 3,
  FIFTH = 5,
};

union subtest {
  enum subtest_enum st_enum;
  long unsigned lu;
  long double ld;
};

union test {
  char c;
  int i;
  float f;
  union subtest st;
};


int main(void)
{
  union test t = {
    42,
  };

  return 0;
}
