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

const char STRING[] = "Hello, Code Listener!";

char foo(const unsigned pos, const char* str)
{
  return str[pos];
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  const unsigned u = 4;
  unsigned v = 6;
  char c;
  char local_string[] = "KEEP CALM AND KEEP CODING";

  c = foo(v, STRING);
  c += foo(u, local_string);

  return (int) c;
}
