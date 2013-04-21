/**
* @file test-0068.c
*
* @brief Using global variables.
*/

int globVar = 10;
float modifGlobVar = 20;

int main(int argc, const char *argv[]) {
	int i = globVar;
	float f = modifGlobVar;
	modifGlobVar *= 2;

	return 0;
}
