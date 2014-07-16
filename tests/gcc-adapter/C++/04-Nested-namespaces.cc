/* Test-case: #04 */

namespace nsX {
  char c = 42;

  namespace nsY {
    int i = 89;

    namespace nsZ{
      float f = 3.14;

      namespace {
        long l = 2048;
      }
    }
  }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // //

int main()
{
  nsX::c += nsX::nsY::i;
  nsX::nsY::nsZ::f -= nsX::nsY::nsZ::l;

  return 0;
}
