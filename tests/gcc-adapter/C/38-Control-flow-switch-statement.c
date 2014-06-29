/* Test case: #38 */

enum enumerate {
  NONE,
  FIRST,
  SECOND,
  THIRD,
  LAST,
};


int main(void)
{
  enum enumerate e = LAST;

  switch (e) {
    case NONE :
      break;

    case FIRST :
      return 1;

    case SECOND :
      return 2;

    case THIRD :
      return 3;

    default :
      break;
  }

  return 0;
}
