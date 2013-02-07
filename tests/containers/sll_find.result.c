#include <list>
using namespace std;

list<int> SLL_create(int const n, int const* const A)
{
    list<int> sll;
    for (int i = 0; i < n; ++i)
        sll.push_front(A[i]);
    return sll;
}

list<int>::const_iterator SLL_find(list<int>& sll, int const x)
{
    for (list<int>::const_iterator p = sll.begin(); p != sll.end(); ++p)
        if (*p == x)
            return p;
    return sll.end();
}

list<int>::const_iterator SLL_find_rec(list<int>& sll, list<int>::const_iterator const it,
                                       int const x)
{
    if (it == sll.end())
        return sll.end();
    return (*it == x) ? it : SLL_find_rec(sll,next(it,1),x);
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

    list<int>::const_iterator it1 = SLL_find(sll,3);
    list<int>::const_iterator it2 = SLL_find_rec(sll,sll.begin(),3);

    SLL_destroy(sll);

    return 0;
}
