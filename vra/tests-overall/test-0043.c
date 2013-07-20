/**
* @file test-0043.c
*
* @brief Bit not/and/or/xor/left shift/right shif/left rotate/right rotate.
*/

int main(int argc, const char *argv[]) {
	// Bit not.
	unsigned a = 0;
	unsigned b = ~a;

	// Bit and.
	unsigned c = a & b;

	// Bit or.
	unsigned d = a | b;

	// Bit xor.
	unsigned e = 0b00110011 ^ 0b11001100;

	// Left shift.
	unsigned f = 100 << 2;

	// Right shift.
	unsigned g = 100 >> 2;

	// Left rotate.
	unsigned char h = 0b00001111;
	unsigned i = (h << 4) + (h >> 4);

	return 0;
}

