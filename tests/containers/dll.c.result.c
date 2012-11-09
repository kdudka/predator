#include <list>
using namespace std;

int main()
{
    list<int> dll;

    for (int k = 0; k < 3; ++k)
    {
        for (int i = 0; i < 10; ++i)
            dll.push_back(i);

        for (int i = 0; i < 10; ++i)
            dll.push_front(10+i);

        if (k == 1)
            dll.clear();
    }

    dll.clear();

    return 0;
}
