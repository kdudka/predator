/* Test case: #14
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

char C = '0';
int I = 42;
long L = 89;
double D = 3.14;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  char &C_ref = C;
  int &I_ref = I;
  long &L_ref = L;
  double &D_ref = D;

  char &C_ref2 = C_ref;
  int &I_ref2 = I_ref;
  long &L_ref2 = L_ref;
  double &D_ref2 = D_ref;

  C_ref = 42;
  I_ref++;
  L_ref--;
  D_ref = 2.72;
  
  // Integral division:
  L_ref2 = (L_ref2 * C_ref2 * L_ref * C_ref) / (I_ref2 * I_ref);
  // Floating-point division:
  D_ref2 = (D_ref2 * D_ref) / (L_ref2 * L_ref);

  return 0;
}
