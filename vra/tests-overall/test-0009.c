/**
* @file test-0009.c
*
* @brief Assignment of structure composed of other structure to structure one by one item.
*/

int main(int argc, const char *argv[]) {

	struct my_struct{
		int a;
		float b;
		int c;
		struct {
			int e;
			float f;
			double g;
			struct {
				float i;
			} h;
			int j;
		} d;
	} x, y;

	x.a = 1;
	x.b = 1.3;
	x.c = 12;
	x.d.e = 100;
	x.d.f = 1.45;
	x.d.g = 45.85;
	x.d.h.i = 148963;
	x.d.j = 1578;

	y.a = x.a;
	y.b = x.b;
	y.c = x.c;
	y.d.e = x.d.e;
	y.d.f = x.d.f;
	y.d.g = x.d.g;
	y.d.h.i = x.d.h.i;
	y.d.j = x.d.j;

	return 0;
}


