/**
* @file test-0074.c
*
* @brief Structures and arrays.
*/

int main(int argc, const char *argv[]) {
	struct s1{
		int a;
	} c;

	struct s2{
		struct s1 b[2];
	} d;

	d.b[0] = c;

	return 0;
}
