/* Test case: #39 */

int main(void)
{
  unsigned i = 0;

  for ( ; ; i++) {
    if (i < 10)
      continue;
    else
      break;
  }

  while (1) {
    i++;

    if (i == 42)
      break;
  }

  do {
    if ((i % 3) == 0) {
      i++;
      continue;
    }

    i += 2;
  } while (i < 89);

  return 0;
}
