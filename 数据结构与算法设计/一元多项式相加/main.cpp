#include "base.h"
#include "Linklist.h"
#include "Ploynomial.h"

int main()
{
    polynomial A, B;
    cout << "请输入第一个多项式的项数为：";
    int length; //一元多项式项数
    cin >> length;
    CreatPolyn(A, length);
    //显示A出来
    cout << "PA(x) = ";
    PrintPolyn(A);//打印一元多项式
    cout << endl;
    //输入B
    cout << "请输入第二个多项式的项数为：";
    cin >> length;
    CreatPolyn(B, length);
    //输出B
    cout << "PB(x) = ";
    PrintPolyn(B);
    cout << endl;
    //假设以上输入成功
    //进行相加
    AddPolyn(A, B);//一元多项式相加
    //这时候A是合并后的结果
    cout << "PA(x)+PB(x) = ";
    PrintPolyn(A);
    cout << endl;
    //cout<<"一元多项式的长度："<<PolynLength(A)<<endl;
    DestroyPolyn(A);
    return 0;
}
