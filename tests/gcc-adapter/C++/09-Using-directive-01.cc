/* Test case: #09
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

namespace A {
  int i = 69;
}

namespace B {
  int i;
  int j;

  namespace C {
    namespace D {
      using namespace A;  // all names from A injected into global namespace
      int j;
      int k;
      int a = i;          // i is B::i, because A::i is hidden by B::i
    }

    using namespace D;    // names from D are injected into C
                          // names from A are injected into global namespace
    int k = 89;           // OK to declare name identical to one introduced by
                          // a using ambiguous: C::k or D::k
    int l = D::k;
    int m = i;            // OK: B::i hides A::i
    int n = j;            // OK: D::j hides B::j
  }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  using namespace B::C::D;

  j = 89;
  k = 42;
  a = 128;
  i = 256;

  return 0;
}
