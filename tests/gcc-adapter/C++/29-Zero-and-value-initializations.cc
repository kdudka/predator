/* Test case: #29
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

// FIXME: Update the test after adding STD library support.

// #include <string>
// #include <vector>
// #include <iostream>

namespace NS {
  struct A {
      int i;
      A() {}            // user-provided ctor, does not initialize i
  };

  struct B {            // no user-provided ctor
    A a;
  };
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

struct T1 {             // no constructors
  int mem1;
  // std::string mem2;
  NS::B mem2;
};

struct T2 {
  int mem1;
  // std::string mem2;

  T2(const T2&) {}      // a constructor, but no default
};

struct T3 {
  int mem1;
  // std::string mem2;
  T3() {}               // user-provided default ctor
};

// C++98 does not allow initialization by '{}'
#if __cplusplus <= 199711L
  // value initialization of temporary and copy-initializing the object:
  NS::A a = NS::A();
#else
  NS::A a{};              // calls default ctor in C++11
#endif

// std::string s{};        // calls default ctor, the value is "" (empty string)

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  int i = NS::B().a.i;
  // std::cout << B().a.i << '\n'; // value-initializes a B temporary
                                // leaves b.a.i uninitialized in C++03
                                // sets b.a.i to zero in C++11

  // (note that B{}.a.i leaves b.a.i uninitialized in C++14, but for
  //  a different reason: C++14's B{} is aggregate-initialization)

#if __cplusplus <= 199711L
  int n = int();              // int n(); would not initialize object!!
#else
  int n{};                    // non-class value-initialization, value is 0
#endif
  double f = double();        // non-class value-init, value is 0.0
  int array1[10];             // should be zero initialized
  // int* a = new int[10]();     // array of 10 zeroes

#if __cplusplus <= 199711L
  T1 t1 = {0, NS::B()};       // value initializer list
#else
  T1 t1{};                    // no ctors: zero-initialized
#endif
                              // t1.mem1 is zero-initialized
                              // t1.mem2 is default-initialized
                              //
  // T2 t2{};                 // error: has a ctor, but no default ctor

#if __cplusplus <= 199711L
  T3 t3 = T3();               // same as above
#else
  T3 t3 {};                   // user-defined default ctor:
#endif
                              // t3.mem1 is default-initialized (the value is
                              // indeterminate), t3.mem2 is default-initialized

  // std::vector<int> v(3);      // value-initializes three ints
  int array2[3] = {0, 1, 2};  // value initialized

  // std::cout << s.size() << ' ' << n << ' ' << f << ' ' << a[9] << ' ' << v[2] << '\n';
  // std::cout << t1.mem1 << ' ' << t3.mem1 << '\n';
  // delete[] a;

  return 0;
}
