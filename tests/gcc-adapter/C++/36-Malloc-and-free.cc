/* Test case: #36
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

#include <new>
#include <cstdlib>

struct S {
  double d;
  long l;
  char c;

  S() : d(3.14), l(4096), c('@')
  {
    return;
  }

  ~S()
  {
    d = 0;
    l = 0;
    c = 0;

    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

// NOTE: This code isn't supposed to be run ->> we're not testing allocation
//       success.

int main()
{
  long *p_long = (long *) std::malloc(sizeof(*p_long));

  // Constructors are not called when using malloc!
  S *p_S1 = (struct S *) std::malloc(sizeof(*p_S1));
  S *p_S2 = (struct S *) std::calloc(1, sizeof(*p_S2));

  new (p_S1) S(); // Explicit call of constructor as 'placement new'.
  new (p_S2) S(); // Explicit call of constructor as 'placement new'.

  *p_long = 2048;
  p_S1->d = 2.72;
  p_S1->l = *p_long;
  p_S1->c = '#';

  // Call the destructor first ->> "what has been constructed, has to be
  // destructed"... by Petr Peringer.
  p_S2->~S();

  // Create an array of objects S to test realloc, but don't call constructors:
  p_S2 = (struct S *) std::realloc(p_S2, sizeof(*p_S2) * 10);

  p_S1->~S();     // Explicit call of destructor.
  std::free((void *) p_S1);

  // p_S2->~S();  // Destructor was already called.
  std::free((void *) p_S2);

  std::free((void *) p_long);
  p_long = NULL;

  return 0;
}
