#include <list>
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

    list<int> sll = SLL_create(n,A);
    SLL_destroy(sll);

    return 0;
}
