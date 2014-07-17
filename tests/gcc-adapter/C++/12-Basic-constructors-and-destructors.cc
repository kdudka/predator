/* Test-case: #12 */

class C {
    int i;

  public:
    // Basic constructors:
    C() : i(42)
    {
      return;
    }

    C(int i) : i(i)
    {
      return;
    }

    C(int i, int j) : i(i)
    {
      i += j;
      return;
    }

    //  Destructor:
    ~C()
    {
      return;
    }
};

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  // 'new' / 'delete' & 'malloc()' / 'free()' are to be covered lately in the
  // test-suite ->> only static allocation on stack for now:
  C cls_1;
  C cls_2(89);
  C cls_3(13, 69);

  // Destructors are called automatically when main() returns.
  return 0;
}
