/* Test-case: #03 */

namespace ns {
  char c = 42;
}

namespace ns {
  int i = 89;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main(void)
{
  ns::c -= 2;
  ns::i += 3;

  return 0;
}
