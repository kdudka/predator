#include <list>
#include <vector>
using namespace std;

list<int> SLL_create(int const n, int const* const A)
{
    list<int> sll;
    for (int i = 0; i < n; ++i)
        sll.push_front(A[i]);
    return sll;
}

void SLL_destroy(list<int>& sll)
{
    while (!sll.empty())
        sll.pop_front();
}

int main()
{
    int const A[] = { 1, 2, 3, 4, 5 };
    int const n = sizeof(A)/sizeof(A[0]);

    vector<list<int> > arrsll;
    arrsll.resize(3);
    for (int i = 0; i < 3; ++i)
        arrsll.at(i) = SLL_create(n,A);

    for (int i = 0; i < 3; ++i)
        SLL_destroy(arrsll.at(i));
    arrsll.clear();

    return 0;
}
