/* Test case: #40
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
  unsigned i, j, k, result;

start:
  for (i = 0; i < 10; i++) {
    for (j = 0; j < 20; j++) {
      for (k = 0; k < 30; k++) {
        result = i * j * k;

        if ((result % 13) == 0)
          goto end_of_fors;
      }
    }
  }
  

end_of_fors:
  if ((i + j + k) % 2 == 1)
    goto start;

  return 0;
}
