/* Test case: #36 */

int main(void)
{
  unsigned i = 15, j = 0;

  do {
    i--;
    j++;
  } while (i > 0);

  do
    i++;
  while (i < j);

  return 0;
}
