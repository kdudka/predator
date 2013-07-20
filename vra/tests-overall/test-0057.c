/**
* @file test-0057.c
*
* @brief Fibonacci computation.
*/

int main(int argc, const char *argv[]) {
    int i;        // The index of fibonacci number.
    int current;  // The value of the (i)th fibonacci number.
    int next;     // The value of the (i+1)th fibonacci number.
    int twoaway;  // The value of the (i+2)th fibonacci number.

	next = current = 1;
	for (i = 0; i < 10; i++) {
		twoaway = current+next;
		current = next;
		next    = twoaway;
	}

	return 0;
}

