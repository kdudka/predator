/* Test case: #23 */

int main(void)
{
  char result, value = 42;

  result = 42 | 89;
  result = 42 & 218;
  result = 42 ^ 89;
  result = 42 << 2;
  result = 42 >> 1;
  result = ~42;

  result = value | 89;
  result = result & 218;
  result = result ^ 89;
  result = result << 2;
  result = result >> 1;
  result = ~result;

  return 0;
}
