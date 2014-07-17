/* Test-case: #10 */

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-directives
 */

namespace D {
  int d1;
  void f(char);
}
using namespace D;      // introduces D::d1, D::f, D::d2, D::f,
                        //  E::e, and E::f into global namespace!
 
int d1;                 // OK: no conflict with D::d1 when declaring
namespace E {
    int e;
    void f(int);
}
namespace D {           // namespace extension
    int d2;
    using namespace E;  // transitive using-directive
    void f(int);
}
void f() {
    D::d1++;            // OK
    ::d1++;             // OK
    D::d1++;            // OK
    d2++;               // OK, d2 is D::d2
    e++;                // OK: e is E::e due to transitive using
    E::f(1);            // OK: E::f(int)
    f('a');             // OK: the only f(char) is D::f(char)
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  f();
  return 0;
}
