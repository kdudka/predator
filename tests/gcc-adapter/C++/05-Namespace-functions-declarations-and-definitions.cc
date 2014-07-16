/* Test-case: #05 */

namespace ns {
  char c = 42;
  int i = 89;

  // Function declarations:
  extern void foo();
  char bar();

  // Inside namespace function definition:
  int func()
  {
    i *= 256;
    return i - c;
  }
}

// Unnamed namespace (anonymous namespace in terms of GIMPLE docs):
// Code Listener has to be able to distinguish between named & unnamed
// namespaces.
namespace {
  char ch = 69;

  char foobar()
  {
    ch <<= 1;
    return ch;
  }
}

// Out-of-namespace definitions:
char ns::bar()
{
  c += '0';
  return c;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  long l = ns::bar() + foobar();
  
  l *= ns::func();

  return 0;
}
