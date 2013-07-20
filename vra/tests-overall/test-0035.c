/**
* @file test-0035.c
*
* @brief Complicated structure.
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
				float i[10];
			} h;
			int j;
		} d;
	} y;

	struct my_struct x = { .a = 1, .b = 1.3, .c = 12, .d.e = 100, .d.f = 1.45,
			 			   .d.g = 45.85, .d.h.i = {0,1,2,3,4,5,6,7,8,9}, .d.j = 1578};

	y = x;
	x.d.h.i[0] = 20;
	x.d.h.i[1] = 30;
	x.d.e = 10;

	return 0;
}

