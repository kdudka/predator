/* Test case: #13
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

char c = '&';
int i = 42;

namespace A {
    int x;
}

namespace B {
    int i;

    struct g { };
    struct x { };

    void f(int);
    void f(double);
    void g(char);     // OK: function name g hides struct g
}

void func() {
    int i;
    // using B::i;       // error: i declared twice

    void f(char);
    using B::f;       // OK: f(char), f(int), f(double) are overloads
    f(3.5);           // calls B::f(double)

    using B::g;
    g('a');           // calls B::g(char)
    struct g g1;      // declares g1 to have type struct B::g

    using B::x;
    using A::x;       // OK: hides struct B::x
    x = 99;           // assigns to A::x
    struct x x1;      // declares x1 to have type struct B::x
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  func();
  return 0;
}
