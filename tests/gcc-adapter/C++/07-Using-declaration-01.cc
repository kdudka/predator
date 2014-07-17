/* Test-case: #07 */

char ch = '!';

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-declarations
 */
void f();

namespace A {
  int i = 42;

  void g();
}

namespace X {
  long l = 2048;

  using ::f;      // global f is now visible as ::X::f
  using A::g;     // A::g is now visilbe as ::X::g
}

void h()
{
  X::f();         // calls ::f
  X::g();         // calls A::g
}

// Adding function definitions for the example to work:
void f()
{
  A::i += ch;
  return;
}

void g()
{
  ::ch = X::l % ::ch;
  return;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  h();
  return 0;
}
