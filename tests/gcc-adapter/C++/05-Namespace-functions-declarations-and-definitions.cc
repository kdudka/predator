/* Test case: #05
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator. If not, see <http://www.gnu.org/licenses/>.
 */

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
