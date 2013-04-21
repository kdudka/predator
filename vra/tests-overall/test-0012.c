/**
* @file test-0011.c
*
* @brief Assignment of uninitialized structure composed of other structure to structure.
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

	y = x;

	return 0;
}


