/* Test case: #34 */

int main(void)
{
  char value1 = 1, value2 = 0;

  if (value1 == 1)
    value2 = 1;

  if (value2 == 0)
    value1 = 0;
  else
    value1 = 2;

  if (value1 != 1) {
    value1++;
    value2--;
  }
  else {
    value1--;
    value2++;
  }

  return (value1) ? 0 : 1;
}
