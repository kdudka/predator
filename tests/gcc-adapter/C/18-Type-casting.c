/* Test case: #18
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
  long l;
  double d;
};

int main(void)
{
  char c = 0;
  int i = '0';
  double d = 3.14;

  struct test t1, t2;
  void *v_ptr = 0x0;

  c = (char) i;
  i = (int) d;
  v_ptr = (void *) &t1;
  v_ptr = (void *) &c;
  v_ptr = (void *) &t2;
  v_ptr = (void *) &i;

  return 0;
}
