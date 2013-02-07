#include <list>
using namespace std;

list<int> SLL_create(int const n, int const* const A)
{
    list<int> sll;
    for (int i = 0; i < n; ++i)
        sll.push_front(A[i]);
    return sll;
}

void SLL_copy(list<int>::const_iterator begin,
              list<int>::const_iterator const end,
              list<int>::iterator out)
{
    for ( ; begin != end; ++begin, ++out)
        *out = *begin;
}

void SLL_xcopy(list<int>::const_iterator begin,
               list<int>::const_iterator const end,
               list<int>& out,
               list<int>::iterator it)
{
    if (it != out.end())
        ++it;
    for ( ; begin != end; ++begin)
    {
        out.insert(it,*begin);
        /* or equally:
        int& val = *out.insert(it,int());
        val = *begin;
        */
    }
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

    list<int> sll_1 = SLL_create(n,A);
    list<int> sll_2 = SLL_create(2,A);

    SLL_copy(next(sll_1.begin(),2),next(sll_1.begin(),4),sll_2.begin());
    SLL_xcopy(sll_1.begin(),next(sll_1.begin(),5),sll_2,next(sll_2.begin(),1));

    SLL_destroy(sll_2);
    SLL_destroy(sll_1);

    return 0;
}
