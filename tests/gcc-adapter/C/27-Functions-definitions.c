/* Test case: #27
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

int GLOBAL_RESULT;


void func_foo(void)
{
  int i = 42, j = 89;

  GLOBAL_RESULT =  i + j;

  return;
}

int func_bar(int input)
{
  int result = GLOBAL_RESULT;

  return result + input;
}

double func_foobar(char *formal_parameters, ...)
{
  double retval = 0;

  retval++;

  return retval * 3.14159265359F;
}

int (*func_bar_pointer)(int) = func_bar;

double (*array_of_func_pointers[42])(char *formal_parameters, ...);


int main(void)
{
  return 0;
}
