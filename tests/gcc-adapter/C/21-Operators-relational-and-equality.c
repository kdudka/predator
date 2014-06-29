/* Test case: #21 */

int main(void)
{
  int truth_value, value1 = 42, value2 = 89;

  truth_value = 42 < 89;
  truth_value = 42 <= 89;
  truth_value = 42 > 89;
  truth_value = 42 >= 89;

  truth_value = 42 == 89;
  truth_value = 42 != 89;

  truth_value = value1 < value2;
  truth_value = value1 <= value2;
  truth_value = value1 > value2;
  truth_value = value1 >= value2;

  truth_value = value1 == value2;
  truth_value = value1 != value2;

  return 0;
}
