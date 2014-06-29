/* Test case: #32 */

char *GLOBAL_STRING = "Hello, Code-Listener!";


int main(void)
{
  char *str_ptr1, *str_ptr2;

  str_ptr1 = GLOBAL_STRING;
  str_ptr2 = str_ptr1 + 8;
  str_ptr1 = str_ptr2 - 5;
  str_ptr2 = (char *) GLOBAL_STRING[3];
  str_ptr1 = str_ptr2 + 18;
  str_ptr2 = GLOBAL_STRING + (str_ptr1 - str_ptr2);

  return 0;
}
