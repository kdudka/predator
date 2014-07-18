/* Test case: #07
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

int main(void)
{
  short int si = -32767;
  unsigned short int usi = 65535U;

  int i = -42;
  unsigned int ui = 89U;

  long int li = -2147483648;
  unsigned long int uli = 4294967295U;

#ifdef __LP64__
  long long int lli = -9223372036854775808;
  unsigned long long int ulli = 18446744073709551615U;
#endif

  return 0;
}
