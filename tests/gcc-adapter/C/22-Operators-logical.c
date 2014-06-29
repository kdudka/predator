/* Test case: #22 */

int main(void)
{
  int truth_value, true_value = 1, false_value = 0;

  truth_value = 0 || 1;
  truth_value = 0 && 1;
  truth_value = 1 && 0;
  truth_value = !truth_value;

  truth_value = false_value || true_value;
  truth_value = false_value && true_value;
  truth_value = true_value && false_value;
  truth_value = !truth_value;

  return 0;
}
