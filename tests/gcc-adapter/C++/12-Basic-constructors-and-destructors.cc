/* Test case: #12
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

class C {
    int i;

  public:
    // Basic constructors:
    C() : i(42)
    {
      return;
    }

    C(int i) : i(i)
    {
      return;
    }

    C(int i, int j) : i(i)
    {
      i += j;
      return;
    }

    //  Destructor:
    ~C()
    {
      return;
    }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // 'new/delete' & 'malloc()/free()' are to be covered lately in the
  // test-suite ->> only static allocation on stack for now:
  C cls_1;
  C cls_2(89);
  C cls_3(13, 69);

  // Destructors are called automatically when main() returns.
  return 0;
}
