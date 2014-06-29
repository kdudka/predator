/* Test case: #25 */

int main(void)
{
  int i = 0;

  i++, i = (i == 0) ? 42 : 89;

  return 0;
}
