/* Test case: #35
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

// NOTE: Explanation of the issue can be found @:
//       http://en.cppreference.com/w/cpp/language/overloaded_address
// FIXME: Allow this test when C++ STD library support is enabled.
// FIXME: Transform this with conditional compilation, this is not C++98 valid.

#if 0
#include <algorithm>
#include <cctype>   // int std::toupper(int)
#include <iostream>
#include <locale>   // template<class CharT> CharT std::toupper(CharT,const locale&)
#include <string>

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

int f(int) {return 1;}
int f(double) {return 2;}

void g(int(&f1)(int), int(*f2)(double)) {}

template<int(*F)(int)>
struct Templ {};

struct Foo {
  int mf(int) {return 3;}
  int mf(double) {return 4;}
};

// 5. return value ->> selects inf f(int)
int (*(foo)(void))(int) {return f;}
#endif

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  #if 0
  // 1. initialization
  int (*pf)(double) = f;              // selects int f(double)
  int (&rf)(int) = f;                 // selects int f(int)
  int (Foo::*mpf)(int) = &Foo::mf;    // selects int mf(int)

  // 2. assignment

#if GCC_VERSION > 46000
  pf = nullptr;                 // nullptr support was introduced in gcc-4.6.0
#endif
  pf = &f;                      // selects int f(double)

  // 3. function argument
  g(f, f);  // selects int f(int) for the 1st argument
            // and int f(double) for the second

  // 4. user-defined operator

  // 6. cast
  std::string str = "example";

  // selects int toupper(int):
  std::transform(str.begin(), str.end(), str.begin(),
                  static_cast<int(*)(int)>(std::toupper));

  std::cout << str << '\n';

  // 7. template argument
  Templ<f> t;                         // selects int f(int)
  #endif

  return 0;
}
