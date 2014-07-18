/* Test case: #07
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

char ch = '!';

void f();

namespace A {
  int i = 42;

  void g();
}

namespace X {
  long l = 2048;

  using ::f;      // global f is now visible as ::X::f
  using A::g;     // A::g is now visilbe as ::X::g
}

void h()
{
  X::f();         // calls ::f
  X::g();         // calls A::g
}

// Adding function definitions for the example to work:
void f()
{
  A::i += ch;
  return;
}

void g()
{
  ::ch = X::l % ::ch;
  return;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  h();
  return 0;
}
