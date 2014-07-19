/* Test case: #20
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

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  long l = 42;
  
  long &lval_ref1 = l;
  const long &lval_const_ref = l;
  volatile long &lval_volatile_ref = l;
  const volatile long &lval_const_volatile_ref = l;
  
  // NOTE: Casting not implemented/tested yet.
  // long &lval_ref2 = const_cast<long &>(cr);

  lval_ref1++;
  lval_volatile_ref = lval_ref1 + lval_const_ref - lval_const_volatile_ref;

  return 0;
}
