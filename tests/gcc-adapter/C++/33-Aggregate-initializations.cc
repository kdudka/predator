/* Test case: #33
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
#include <array>

struct S {
  int x;

  struct Foo {
    int i;
    int j;
    int a[3];
  } b;
};

union U {
  int a;
  const char* b;
};
#endif

int main()
{
  #if 0
  S s1 = { 1, { 2, 3, {4, 5, 6} } };
  S s2 = { 1, 2, 3, 4, 5, 6};         // same, but with brace elision
  S s3{1, {2, 3, {4, 5, 6} } };       // same, using direct-list-initialization
                                      // syntax

  // S s4{1, 2, 3, 4, 5, 6};          // error: brace-elision only allowed with
  //                                     equals sign ('=')

  int ar[] = {1,2,3};                 // ar is int[3]
  // char cr[3] = {'a', 'b', 'c', 'd'}; // too many initializer clauses
  char cr[3] = {'a'};                 // array initialized as {'a', '\0', '\0'}

  int ar2d1[2][2] = {{1, 2}, {3, 4}};   // fully-braced 2D array: {1, 2}
                                        //                        {3, 4}

  int ar2d2[2][2] = {1, 2, 3, 4};       // brace elision: {1, 2}
                                        //                {3, 4}

  int ar2d3[2][2] = {{1}, {2}};         // only first column: {1, 0}
                                        //                    {2, 0}

  std::array<int, 3> std_ar2{ {1,2,3} };  // std::array is an aggregate
  std::array<int, 3> std_ar1 = {1, 2, 3}; // brace-elision okay

  int ai[] = { 1, 2.0 };          // narrowing conversion from double to int:
                                  // error in C++11, okay in C++03

  std::string ars[] = {std::string("one"),  // copy-initialization
                        "two",              // conversion, then copy-init
                        {'t', 'h', 'r', 'e', 'e'} };  // list-initialization

  U u1 = {1};                     // OK, first member of the union
  // U u2 = { 0, "asdf" };        // error: too many initializers for union
  // U u3 = { "asdf" };           // error: invalid conversion to int
  #endif

  return 0;
}
