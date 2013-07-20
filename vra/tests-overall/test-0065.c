/**
* @file test-0065.c
*
* @brief Nestes structures.
*/

int main(int argc, const char *argv[]) {
	struct MyStruct {
		int a;
		int b;
	};

	struct MyStruct var = { 1, 2 };

	struct MyStruct x[2][2] = { {var, var}, {var, var}};

	struct {
		struct MyStruct x[2][2];
	} q;

	q.x[0][0] = x[1][1];

	return 0;
}
