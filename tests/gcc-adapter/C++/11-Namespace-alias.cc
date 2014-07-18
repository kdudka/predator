/* Test case: #11
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * However, the example(s) below were taken from (and modified):
 * <http://en.cppreference.com/>
 *
 * Because of it, this file is licensed under GFDL v1.3 and CC BY-SA 3.0
 * licenses as requested at: <http://en.cppreference.com/w/Cppreference:FAQ>
 *
 * You should have receive copies of these licenses in the ~/predator/tests/
 * folder with the predator itself. If not, please, see their online versions:
 * <http://www.gnu.org/copyleft/fdl.html>
 * <https://creativecommons.org/licenses/by-sa/3.0/legalcode>
 */

namespace live {
  namespace universe {
    namespace everything {
      long answer = 42;
    }
  }
}

namespace question = live::universe::everything;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  question::answer++;

  return --question::answer;  // 43? No way! :)
}
