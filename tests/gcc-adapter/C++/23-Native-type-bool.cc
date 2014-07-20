/* Test case: #23
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

bool GLOBAL_FLAG = false;
static bool GLOBAL_STATIC_FLAG = true;
extern bool EXTERN_FLAG;

const bool C_GLOBAL_FLAG = true;
volatile bool V_GLOBAL_FLAG = false;
static const volatile bool CV_GLOBAL_STATIC_FLAG = true;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  static bool local_static_flag = true;
  bool local_flag = false;
  static const bool c_local_static_flag = true;
  static volatile bool v_local_static_flag = false;
  const volatile bool cv_local_flag = true;

  bool array_of_flags[11] = {
    GLOBAL_FLAG,
    GLOBAL_STATIC_FLAG,
    EXTERN_FLAG,
    C_GLOBAL_FLAG,
    V_GLOBAL_FLAG,
    CV_GLOBAL_STATIC_FLAG,
    local_static_flag,
    local_flag,
    c_local_static_flag,
    v_local_static_flag,
    cv_local_flag,
  };

  for (unsigned i = 0; i < 11; i++) {
    array_of_flags[i] = ~array_of_flags[i];
  }

  return 0;
}
