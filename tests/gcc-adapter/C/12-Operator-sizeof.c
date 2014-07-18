/* Test case: #12
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

struct test {
  char c1;
  char c2;
  double d;
};


int main(void)
{
  long unsigned size = sizeof(long unsigned);
  struct test t = {
    'N',
    'P',
    3.14,
  };

  size = sizeof(struct test);
  size = sizeof(size);
  size = sizeof(t);

  return 0;
}
