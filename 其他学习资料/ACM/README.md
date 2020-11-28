A:题目意思很明确了，前k-1次是不成功，最后一次是成功。

```C++
#include <iostream>
#include <cmath>
using namespace std;
int t, k;
double x;
int main(void) {
    cin >> t;
    while (t--) {
        cin >> x >> k;
        printf("%.6lf\n", pow(1 - x, k - 1) * x );
    }   
}
```



B:让第k个数尽可能大，那就先对a，b数组排好序，然后我们要让第k大的数尽可能大的话。就直接取a，b数组后面的k~n的数。然后让a数组较大的一端和b数组较小的一端相加。然后取最小值就是我们要找的尽可能大的第k大的数

```c++
#include <iostream>
#include <algorithm>
using namespace std;
#define maxn 1000000
int N, K;
int a[maxn], b[maxn];
int ans = 0x7fffffff;
int main(void) {
    cin >> N >> K;
    for (int i = 1; i <= N; i++) cin >> a[i];
    for (int i = 1; i <= N; i++) cin >> b[i];
    sort(a + 1, a + N + 1);
    sort(b + 1, b + N + 1);
    int j = 1;
    for (int i = N - K + 1; i <= N; i++) {
        ans = min(ans, a[i] + b[N - j + 1]);
        j++;
    }
    cout << ans << endl;
    return 0;
} 
```



C:我出的== ，其实好好想想，我们只是取前32位。要得到一个数。我们任意一个32位的数都可以找到对应的值。从0 ~ 2147483647。从 -2147483648 ~ -1所以每个32位的数都是4294967296种。

```c++
#include <iostream>
using namespace std;
int main(void) {
    int n;
    cin >> n;
    cout << "4294967296" << endl;
    return 0;
}
```



D: 神仙题目，这边建议跳过，膜拜下大佬代码==

```C++
#include <bits/stdc++.h>
using namespace std;
 
const int MAXN = 2e2 + 5;
 
int l, la, ansl, nob, tot, mx, f[MAXN];
char a[MAXN], b[MAXN], ans[MAXN];
 
int main() {
    cin >> l >> a + 1, la = strlen(a + 1);
    for (int i = la; i >= l; i--)
        for (int j = 1; j <= la - i + 1; j++) {
            tot = 0;
            memset(b, 0, sizeof(b));
            for (int k = j; k <= j + i - 1; k++)
                b[k - j + 1] = a[k];
            memset(f, 0, sizeof(f));
            f[0] = -1;
            for (int k = 1, x = -1; k <= i; k++) {
                while (x >= 0 && b[x + 1] != b[k]) x = f[x];
                f[k] = ++x;
            }
            for (int k = 1, x = 0; k <= la; k++) {
                while (x >= 0 && b[x + 1] != a[k]) x = f[x];
                tot += (++x == i);
            }
            if (tot > mx) {
                mx = tot;
                for (int k = 1; k <= i; k++)
                    ans[k] = b[k];
                ansl = i; 
            }
        }
    for (int i = 1; i <= ansl; i++)
        cout << ans[i];
    return 0;
} 
```



E: 跟着思路打就对了，当逐个找质数，然后相乘取模，到n个数就退出输出答案就行。

```C++
#include <iostream>
#include <cmath>
using namespace std;
int n, c;
long long ans = 1;
 
bool is_true(int x) {
    bool flag = false;
    for (int i = 2; i <= sqrt(x); i++) {
        if (x % i == 0) {
            flag = true; break;
        }
    }
    if (flag) return false;
    else return true;
}
 
void init() {
    for (int i = 2; ; i++) {
        if(is_true(i)) {
            c++;
            ans = (ans * i) % 50000;
            if (c == n) break;
        }
    }
}
 
int main(void) {
    cin >> n;
    init();
    cout << ans << endl;
    return 0;
}
```



F:刚开始可以发现偶数是n个，奇数是n-1个，如果伊井野擦掉的是奇数的话，那就是n个偶数，n - 2个偶数。辉夜大小姐是以最优策略来擦的，所以最后一定是两个偶数，辉夜必胜!!!!

```c++
#include <iostream>
using namespace std;
long long n;
int main(void) {
    cin >> n;
    cout << n - 1 << endl;
    return 0;
}
```



G:适中的一道搜索题，看看应该能明白==

```c++
#include <iostream>
#include <vector>
using namespace std;
#define maxn 1000000
int n, m, a[maxn], ans;
vector<int> G[maxn];
int dfs(int now, int fa, int count) {
    if (G[now].size() == 1 && G[now][0] == fa) return 1;
    for (int i = 0; i < G[now].size(); i++) {
        if (G[now][i] == fa) continue;
        if (a[G[now][i]]) {
            if (count + 1 > m) continue;
            else ans += dfs(G[now][i], now, count + 1);
        }
        else ans += dfs(G[now][i], now, 0);
    }
}
int main(void) {
    cin >> n >> m;
    for (int i = 1; i <= n; i++) cin >> a[i];
    for (int i = 1; i <= n - 1; i++) {
        int a, b; cin >> a >> b;
        G[a].push_back(b); G[b].push_back(a);
    }
    dfs(1, 0, a[1] == 1 ? 1 : 0);
    cout << ans << endl;
    return 0;
}
```



H:利用前缀和思想,得出每个温度的推荐食谱本数,进而得出每个温度是否是推荐温度,再次通过前缀和回答问题

```c++
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <iostream>
using namespace std;
typedef long long ll;
const int N = 2e5 + 50;
int d[N],a[N];
int t[N], n,k,q;
int lowbit(int x){ return x & -x;}
void add(int x, int v) {
    while(x <= n) {
        t[x] += v;
        x += lowbit(x);
    }
}
int sum(int x) {
    int ans = 0;
    while(x) {
        ans += t[x];
        x -= lowbit(x);
    }
    return ans;
}
void pre() {
}
 
void solve(int _case) {
    scanf("%d %d %d", &n, &k, &q);
    int rmax = 0;
    for(int i = 1; i <= n; i++) {
        int l,r;
        scanf("%d %d", &l, &r);
        d[l]++;
        d[r+1]--;
        rmax = max(rmax, r);
    }
    n = rmax + 5;
    for(int i = 1; i <= n; i++) a[i] += a[i-1] + d[i];
    for(int i = 1; i <= n; i++) if(a[i] >= k) add(i,1);
    for(int i = 1; i <= q; i++) {
        int a,b;
        scanf("%d %d", &a, &b);
        b = min(b,n);
        a = min(a,n);
        int ans = sum(b) - sum(a-1);
        printf("%d\n", ans);
    }
}
int main() {
    pre();
    int T;
    T = 1;
    for(int i = 1; i <= T; i++) {
        solve(i);
    }
    return 0;
}
```





I:枚举两张卡片,若某个属性相同则第三张相同,不同则第三张也不同,就确定下了第三张卡片,但是这样枚举会重复计算,所以要去重

```c++
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <iostream>
using namespace std;
typedef long long ll;
const int N = 2e4 + 50;
void pre() {
}
string s[N];
void solve(int _case) {
    int n,k;
    map<string, int> mp;
    cin >> n >> k;
    for(int i = 1; i <= n; i++) {
        cin >> s[i];
        mp[s[i]]++;
    }
    int ans = 0;
    for(auto pr : mp) {
        if(pr.second >= 3) {
            ans += 1LL*pr.second*(pr.second-1)*(pr.second-2)/6;
        }
    }
    string tmp(k,' ');
    for(int i = 1; i <= n; i++) {
        int cnt = 0;
        for(int j = i+1; j <= n; j++) {
            if(s[i] == s[j]) continue;
            for(int it = 0; it < k; it++) {
                if(s[i][it] == s[j][it]) {
                    tmp[it] = s[i][it];
                }
                else
                    tmp[it] = 'Q'^'A'^'B'^s[i][it]^s[j][it];
            }
            cnt += mp[tmp];
     
        }
        cnt /= 2;
        ans += cnt;
        mp[s[i]]--;
    }
    printf("%d\n", ans);
}
int main() {
    // freopen("data.in","r",stdin);
    pre();
    int T;
    // scanf("%d", &T);
    T = 1;
    for(int i = 1; i <= T; i++) {
        solve(i);
    }
    return 0;
}
```





J:找规律可以发现和斐波那契数列有关

```c++
#include<cstdio>
#include<iostream>
using namespace std;
#define R register
int N,T;
long long fib[90];
int main() {
    scanf("%d", &T);
    fib[0] = 0, fib[1] = 1, fib[2] = 2;
    for(R int i = 3;i < 85; ++i) fib[i] = fib[i - 1] + fib[i - 2];
    while(T--) {
        scanf("%d", &N);
        printf("%lld\n", fib[N] + fib[N + 1]);
    }
    return 0;
}
```



K:直接暴力O(N^2)划分就行

```C++
#include<cstdio>
#include<cstring>
#include<iostream>
using namespace std;
#define ld double
 
char s[300];
int len;
 
inline bool pd(int x,int y){
    string t="";
    for(int i=1;i<=len;++i)
        if(x<=i && i<=y) continue;
        else t+=s[i];
    if(t=="WELCOMETOACM") return 1;
    else return 0;
}
 
int main()
{
    scanf("%s",s+1); len=strlen(s+1);
    for(int i=1;i<=len;++i)
        for(int j=i;j<=len;++j)
            if(pd(i,j)) {printf("Yes!"); return 0;}
    printf("No.");
    return 0;
}
```







