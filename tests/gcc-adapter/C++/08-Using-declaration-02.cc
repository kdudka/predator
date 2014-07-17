/* Test-case: #08 */

char c = '&';
int i = 42;

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-declarations
 */
namespace A {
  void f(int);
}

using A::f;       // ::f is now a synonym for A::f(int)
 
namespace A {     // namespace extension
  void f(char);   // does not change what ::f means
}

void foo() {
  f('a');         // calls f(int), even though f(char) exists.
} 

void bar() {
  using A::f;     // this f is a synonym for both A::f(int) and A::f(char)
  f('b');         // calls f(char)
}

// Adding function definitions for the example to work:
void A::f(int i)
{
  ::i += i;
  ::i--;
  i++;
  i += ::i;
  ::i %= i;

  return;
}

void A::f(char c)
{
  ::c -= c;
  ::c++;
  c--;
  c -= ::c;
  ::c /= c;
  
  return;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  foo();
  bar();

  return 0;
}
