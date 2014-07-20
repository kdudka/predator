/* Test case: #25
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

long GLOBAL_VAL = 42;

char func1()
{
  return (char) GLOBAL_VAL;
}

int func2(char c = '@')
{
  return (c != '&') ? (int) GLOBAL_VAL : 0;
}

long func3(char c, int i = 256)
{
  if (c != '$') {
    return (i == 256) ? 2048L : 0L;
  }
  else {
    return (i != 256) ? 2048L : 0L;
  }
}

double func4(char c, int i, long l = 2048)
{
  return ((double) c * (double) i) / (double) l;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  char c = func1();
  int i = func2('#');
  GLOBAL_VAL = func3(c, 4096);
  double d = func4(c, i);

  return (int) ((func4(func1(), func2(), func3(func1()))) * d);
}
