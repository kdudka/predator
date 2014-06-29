/* Test case: #07 */

int main(void)
{
  short int si = -32767;
  unsigned short int usi = 65535U;

  int i = -42;
  unsigned int ui = 89U;

  long int li = -2147483648;
  unsigned long int uli = 4294967295U;

#ifdef __LP64__
  long long int lli = -9223372036854775808;
  unsigned long long int ulli = 18446744073709551615U;
#endif

  return 0;
}
