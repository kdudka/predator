/* Test case: #10
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * However, the example(s) below were taken from (and modified):
 * <http://en.cppreference.com/>
 *
 * Because of it, this file is licensed under GFDL v1.3 and CC BY-SA 3.0
 * licenses as requested at: <http://en.cppreference.com/w/Cppreference:FAQ>
 *
 * You should have receive copies of these licenses in the ~/predator/tests/
 * folder with the predator itself. If not, please, see their online versions:
 * <http://www.gnu.org/copyleft/fdl.html>
 * <https://creativecommons.org/licenses/by-sa/3.0/legalcode>
 */

namespace D {
  int d1;
  void f(char);
}
using namespace D;      // introduces D::d1, D::f, D::d2, D::f,
                        //  E::e, and E::f into global namespace!
 
int d1;                 // OK: no conflict with D::d1 when declaring
namespace E {
    int e;
    void f(int);
}
namespace D {           // namespace extension
    int d2;
    using namespace E;  // transitive using-directive
    void f(int);
}
void f() {
    D::d1++;            // OK
    ::d1++;             // OK
    D::d1++;            // OK
    d2++;               // OK, d2 is D::d2
    e++;                // OK: e is E::e due to transitive using
    E::f(1);            // OK: E::f(int)
    f('a');             // OK: the only f(char) is D::f(char)
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  f();
  return 0;
}
