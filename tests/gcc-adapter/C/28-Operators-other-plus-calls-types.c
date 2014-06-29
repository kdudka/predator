/* Test case: #28 */

struct test {
  int value;
  int array[42];
};

int initval(struct test t)
{
  return t.value;                 /* Call by value. */
}

int retval(struct test *t_ptr)
{
  return t_ptr->value;            /* Call by reference. */
}


int main(void)
{
  struct test t, *t_ptr = &t;
  
  t.value = 0;                    /* Access by '.' notation. */

  /* Structure member access via pointer + array member access. */
  t_ptr->array[0] = 89;
  t_ptr->array[1] = retval(&t);

  return retval(t_ptr);
}
