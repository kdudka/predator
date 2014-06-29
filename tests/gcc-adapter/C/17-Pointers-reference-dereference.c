/* Test case: #17 */

int main(void)
{
  char c1 = 0, c2 = 1; 
  char *c_ptr;
  char **c_pptr;

  c_ptr = &c1;
  *c_ptr = c2;
  c_ptr = &c2;
  *c_ptr = 0;
  c_pptr = &c_ptr;
  *c_pptr = &c1;
  **c_pptr = 1;

  return 0;
}
