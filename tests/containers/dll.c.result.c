#include <list>
using namespace std;

int main()
{
    list<int> dll;

    int k;
    for (k = 0; k < 3; ++k)
    {
        int i;
        for (i = 0; i < 10; ++i)
            dll.push_back(i);

        for (i = 0; i < 10; ++i)
            dll.push_front(10+i);

        if (k == 1)
            dll.clear();
    }

    dll.clear();

    return 0;
}
