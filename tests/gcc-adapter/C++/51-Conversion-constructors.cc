/* Test case: #51
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

class C {
    const char *str;
    long l;
    double d;
  public:
    C() : str(0x0), l(42), d(3.14) {};
    C(const char *s, long val = 89) : str(s), l(val), d(2.72) {};
    C(double d) : str(0x0), l(76), d(d) {};
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  class C c = "Code Listener";
  c = 2048;
  c = C(9.78);

  return 0;
}
