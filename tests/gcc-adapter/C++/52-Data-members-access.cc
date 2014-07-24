/* Test case: #52
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
    char c;
    int i;

  protected:
    long l;

  public:
    static const unsigned u_const = 42;
    static unsigned u_static;

    double d;

    C() : c('@'), i(49), l(2048), d(6.72) {};

    char get_c() { return this->c; };
    int get_i() { return this->i; };
    long get_l() { return this->l; };

    void set_c(char c) { this->c = c; };
    void set_i(int i) { this->i = i; };
    void set_l(long l) { this->l = l; };
};

unsigned C::u_static = 0;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  class C c1 = C();
  class C *c2_ptr = new C();

  char c = c1.get_c();
  int i = c1.get_i();
  long l = c1.get_l();
  double d = c1.d;

  c = (char) C::u_const;
  C::u_static++;
  i = C::u_static;
  l += c + i;
  d /= l;

  c2_ptr->set_c(c);
  c2_ptr->set_i(i);
  c2_ptr->set_l(l);
  c2_ptr->d = d;

  delete c2_ptr;

  return 0;
}
