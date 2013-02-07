#include <list>
using namespace std;

list<int*> SLL_create(int const n, int** const A)
{
    list<int*> sll;
    for (int i = 0; i < n; ++i)
    {
        sll.push_front((int*)(0));
        sll.front() = A[i];
        /* or simply: sll.push_front(A[i]); */
    }
    return sll;
}

void SLL_destroy(list<int*>& sll)
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

    list<int*> sll = SLL_create(3,A);
    SLL_destroy(sll);

    return 0;
}
