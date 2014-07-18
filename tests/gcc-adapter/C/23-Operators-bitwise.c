/* Test case: #23
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
  char result, value = 42;

  result = 42 | 89;
  result = 42 & 218;
  result = 42 ^ 89;
  result = 42 << 2;
  result = 42 >> 1;
  result = ~42;

  result = value | 89;
  result = result & 218;
  result = result ^ 89;
  result = result << 2;
  result = result >> 1;
  result = ~result;

  return 0;
}
