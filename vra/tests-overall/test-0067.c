/**
* @file test-0067.c
*
* @brief Using global variables.
*/

struct Info {
	float weight;
	float height;
	int age;
};

struct Info donaldDuck = {20.3, 50.1, 100};
struct Info mickeyMouse = {40.5, 78.4, 120};

int main(int argc, const char *argv[]) {
	struct Info donalDuck2 = donaldDuck;
	struct Info mickeyMouse2 = mickeyMouse;
	mickeyMouse.height = 65.4;

	return 0;
}

