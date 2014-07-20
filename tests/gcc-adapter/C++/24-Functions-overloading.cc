/* Test case: #24
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

void func(char c)
{
  GLOBAL_VAL = (c == '$') ? 0 : 1;
  return;
}

void func(int i)
{
  GLOBAL_VAL = (i != 89) ? 2 : 3;
  return;
}

void func(long l)
{
  GLOBAL_VAL = (l == 2048L) ? 4 : 5;
  return;
}

void func(double d)
{
  GLOBAL_VAL = (d != 3.14F) ? 5 : 0;
  return;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  char c = '@';
  int i = 256;
  long l = 4096;
  double d = 3.14F;

  func(c);
  func(i);
  func(l);
  func(d);

  return GLOBAL_VAL;
}
