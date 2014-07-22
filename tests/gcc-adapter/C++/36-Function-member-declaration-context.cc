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

namespace {   // Unnamed namespace. (Anonymous namespace in terms of GIMPLE.)
  struct S {
    int i;

    int get_i()
    {
      return i;
    }

    void set_i(int i)
    {
      this->i = i;
    }

    // Basic constructors:
    S() : i(42)
    {
      set_i(get_i() + 4);
      return;
    }

    S(int i) : i(i)
    {
      return;
    }

    S(int i, int j) : i(i)
    {
      i += j;
      return;
    }

    //  Destructor:
    ~S()
    {
      return;
    }
  };
}

struct T {    // T is intentionally in global namespace!
  struct S obj_S;

  T() : obj_S()
  {
    return;
  }

  T(int i) : obj_S(i)
  {
    return;
  }

  T(int i, int j) : obj_S(i, j)
  {
    return;
  }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // 'new/delete' & 'malloc()/free()' are to be covered lately in the
  // test-suite ->> only static allocation on stack for now:
  T obj_1;
  T obj_2(89);
  T obj_3(13, 69);

  // Destructors are called automatically when main() returns.
  return 0;
}
