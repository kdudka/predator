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

int main(void)
{
  long int_result;
  float float_result;

  int_result = 0;
  int_result += 42;
  int_result -= 89;
  int_result *= 256;
  int_result /= 10;
  int_result %= 3;
  int_result <<= 8;
  int_result >>= 3;
  int_result |= 42;
  int_result &= 89;
  int_result ^= 127;

  float_result = 0;
  float_result += 3.14;
  float_result -= 2.72;
  float_result *= 99.9;
  float_result /= 100;

  return 0;
}
