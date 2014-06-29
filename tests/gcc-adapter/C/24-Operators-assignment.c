/* Test case: #24 */

int main(void)
{
  long int_result;
  float float_result;

  int_result = 0;
  int_result += 42;
  int_result -= 89;
  int_result *= 256;
  int_result /= 10;
  int_result %= 3;
  int_result <<= 8;
  int_result >>= 3;
  int_result |= 42;
  int_result &= 89;
  int_result ^= 127;

  float_result = 0;
  float_result += 3.14;
  float_result -= 2.72;
  float_result *= 99.9;
  float_result /= 100;

  return 0;
}
