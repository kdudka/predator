/* Test case: #20 */

int main(void)
{
  long result_int = 0;
  float result_real = 0.0;

  result_int = result_int + 42;
  result_int = result_int - 89;
  result_int = result_int * 10;
  result_int = result_int / 5;
  result_int = result_int % 2;
  result_int = ((result_int + 10) * 3) % ((result_int - 42) / 6);

  result_real = result_real + 3.14;
  result_real = result_real - 2.72;
  result_real = result_real * 10;
  result_real = result_real / 256;
  result_real = ((result_real + 99.9) / 16.6) * ((result_real - 33.3) / 66.6);

  return 0;
}
