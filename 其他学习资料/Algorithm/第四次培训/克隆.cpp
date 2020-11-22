[题目链接](https://ac.nowcoder.com/acm/contest/8564/J)
```
#include <iostream>
#include <algorithm>
#include <set>
#include <queue>
#include <cstdio>
#include <vector>
#include <map>
#include <stack>
using namespace std;
#define For(i, a, b) for(int i = (a), _ = (b); i <= _; i++)
#define IO ios::sync_with_stdio(false);cin.tie(0);cout.tie(0)
#define maxn 5000010
#define inf 0x7fffffff
int n, m, t, k, tot, first[maxn], vis[maxn];
struct node {
    int to, next;
}e[maxn];
void add(int a, int b) {
    e[++tot].next = first[a];
    first[a] = tot;
    e[tot].to = b;
}
vector<int> s, ans;

void dfs(int x) {
    s.push_back(x);
    for (int i = first[x]; i; i = e[i].next) {
        if (!vis[e[i].to]) {
            vis[e[i].to] = 1;
            dfs(e[i].to);
            s.push_back(x);
        }
    }
}
int main() {
    IO;
    cin >> n >> m >> k;
    t = (2 * n) / k + ((2 * n) % k == 0 ? 0 : 1);
    For (i, 1, m) {
        int a, b; cin >> a >> b;
        add (a, b); add(b, a);
    }
    vis[1] = 1;
    dfs(1);
    cout << "YES" << endl;
    int count = 0;
    while (k--) {
        ans.clear();
        while (count < s.size() && ans.size() < t) ans.push_back(s[count++]);
        cout << ans.size() << " ";
        for (int x : ans) cout << x << " ";
        cout << endl;
    }

    return 0;
}

```



