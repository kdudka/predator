#include <list>
#include <vector>
using namespace std;

list<vector<int> > SLL_create(int const n, int** const A, int const* m)
{
    list<vector<int> > sll;
    for (int i = 0; i < n; ++i)
    {
        sll.push_front(vector<int>());
        sll.front().resize(m[i]);
        for (int j = 0; j < m[i]; ++j)
             sll.front().at(j) = A[i][j];
    }
    return sll;
}

void SLL_destroy(list<vector<int> >& sll)
{
    while (!sll.empty())
        sll.pop_front();
}

int main()
{
    int A0[] = { 1, 2, 3};
    int A1[] = {4, 5};
    int A2[] = {6, 7, 8};
    int m[3] = { 3, 2, 3 };
    int* A[3] = { A0, A1, A2 };

    list<vector<int> > sll = SLL_create(3,A,m);
    SLL_destroy(sll);

    return 0;
}
