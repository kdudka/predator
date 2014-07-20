/* Test case: #31
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
#include <utility>
#include <memory>
#endif

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  #if 0
  std::string s = "test";         // OK: constructor is non-explicit
  std::string s2 = std::move(s);  // this copy-initialization performs a move

  // std::unique_ptr<int> p = new int(1);   // error: constructor is explicit
  std::unique_ptr<int> p(new int(1));       // OK: direct-initialization

  int n = 3.14;       // floating-integral conversion
  const int b = n;    // const doesn't matter
  int c = b;          // ...either way
  #endif

  return 0;
}
