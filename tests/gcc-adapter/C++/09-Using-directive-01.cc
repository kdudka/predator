/* Test-case: #09 */

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-directives
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
