/* Test case: #14
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

char *char_ptr = 0x0;
int *int_ptr = 0x0;
float *float_ptr = 0x0;

typedef union test_union {
  long l;
  double d;
} union_t;

typedef struct test_struct {
  char c;
  union_t u;
  struct test_struct *struct_ptr;
} struct_t;


int main(void)
{
  char c = '$';
  int i = 42;
  float f = 3.14;
  
  char_ptr = &c;
  int_ptr = &i;
  float_ptr = &f;

  union_t u = {
    89,
  };

  struct_t s = {
    '0',
    { 2.72, },
    0x0,
  };

  union_t *union_t_ptr = &u;
  struct_t *struct_t_ptr = &s;

  return 0;
}
