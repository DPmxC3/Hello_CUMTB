A: a + b问题。 C++要写高精度，何不用Python？ 爽到飞起
```
n = int(input());
for i in range(n):
    a,b = input().split(' ');
    print(int(a) + int(b));
```

B：在x轴和y轴分别有n个点,组成n条不相交的线段,那么一定会是将x,y排序后,依次对应的两个点构成一条线段二分答案,二分最后一条与询问线段相交的线段,这条线段左下部分的线段一定与询问线段相交
```
#include<cstdio>
using namespace std;
#define R register
#define ll long long
#define N 1000005

ll ave,ans;
int a[N],b[N],c[N],n;

inline int abs(int x){return x<0 ? -x : x;}

inline void merge(int x,int y){
	if(y-x>1){
		R int m=x+y>>1;
		R int p=x,q=m,i=x;
		merge(x,m);
		merge(m,y);
		while(p<m || q<y)
			if(q>=y || (p<m&&b[p]<=b[q])) c[i++]=b[p++];
			else c[i++]=b[q++];
		for(R int i=x;i<y;++i) b[i]=c[i];
	}
}

inline int dy(){
	//freopen("candy.in","r",stdin);
	//freopen("candy.out","w",stdout);
	scanf("%d",&n);
	for(R int i=1;i<=n;++i) 
		scanf("%d",&a[i]),ave+=a[i];
	ave/=n;
	for(R int i=1;i<=n;++i) b[i]=b[i-1]+ave-a[i];
	merge(1,n+1);
	ave=b[1+n>>1];
	for(R int i=1;i<=n;++i) ans+=abs(b[i]-ave);
	printf("%lld\n",ans);
	return 0;
}
int QAQ = dy();
int main(){;}
```

C: 题意：给一个N，K，首先对于所有1，k（1<=k<=K），都是符合要求的，然后所有的符合要求的数的数（n，k），若n+k<N，则(n+k，k)也是好的，若nk<N，则（nk，k）也是好的，求符合要求的数量。

题解：首先可以先写一写，举个栗子，（1，3）。

可以变为（4，3），（3，3）。

现在应该很容易发现，乘法操作已经没用了，因为乘3必然是3的倍数，一定可以通过加3到达，所以可以写成两个不等式。

1+m*k<=N，

m*k<=N，

m表示符合要求的个数，然后暴力枚举k，算出每个m既可，K过大枚举超时，但是我们可以发现k中间有很多的m都相等，呈线性关系，故可用整除分块优化。
```
#include<iostream>
#define ll long long
using namespace std;
const ll mod=1e9+7;
ll n,k,ans,m,to,now;
int main(){
	scanf("%lld%lld",&n,&k);
	for(ll i=2;i<=k;i++){
		now=i;
		m=n/i;
		if(m==0)break;
		to=min(n/m,k);
		i=to;
		m%=mod;
		to%=mod;
		ans=(ans+m*(to-now+1))%mod;
		//printf("%lld %lld %lld %lld\n",now,to,ans,i);
	}
	for(ll i=2;i<=k;i++){
		now=i;
		m=(n-1)/i;
		if(m==0)break;
		to=min((n-1)/m,k);
		i=to;
		m%=mod;
		to%=mod;
		ans=(ans+m*(to-now+1))%mod;
		//printf("%lld %lld %lld i=%lld\n",now,to,ans,i);
	}
	printf("%lld\n",(ans+k+n-1)%mod);
}
```

D: 找规律的题目，自己看看就明白
```
#include <iostream>
using namespace std;
typedef unsigned long long ll;
ll q[3][3]={0};
ll m=0,n=0,p=0,c=1;
ll x;
int main()
{
	cin>>x;
	for (ll i=2;i<=x;i++) {
		if (i%2==0) {
			p=n+c;
			m=n+n;
		}
		else {
			n=m+p;
			c=p+p+1;
		}
	}
	cout<<"A->B:"<<p<<'\n';
	cout<<"A->C:"<<c<<'\n';
	cout<<"B->A:"<<n<<'\n';
	cout<<"B->C:"<<p<<'\n';
	cout<<"C->A:"<<m<<'\n';
	cout<<"C->B:"<<n<<'\n';
	cout<<"SUM:"<<m+c+n+n+p+p<<endl;
}
```


E: 


H: 如果做过NOIP2002均分纸牌的话,在看到这道题的时候应该很容易联想过去,然后打个假贪心,莫名其妙就A掉了,并且,这道题的小数据恰好是负载平衡问题的数据范围,也就是说,是负载平衡问题和均分纸牌的数据范围加强版,但是思路和做法是一样的数学证明: 最终的局面是每个小朋友的糖果数量相同,为平均数(ave)设编号为i的小朋友开始有Ai个糖果Xi表示第i个小朋友给了第i-1个小朋友Xi个糖果,Xi<0表示第i-1个小朋友给了第i个小朋友|Xi|个糖果则最终的答案ans=|X1|+|X2|+……+|Xn|对于第一个小朋友,他给了第n个小朋友X1个糖果,得到第2个小朋友的X2个糖果,最终还剩A1-X1+X2个糖果,即ave个糖果,所以得到方程A1-X1+X2=ave同理可得A2-X2+X3=ave......这几个方程综合一下:X2=ave-A1+X1=X1-C1 (C1=A1-ave)X3=2ave-A1-A2+X1+X2=X1-C2 (C2=A1+A2-X2-2ave)X4=3ave-A1-A2-A3+X1+X2+X3=X1-C3 (C3=A1+A2+A3-X2-X3-3ave)......ans=|X1|+|X1-C1|+|X1-C2|+......+|X1-Cn-1| |X1-Ci|的几何意义是数轴上点X1到Ci的距离问题就转化为: 给定数轴上n个点,找一个点使其到各点的距离和最小,而这个点就是中位数
```
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
#define X first
#define Y second
typedef long long ll;
const int N = 2e5 + 50;

void pre() {
    ios::sync_with_stdio(false);
    cin.tie(0);
}
int n,m;
int x[N],y[N],p;
int gettype(int x,int y) {
    int t = 0;
    t |= (x<0) << 1;
    t |= (y<0);
    return t;
}
vector<pair<int,int> > seg[4];
bool check(int ax,int by,int x,int y) {
    ax = abs(ax); by = abs(by); x = abs(x); y = abs(y);
    double py = double(by)/ax*(ax-x);
    return py <= y;
}
void solve(int _case) {
    cin >> n;
    for(int i = 1; i <= n; i++) cin >> x[i];
    for(int i = 1; i <= n; i++) cin >> y[i];
    sort(x+1, x+n+1);
    sort(y+1, y+n+1);
    int base = 0;
    for(int i = 1; i <= n; i++) {
        if(x[i] == 0 || y[i] == 0) base++;
        else {
            int t = gettype(x[i],y[i]);
            seg[t].push_back({x[i],y[i]});
        }
    }
    cin >> m;
    for(int i = 1; i <= m; i++) {
        int x,y;
        cin >> x >> y;
        int t = gettype(x,y);
        int l = 0,r = seg[t].size();
        while(l < r) {
            int mid = (l+r)>>1;
            if(check(seg[t][mid].X,seg[t][mid].Y,x,y)) l = mid+1;
            else r = mid;
        }
        printf("%d\n", r+base);
    }
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



J: 由于有一边与x或y平行，所以只有1为底，2为高，1为高2为底这两种情况，，，然后你画一个简单的图，把他们表示出来，注意不要重复运算，然后为了取模，将它变为（ab）%c=（（a%c）（b%c））%c的格式，也就是因式分解，就完成了，详细一点为，比方说有一个mn的格点，x为m，y方向n个，我们以2为底，平行于x轴，那么在m个格点里我们共有m-2个2，（比如三个格点只有一个2），那么它所有的上面一行的m个点，都可以构成好三角形，有m个，最上面一行无法向上构成，因而又（n-1）行，又，可以从上往下画三角形，同理，也是n-1行，每行有m-2个2，每个2对应m个点。然后这种三角形我们平行y轴，将刚刚式子，m和n对调即可。对于底为1，高为2的情况，同理每行有m-1个1，每个1对应m-2个点（这里减去两个是为了把和平行y轴，底为2的重复的去掉），显然有n-2行，上下再2，m、n再对调，加起来即为总和，因式分解到能够取模的地步，完成。
```
#include<bits/stdc++.h>
using namespace std;
long long n,m;
const int N = 1e9+7; 
int main(){
	cin >> n >> m ;
	cout<<(((n-2)*(m-1)%N+ (n-1)*(m-2)%N )%N *2*(m+n -2)%N)%N<<endl;
	return 0;	
}
```

K: 这就是模拟一下就出来了吧，给每个列记一下分数，然后输出最少的分数就行了。
```
#include <iostream>
using namespace std;
int a[100010];
int min_ = 0x7fffffff;
int main() {
    int n, m;
    cin >> n >> m;
    for(int i = 1; i <= m; i++) {
        int x; cin>>x;
        a[x]++;
    }
    for(int i = 1; i <= n; i++) {
        min_ = min_ > a[i] ? a[i] : min_;
    }
    cout << min_ << endl;
    return 0;
}
```
