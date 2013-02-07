#include <vector>
using namespace std;

int main()
{
    vector<int> A;

    for (int i = 0; i < 10; ++i)
        A.push_back(i);

    for (int i = 0; i < A.size(); ++i)
        if (A.at(i) = 3)
            break;

    A.clear();

    return 0;
}
