/* Test-case: #XX */

char c = '&';
int i = 42;

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-declarations
 */
namespace A {
    int x;
}

namespace B {
    int i;

    struct g { };
    struct x { };

    void f(int);
    void f(double);
    void g(char);     // OK: function name g hides struct g
}

void func() {
    int i;
    // using B::i;       // error: i declared twice
 
    void f(char);
    using B::f;       // OK: f(char), f(int), f(double) are overloads
    f(3.5);           // calls B::f(double)
 
    using B::g;
    g('a');           // calls B::g(char)
    struct g g1;      // declares g1 to have type struct B::g
 
    using B::x;
    using A::x;       // OK: hides struct B::x
    x = 99;           // assigns to A::x
    struct x x1;      // declares x1 to have type struct B::x
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  func();
  return 0;
}
