/* Test-case: #01 */

namespace ns{
  char c = 42;
  int i = 89;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  ns::c++;
  ns::i--;

  return 0;
}
