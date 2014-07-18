/* Test case: #20
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
  long result_int = 0;
  float result_real = 0.0;

  result_int = result_int + 42;
  result_int = result_int - 89;
  result_int = result_int * 10;
  result_int = result_int / 5;
  result_int = result_int % 2;
  result_int = ((result_int + 10) * 3) % ((result_int - 42) / 6);

  result_real = result_real + 3.14;
  result_real = result_real - 2.72;
  result_real = result_real * 10;
  result_real = result_real / 256;
  result_real = ((result_real + 99.9) / 16.6) * ((result_real - 33.3) / 66.6);

  return 0;
}
