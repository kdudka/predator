/* Test case: #31 */

struct struct_flags {
  unsigned flag1      : 1;
  unsigned flag2      : 1;
  unsigned flag3      : 1;
  unsigned            : 2;    /* Filling. */
  unsigned wide_flag1 : 2;

  unsigned            : 0;    /* Alignment to next unsigned. */
  unsigned wide_flag2 : 4;
  unsigned            : 2;    /* Filling. */
  unsigned wide_flag3 : 2;
};

union union_with_flags {
  unsigned wide_flag1 : 8;
  unsigned wide_flag2 : 32;

  float f;
  double d;
  long l;
};


int main(void)
{
  struct struct_flags s;
  union union_with_flags u;

  s.flag1 = 0;
  s.flag2 = 1;
  s.flag3 = s.flag1;

  s.wide_flag1 = 0x3;
  s.wide_flag2 = 0xA;
  s.wide_flag3 = s.wide_flag1;

  u.d = 3.14;
  u.wide_flag1 = 0xFF;
  u.wide_flag1 = 0xF0F0;

  return 0;
}
