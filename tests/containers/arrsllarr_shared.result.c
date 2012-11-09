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
    struct Inner* data;
    list<Inner*> next;
};

struct Outer
{
    float f;
    list<Inner*> sll_1;
    int m;
    struct SLL sll_2;
};

list<Inner*> SLL_create(int const n, Inner* const data)
{
    list<Inner*> sll;
    for (int i = 0; i < n; ++i)
    {
        sll.push_front(data);
        /* or equally:
        sll.push_front(Inner());
        sll.front() = data;
        */
    }
    return sll;
}

void SLL_destroy(list<Inner*>& sll)
{
    while (!sll.empty())
        sll.pop_front();
}


int main()
{
    vector<Outer> outer;
    outer.resize(5);
    struct Inner inner;
    inner.c = 'A';
    inner.n = 3;
    inner.arr.resize(inner.n);
    for (int j = 0; j < inner.n; ++j)
        inner.arr.at(j) = j;
    for (int i = 0; i < 5; ++i)
    {
        outer.at(i).f = (float)i + 0.5f;
        outer.at(i).m = i;
        outer.at(i).sll_1 = SLL_create(i,&inner);
        outer.at(i).sll_2.data = outer.at(i).sll_1.front();
        outer.at(i).sll_2.next = list<Inner*>(next(outer.at(i).sll_1.begin(),1),
                                              outer.at(i).sll_1.end());
    }

    for (int i = 0; i < 5; ++i)
        SLL_destroy(outer.at(i).sll_1);
    inner.arr.clear();
    outer.clear();

    return 0;
}
