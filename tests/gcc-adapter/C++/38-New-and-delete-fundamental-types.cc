/* Test case: #38
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

#include <new>

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  char *p_char = new char;
  short *p_short = new short();
  long *p_long = new long(2048);

  int *p_int = new int + 42;
  unsigned *p_unsigned = new unsigned(4096) + 89;

  double *p_double = new double(2.72);

  delete p_char;
  delete p_short;
  delete p_long;

  p_int -= 42;
  p_unsigned -= 89;

  delete p_int;
  delete p_unsigned;
  delete p_double;

  return 0;
}
