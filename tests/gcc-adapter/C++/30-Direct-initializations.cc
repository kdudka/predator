/* Test case: #30
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

// FIXME: Allow this test when C++ STD library support is enabled.
// FIXME: Transform this with conditional compilation, this is not C++98 valid.

#if 0
#include <string>
#include <iostream>
#include <memory>

struct Foo {
    int mem;
    explicit Foo(int n) : mem(n) {}
};
#endif

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  #if 0
  std::string s1("test");                 // constructor from const char*
  std::string s2(10, 'a');

  std::unique_ptr<int> p(new int(1));     // OK: explicit constructors allowed
  // std::unique_ptr<int> p = new int(1); // error: constructor is explicit

  Foo f(2);   // f is direct-initialized:
              // constructor parameter n is copy-initialized from the rvalue 2
                  // f.mem is direct-initialized from the parameter n
  // Foo f2 = 2;  // error: constructor is explicit

  std::cout << s1 << ' ' << s2 << ' ' << *p << ' ' << f.mem  << '\n';
  #endif

  return 0;
}
