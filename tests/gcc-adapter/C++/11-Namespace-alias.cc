/* Test-case: #11 */

/*
 * Example taken from (with some modifications):
 * http://en.cppreference.com/w/cpp/language/namespace#Using-directives
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
