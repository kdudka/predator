#include <stack>
using namespace std;

stack<int>& S_push(stack<int>& s, int const val)
{
    s.push(val);
    return s;
}

int S_top(stack<int>& s)
{
    return s.top();
}

stack<int>& S_pop(stack<int>& s)
{
    s.pop();
    return s;
}

bool S_empty(stack<int>& s)
{
    return s.empty();
}

void S_destroy(stack<int>& s)
{
    while (!s.empty())
        s.pop();
}

int main()
{
    stack<int> s;
    S_push(s,1);
    S_push(s,2);
    S_push(s,3);
    S_pop(s);
    S_destroy(s);
    return 0;
}
