/* Test case: #08
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

char c = '&';
int i = 42;

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
