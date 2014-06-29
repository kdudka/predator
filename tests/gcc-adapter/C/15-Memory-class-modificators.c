/* Test case: #15 */

extern float NOT_HERE;
static float HERE;

int main(void)
{
  auto char i, j = 42;
  static int c = 0;
  register long r = 89;

  return 0;
}
