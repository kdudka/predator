/* Test case: #30
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

char *STRING_GLOBAL = "Hello, Code-Listener!";

int main(void)
{
  char *empty_string1 = "";
  char empty_string2[1] = {'\0'};
  char *string_ptr;

  string_ptr = empty_string1;
  string_ptr = STRING_GLOBAL;
  string_ptr = empty_string2;

  return 0;
}
