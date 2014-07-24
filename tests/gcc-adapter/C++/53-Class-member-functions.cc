/* Test case: #53
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
    static unsigned instances_counter;
    long l;

  public:
    C() : l(42)
    {
      this->instances_counter++;
      return;
    };

    ~C()
    {
      this->instances_counter--;
      return;
    }

    static unsigned get_instances_count();
    static inline void set_instances_count(unsigned num);

    long get_l() { return this->l; };
    void set_l(long l) { this->l = l; };

    void multiply_l(long l);
    inline void divide_l(long l);
};

unsigned C::instances_counter = 0;

unsigned C::get_instances_count()
{
  return C::instances_counter;
}

inline void C::set_instances_count(unsigned num)
{
  C::instances_counter = num;
  return;
}

void C::multiply_l(long l)
{
  this->l *= l;
  return;
}

inline void C::divide_l(long l)
{
  this->l /= l;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  C c1 = C();
  C *c2_ptr = new C();

  unsigned instances = C::get_instances_count();
  long value1 = c1.get_l() + (long) instances;
  long value2 = c1.get_l() - (long) instances;

  c1.multiply_l(value1);
  c2_ptr->divide_l(value2);

  c1.set_l(c2_ptr->get_l());
  c2_ptr->set_l(c1.get_l());

  c1.~C();
  delete c2_ptr;

  instances = C::get_instances_count();

  if (instances != 0) {
    return 1;
  }

  C::set_instances_count(4);

  return C::get_instances_count() % 2;
}
