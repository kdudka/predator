/* Test case: #28
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

// #include <string>  // ->> Avoiding the use of 'stdlibc++' ...

struct T1 {};

struct T2 {
  int mem;

  T2() {}             // "mem" not in initializer list
};

namespace NS {
  union T3 {          // naive substitution for std::string in the example
    char array[42];   // "array" also not in initializer list

    T3() {
      for (unsigned i = 0; i < 42; i++) {
        array[42] = 0;
      }

      return;
    };
  };
}

int n; // A two-phase initialization is done
       // In the first phase, zero initialization initializes n to zero
       // In the second phase, default initialization does nothing, leaving n
       // being zero

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  int n;              // non-class: the value is undeterminate

  // std::string s;      // calls default ctor, the value is "" (empty string)
  // std::string a[2];   // calls default ctor, creates two empty strings
  NS::T3 s;           // calls default ctor, producing "empty" string literal
  NS::T3 a[2];        // calls default ctor, creating two empty string literals

  // int& r;          // error: default-initializing a reference
  // const int n;     // error: const non-class type
  // const T1 nd;     // error: const class type with implicit ctor

  T1 t1;              // ok, calls implicit default ctor
  const T2 t2;        // ok, calls the user-provided default ctor
                      // t2.mem is default-initialized (to indeterminate value)

  return 0;
}
