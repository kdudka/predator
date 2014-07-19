/* Test case: #21
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

#define ARRAY_SIZE 3L

long ARRAY[ARRAY_SIZE];

long func(long i)
{
  long l = 42;

  l *= i;

  return (l - i);
}

struct S {
  long data_member;

  S() : data_member(2048)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  S s;
  S &object_ref = s;

  long (&array_ref)[ARRAY_SIZE] = ARRAY;
  long (&func_ref)(long) = func;
  
  for (long i = 0; i < ARRAY_SIZE; i++) {
    array_ref[i] = func_ref(i);
  }

  object_ref.data_member = func_ref(ARRAY_SIZE);

  return 0;
}
