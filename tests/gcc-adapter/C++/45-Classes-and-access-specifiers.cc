/* Test case: #45
 *
 * Copyright NOTE: This file is part of predator's test suite.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator. If not, see <http://www.gnu.org/licenses/>.
 */

#include <new>

class C {
  // implicitly private:
    char c1;
    signed char c2;
    unsigned char c3;

  public:
    short s1;
    signed short s2;
    unsigned short s3;

  protected:
    int i1;
    signed int i2;
    unsigned int i3;

  private:
    long l1;
    signed long l2;
    unsigned long l3;

  public:
    float f1;

  private:
    double d1;

  protected:
    long double ld1;

  public:
    C() : c1('#'), c2('&'), c3('@'), s1(42), s2(-42), s3(89U)
    {
      i1 = 69;
      i2 = -72;
      i3 = 75U;

      l1 = 1024L;
      l2 = -2048L;
      l3 = 4096UL;

      f1 = 3.14F;
      d1 = 19e6F;
      ld1 = 42e42L;

      return;
    }

    ~C()
    {
      c1 = 0;
      c2 = 0x0;
      c3 = '\0';

      i1 = (int) l1;
      i2 = (signed int) l2;
      i3 = (unsigned int) l3;
    }
};

struct S {
  // implicitly public:
    char c1;
    signed char c2;
    unsigned char c3;

  private:
    short s1;
    signed short s2;
    unsigned short s3;

  protected:
    int i1;
    signed int i2;
    unsigned int i3;

  public:
    long l1;
    signed long l2;
    unsigned long l3;

  private:
    float f1;

  protected:
    double d1;

  public:
    long double ld1;

    S() : c1('#'), c2('&'), c3('@'), s1(42), s2(-42), s3(89U)
    {
      i1 = 69;
      i2 = -72;
      i3 = 75U;

      l1 = 1024L;
      l2 = -2048L;
      l3 = 4096UL;

      f1 = 3.14F;
      d1 = 19e6F;
      ld1 = 42e42L;

      return;
    }

    ~S()
    {
      c1 = 0;
      c2 = 0x0;
      c3 = '\0';

      i1 = (int) l1;
      i2 = (signed int) l2;
      i3 = (unsigned int) l3;
    }
};

union U {
  // implicitly public:
    char c1;
    signed char c2;
    unsigned char c3;

  private:
    short s1;
    signed short s2;
    unsigned short s3;

  protected:
    int i1;
    signed int i2;
    unsigned int i3;

  public:
    long l1;
    signed long l2;
    unsigned long l3;

  private:
    float f1;

  protected:
    double d1;

  public:
    long double ld1;

    U() : c1('#')
    {
      return;
    }

    ~U()
    {
      ld1 = 0.0L;
    }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // Automatic allocation on stack:
  C class_1 = C();
  S struct_1 = S();
  U union_1 = U();

  class_1.s1++;
  class_1.s2 -= class_1.s3;

  struct_1.c1--;
  struct_1.c2 += struct_1.c3;

  union_1.l1 = (signed long) class_1.s2 - struct_1.l2;

  C *C_ptr = new C();
  S *S_ptr = new S();
  U *U_ptr = new U();

  C_ptr->s3 = S_ptr->c3;
  S_ptr->c2 = C_ptr->s2;
  U_ptr->ld1 = union_1.l1;    // Some random number.

  delete C_ptr;
  delete S_ptr;
  delete U_ptr;

  return 0; // Destructors called for class_1, struct_1 and union_1.
}
