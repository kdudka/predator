/* Test case: #13
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

typedef char char_t;
typedef int int_t;
typedef float float_t;

typedef union union_test {
  long l;
  long long ll;
} union_t;

typedef struct struct_test {
  long unsigned lu;
  long double ld;
} struct_t;


int main(void)
{
  char_t c = '0';
  int_t i = 42;
  float_t f = 3.14;

  union_t test1 = {
    64,
  };

  struct_t test2 = {
    128,
    256,
  };

  return 0;
}
