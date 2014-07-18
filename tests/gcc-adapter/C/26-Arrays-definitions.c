/* Test case: #26
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

#define ARRAY_SIZE 3

static unsigned GLOBAL_ARRAY[ARRAY_SIZE] = {0, 1, 2};
static unsigned GLOBAL_MATRIX[ARRAY_SIZE][ARRAY_SIZE] = {
  {0, 1, 2},
  {3, 4, 5},
  {6, 7, 8},
};

int main(void)
{
  /* Static & automatic arrays. */
  unsigned size = 3;
  unsigned local_array[size];
  static unsigned static_local_array[ARRAY_SIZE] = {42, 89};

  unsigned (*array_pointer)[ARRAY_SIZE];

#ifndef __cplusplus
  /*
   * Type-casting is required, but ISO C++ forbids casting to an array type
   * 'unsigned int* []'.
   */
  array_pointer = static_local_array;
  array_pointer = local_array;
#endif

  unsigned *array_of_pointers[ARRAY_SIZE];
  unsigned *array_of_pointers_2[size];

  return 0;
}
