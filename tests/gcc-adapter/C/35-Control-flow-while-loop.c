/* Test case: #35 */

int main(void)
{
  unsigned i = 15, j = 0;

  while (i > 0) {
    i--;
    j++;
  }
  
  while (i < j)
    i++;

  return 0;
}
