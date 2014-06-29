/* Test case: #30 */

char *STRING_GLOBAL = "Hello, Code-Listener!";

int main(void)
{
  char *empty_string1 = "";
  char empty_string2[1] = {'\0'};
  char *string_ptr;

  string_ptr = empty_string1;
  string_ptr = STRING_GLOBAL;
  string_ptr = empty_string2;

  return 0;
}
