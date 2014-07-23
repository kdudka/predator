/* Test case: #47
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

class C1 {
    char c;

  public:
    typedef int int_t;
    int_t i;

    C1() : c('#'), i(42)
    {
      return;
    }

    C1(char c, int i = 42) : c(c), i(i)
    {
      return;
    }
};

class C2 {
    typedef long long_t;
    long_t l;
    class C1 inner_class1;

  public:
    struct S {
      typedef double double_t;
      double_t d;

      S() : d(2.72)
      {
        return;
      }

      S(double d) : d(d)
      {
        return;
      }
    };

    struct S inner_class2;

    union U;

  protected:
    union {
      char *c_ptr;
    };

  public:
    C2() : l(2048), inner_class1('$', 89), inner_class2(3.14), c_ptr(0x0)
    {
      return;
    }
};

union C2::U {
  char c;
  int i;
  float f;

  U() : i(69)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  class C2 c1, c2;
  struct C2::S s = C2::S(9.78);

  c1.inner_class2.d = s.d;
  s.d = c2.inner_class2.d;

  C1::int_t i = -64;

  class C1 c3 = C1('@', i);

  return 0;
}
