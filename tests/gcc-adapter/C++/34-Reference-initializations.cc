/* Test case: #34
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
#include <utility>
#include <sstream>

struct S {
  int mi;
  const std::pair<int,int> &mp;   // reference member
};

void foo(int)
{
  return;
}

struct A {};

struct B : A {
  int n;

  operator int &()
  {
    return n;
  };
};

B bar()
{
  return B();
}

// int& bad_r;                  // error: no initializer
extern int& ext_r;              // OK
#endif

int main()
{
  #if 0
  // lvalues:
  int n = 1;
  int &r1 = n;                      // lvalue reference to the object n
  const int &cr(n);                 // reference can be more cv-qualified
  volatile int &cv{n};              // any initializer syntax can be used
  int &r2 = r1;                     // another lvalue reference to the object n
  // int& bad = cr;                 // error: less cv-qualified
  int& r3 = const_cast<int&>(cr);   // const_cast is needed

  void (&rf)(int) = foo;            // lvalue reference to function
  int ar[3];
  int (&ra)[3] = ar;                // lvalue reference to array

  B b;
  A &base_ref = b;                  // reference to base subobject
  int &converted_ref = b;           // reference to the result of a conversion

  // rvalues:
  // int &bad = 1;                  // error: cannot bind lvalue ref to rvalue
  const int &cref = 1;              // bound to rvalue
  int &&rref = 1;                   // bound to rvalue

  const A &cref2 = bar();           // reference to A subobject of B temporary
  A &&rref2 = bar();                // same

  int &&xref = static_cast<int&&>(n); // bind directly to n
  // int&& copy_ref = n;        // error: can't bind to an lvalue
  double&& copy_ref = n;        // bind to an rvalue temporary with value 1.0

  // restrictions on temporary lifetimes:
  std::ostream& buf_ref = std::ostringstream() << 'a';
                    // the ostringstream temporary was bound to the left operand
                    // of operator<<, but its lifetime ended at the semicolon:
                    // buf_ref is now a dangling reference.

  S a { 1, {2,3} }; // temporary pair {2,3} bound to the reference member
                    // a.mp and its lifetime is extended to match a

  S* p = new S{ 1, {2,3} }; // temporary pair {2,3} bound to the reference
                    // member a.mp, but its lifetime ended at the semicolon
                    //  p->mp is a dangling reference
  delete p;

  return 0;
  #endif
}
