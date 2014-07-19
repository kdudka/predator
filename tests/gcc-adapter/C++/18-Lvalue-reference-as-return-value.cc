/* Test case: #18
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

char C_STRING[] = "Hello, CoDe Listener!";
double G = 9.78F;

char &get_C_STRING_pos(unsigned pos)
{
  return C_STRING[pos];
}

double &get_gravitational_acc()
{
  return G;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  get_C_STRING_pos(9) = 'd';
  get_gravitational_acc() = get_gravitational_acc() + 0.02F;

  return 0;
}
