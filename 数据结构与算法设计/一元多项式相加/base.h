//base.h

//----常用的头文件--------
#include <iostream>
using namespace std;
#include <malloc.h>

//-----公用的常量和类型----------
#define   OK                    1
#define   ERROR                 0
#define   OVERFLOW             -2
#define   TRUE                  1
#define   FALSE                 0

typedef   int   Status;

//---------一元多项式存储结构表示----------
typedef struct{//项的表示，多项式的项作为LinkList的数据元素
    float coef;//系数
    int expn;//指数
}term, ElemType;//两个类型：term用于本ADT，ElemType为LinkList的数据对象名
