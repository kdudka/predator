#include <queue>
using namespace std;

queue<int> Q_make()
{
    return queue<int>();
}

void Q_put(queue<int>& q, int const val)
{
    q.push(val);
}

int Q_head(queue<int>& q)
{
    return q.front();
}

void Q_get(queue<int>& q)
{
    q.pop();
}

bool Q_empty(queue<int>& q)
{
    return q.empty();
}
void Q_destroy(queue<int>& q)
{
    while (!q.empty())
        q.pop();
}

int main()
{
    queue<int> q = Q_make();
    Q_put(q,1);
    Q_put(q,2);
    Q_put(q,3);
    Q_get(q);
    Q_destroy(q);
    return 0;
}
