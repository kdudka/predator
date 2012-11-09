#include <list>
using namespace std;

list<int> SLL_create(int const n, int const* const A)
{
    if (n == 0)
        return list<int>();

    list<int> sll = SLL_create(n-1,A+1);
    sll.push_front(A[0]);

    return sll;
}

void SLL_destroy(list<int>& sll, list<int>::const_iterator const it)
{
    if (!sll.empty())
    {
        SLL_destroy(sll,next(it,1));
        sll.erase(it);
    }
}

int main()
{
    int const A[] = { 1, 2, 3, 4, 5 };
    int const n = sizeof(A)/sizeof(A[0]);

    list<int> sll = SLL_create(n,A);
    SLL_destroy(sll,sll.begin());

    return 0;
}
