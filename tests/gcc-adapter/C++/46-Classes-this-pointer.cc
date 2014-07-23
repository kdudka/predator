/* Test case: #46
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
  // implicitly private:
    char c1;
    signed char c2;
    unsigned char c3;

  protected:
    int i1;
    signed int i2;
    unsigned int i3;

  public:
    long l1;
    signed long l2;
    unsigned long l3;

    C() : c1('#'), c2('&'), c3('@')
    {
      this->i1 = 69;
      this->i2 = -72;
      this->i3 = 75U;

      (*this).l1 = 1024L;
      (*this).l2 = -2048L;
      (*this).l3 = 4096UL;

      return;
    }

    C(char c1, char c2, char c3) : c1(c1), c2(c2), c3(c3)
    {
      this->i1 = 69;
      this->i2 = -72;
      this->i3 = 75U;

      (*this).l1 = 1024L;
      (*this).l2 = -2048L;
      (*this).l3 = 4096UL;

      return;
    }

    C(int i1, int i2, int i3) : c1('#'), c2('&'), c3('@')
    {
      // this' pointer is needed for disambiguation:
      i1 = i1;        // Otherwise i1 of formal parameter is assigned to itself.
      this->i1 = i1;
      this->i2 = i2;
      this->i3 = i3;

      (*this).l1 = 1024L;
      (*this).l2 = -2048L;
      (*this).l3 = 4096UL;

      return;
    }

    ~C()
    {
      (*this).c1 = 0;
      (*this).c2 = 0x0;
      (*this).c3 = '\0';

      this->i1 = (int) l1;
      this->i2 = (signed int) l2;
      this->i3 = (unsigned int) l3;
    }
};


// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // Automatic allocation on stack:
  C c = C();
  C *c_ptr1 = new C('$', '*', '+'); // Constructor called explicitly.
  C *c_ptr2 = new C(128, 256, 512); // Constructor called implicitly.

  c.l1 = (long) c_ptr1->l2;
  c_ptr1->l2 = (signed long) c_ptr2->l3;

  return 0; // Destructor for object 'c' is called when main returns.
}
