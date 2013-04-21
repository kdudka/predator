/**
* @file test-0011.c
*
* @brief Assignment of initialized structure composed of other structure to structure.
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
	} y;

	struct my_struct x = { .a = 1, .b = 1.3, .c = 12, .d.e = 100, .d.f = 1.45,
			 .d.g = 45.85, .d.h.i = 148963, .d.j = 1578};

	y = x;

	return 0;
}

