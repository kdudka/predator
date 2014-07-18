/* Test case: #04
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

namespace nsX {
  char c = 42;

  namespace nsY {
    int i = 89;

    namespace nsZ{
      float f = 3.14;

      namespace {
        long l = 2048;
      }
    }
  }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  nsX::c += nsX::nsY::i;
  nsX::nsY::nsZ::f -= nsX::nsY::nsZ::l;

  return 0;
}
