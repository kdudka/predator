/* Test case: #48
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

const class C { // const specification is for GLOBAL_INSTANCE_C, not the class C
    int i;
    int j;

  public:
    int k;

    C() : i(89), j(42), k(49) {};
    ~C() {};
} GLOBAL_INSTANCE_C;

volatile struct S { // same as above goes for the volatile specification
  private:
    char m;
    char n;
    char o;

  public:
    S() : m('m'), n('n'), o('o') {};
    ~S() {};

    void foo() volatile
    {
      char tmp = m;

      m = n;
      n = tmp;

      return;
    }

    void bar()
    {
      char tmp = n;

      n = o;
      o = tmp;

      return;
    }
} GLOBAL_INSTANCE_S;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  class C local_instance_C1 = C();
  struct S local_instance_S1 = S();

  volatile C local_instance_C2 = C();
  const S local_instance_S2 = S();

  local_instance_C1.k = 57;
  // GLOBAL_INSTANCE_C.k = local_instance_C1.k;   // error ->> readonly class
  local_instance_C1.k = GLOBAL_INSTANCE_C.k;      // OK

  GLOBAL_INSTANCE_S.foo();      // OK
  // GLOBAL_INSTANCE_S.bar();   // error ->> bar() has no volatile specification

  local_instance_C2.k = 77;
  // local_instance_S2.bar();   // error ->> readonly structure

  const class C *C_ptr = new C();
  volatile struct S *S_ptr = new S();

  // C_ptr->k = local_instance_C2.k;  // error ->> readonly class
  S_ptr->foo();
  // S_ptr->bar();                    // error ->> missing volatile spec

  delete C_ptr;
  delete S_ptr;

  return 0;
}
