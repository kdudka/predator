/**
* @file test-0066.c
*
* @brief Using structures and enumeration.
*/

#define NumOfEmployees  2
#define MinSalary  5000

int main(int argc, const char *argv[]) {
	enum Color {
		RED = 0,
		GREEN,
		BLUE,
		YELLOW
	};

	struct PersonalInfo {
		int id;
		double weight;
		double height;
		int age;
		enum Color favouriteColor;
		int iq;
		int salary;
	};

	struct PersonalInfo Pat = {1234 , 80, 175, 35, YELLOW, 100, 0};
	struct PersonalInfo Mat = {1235, 82, 174, 34, RED, 99, 0};

	struct PersonalInfo employees[NumOfEmployees] = { Pat, Mat };

	int i;
	for (i = 0; i < NumOfEmployees; ++i) {
		if (employees[i].iq > 130) {
			employees[i].salary = 30000;
		} else {
			employees[i].salary = MinSalary;
		}
	}

	return 0;
}

