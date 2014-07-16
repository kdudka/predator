/* Test-case: #02 */

char c = 42;
int i = 89;

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // Unqualified names:
  c -= 2;
  i += 3;

  // Fully qualified names:
  ::c++;
  ::i--;

  return 0;
}
