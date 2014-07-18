/* Test case: #32
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

char *GLOBAL_STRING = "Hello, Code-Listener!";


int main(void)
{
  char *str_ptr1, *str_ptr2;

  str_ptr1 = GLOBAL_STRING;
  str_ptr2 = str_ptr1 + 8;
  str_ptr1 = str_ptr2 - 5;
  str_ptr2 = (char *) GLOBAL_STRING[3];
  str_ptr1 = str_ptr2 + 18;
  str_ptr2 = GLOBAL_STRING + (str_ptr1 - str_ptr2);

  return 0;
}
