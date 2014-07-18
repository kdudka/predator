/* Test case: #28
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

struct test {
  int value;
  int array[42];
};

int initval(struct test t)
{
  return t.value;                 /* Call by value. */
}

int retval(struct test *t_ptr)
{
  return t_ptr->value;            /* Call by reference. */
}


int main(void)
{
  struct test t, *t_ptr = &t;
  
  t.value = 0;                    /* Access by '.' notation. */

  /* Structure member access via pointer + array member access. */
  t_ptr->array[0] = 89;
  t_ptr->array[1] = retval(&t);

  return retval(t_ptr);
}
