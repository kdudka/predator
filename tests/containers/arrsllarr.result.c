#include <list>
#include <vector>
#include <algorithm>
using namespace std;

struct Inner
{
    char c;
    vector<int> arr;
    int n;
};

struct SLL
{
    struct Inner data;
    list<Inner> next;
};

struct Outer
{
    float f;
    list<Inner> sll_1;
    int m;
    struct SLL sll_2;
};

list<Inner> SLL_create(int const n, Inner const* const data)
{
    list<Inner> sll;
    for (int i = 0; i < n; ++i)
    {
        sll.push_front(Inner());
        sll.front().c = data->c;
        sll.front().n = data->n;
        sll.front().arr.resize(data->n);
        copy(data->arr.begin(),data->arr.begin()+n,sll.front().arr.begin());
    }
    return sll;
}

void SLL_xcopy(list<Inner>::const_iterator begin,
               list<Inner>::const_iterator const end,
               list<Inner>& out,
               list<Inner>::iterator it)
{
    if (it != out.end())
        ++it;
    for ( ; begin != end; ++begin)
    {
        out.insert(it,*begin);
        /* or equally:
        Inner& inner = *out.insert(it,Inner());
        inner.c = begin->c;
        inner.n = begin->n;
        inner.arr.resize(begin->n);
        copy(begin->arr.begin(),begin->arr.begin()+begin->n,inner.arr.begin());
        */
    }
}

void SLL_destroy(list<Inner>& sll)
{
    while (!sll.empty())
    {
        sll.front().arr.clear();
        sll.pop_front();
    }
}


int main()
{
    vector<Outer> outer;
    outer.resize(5);
    for (int i = 0; i < 5; ++i)
    {
        struct Inner inner;
        outer.at(i).f = (float)i + 0.5f;
        outer.at(i).m = i;
        inner.c = 'A';
        inner.n = 4;
        inner.arr.resize(inner.n);
        for (int j = 0; j < inner.n; ++j)
            inner.arr.at(j) = i+j;
        outer.at(i).sll_1 = SLL_create(i,&inner);
        outer.at(i).sll_2.data = inner;
        outer.at(i).sll_2.next.clear();
        SLL_xcopy(outer.at(i).sll_1.begin(),next(outer.at(i).sll_1.begin(),i),
                  outer.at(i).sll_2.next,outer.at(i).sll_2.next.begin());
    }

    for (int i = 0; i < 5; ++i)
    {
        SLL_destroy(outer.at(i).sll_1);
        SLL_destroy(outer.at(i).sll_2.next);
    }
    outer.clear();

    return 0;
}
