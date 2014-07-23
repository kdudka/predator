/* Test case: #49
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

class C {
    const static long const_val = 2048;
    static long instances_num;  // Declaration ->> definition must be outside.

  public:
    C()
    {
      this->instances_num++;
      return;
    }

    ~C()
    {
      this->instances_num--;
      return;
    }

    static long addition(long &l1, long &l2); // Has to be defined outside!
    static long get_instances_num();          // Has to be defined outside!
};

// Definitions of previous declarations:
long C::instances_num = 0;

long C::addition(long &l1, long &l2)
{
  return l1 + l2;
}

long C::get_instances_num()
{
  return C::instances_num;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  long init_instances_num = C::get_instances_num();
  long value1 = C::get_instances_num();
  long value2 = C::get_instances_num();

  if (init_instances_num != 0) {
    return 1;
  }

  class C c1 = C();
  value1 = C::get_instances_num();

  if (value1 != 1) {
    return 2;
  }

  class C *c2_ptr = new C();
  value2 = C::get_instances_num();

  if (value2 != 1) {
    return 3;
  }

  long result = C::addition(value1, value2);

  delete c2_ptr;
  return result;
}
