//LinkList.h
//链式表的基本函数实现

//----------链表的存储结构表示------------
typedef struct LNode{//节点类型
    ElemType  data;//这里表示了每一项，其指数和系数
    struct LNode *next;
}*Link,*Position;
typedef struct{//链表
    Link head, tail;//分别指向线性链表中的头结点和最后一个结点
    int len;//指示线性链表中数据元素的个数
}LinkList;

//----------链表函数的具体实现代码-----------
Status InitList(LinkList *L){
    // 构造一个空的线性链表
    Link p;
    p = (Link)malloc(sizeof(LNode)); // 生成头结点
    if (p)
    {
        p->next = NULL;
        (*L).head = (*L).tail = p;
        (*L).len = 0;
        return OK;
    }
    else
        return ERROR;//内存分配不够
}//InitList

Status MakeNode(Link *p, ElemType e){
    // 分配由p指向的值为e的结点，并返回OK；若分配失败。则返回ERROR
    *p = (Link)malloc(sizeof(LNode));
    if (!*p)
        return ERROR;
    (*p)->data = e;
    return OK;
}//MakeNode

void FreeNode(Link *p){
    // 释放p所指结点
    free(*p);
    *p = NULL;
}//FreeNode

Status InsFirst(LinkList *L, Link h, Link s){
 // h指向L的一个结点，把h当做头结点，将s所指结点插入在第一个结点之前
    s->next = h->next;
    h->next = s;
    if (h == (*L).tail) // h指向尾结点
        (*L).tail = h->next; // 修改尾指针
    (*L).len++;
    return OK;
}//InsFirst

Position GetHead(LinkList L){
    // 返回线性链表L中头结点的位置
    return L.head;
}//GetHead

Status SetCurElem(Link p, ElemType e){
    // 已知p指向线性链表中的一个结点，用e更新p所指结点中数据元素的值
    p->data = e;
    return OK;
}//SetCurElem

Status LocateElemP(LinkList L, ElemType e, Position *q, int(*compare)(ElemType, ElemType)){
     // 若升序链表L中存在与e满足判定函数compare()取值为0的元素，则q指示L中
    // 第一个值为e的结点的位置，并返回TRUE；否则q指示第一个与e满足判定函数
    // compare()取值>0的元素的前驱的位置。并返回FALSE。（用于一元多项式）
    Link p = L.head, pp;
    do
    {
        pp = p;
        p = p->next;
    } while (p && (compare(p->data, e)<0)); // 没到表尾且p->data.expn<e.expn
    if (!p || compare(p->data, e)>0) // 到表尾或compare(p->data,e)>0
    {
        *q = pp;
        return FALSE;
    }
    else // 找到
    {// 没到表尾且p->data.expn=e.expn
        *q = p;
        return TRUE;
    }
}//LocateElemP

Position NextPos(Link p){
   // 已知p指向线性链表L中的一个结点，返回p所指结点的直接后继的位置
   //  若无后继，则返回NULL
    return p->next;
}//NextPos

ElemType GetCurElem(Link p){
     // 已知p指向线性链表中的一个结点，返回p所指结点中数据元素的值
    return p->data;
}//GetCurElem

Status DelFirst(LinkList *L, Link h, Link *q){ // 形参增加L,因为需修改L
  // h指向L的一个结点，把h当做头结点，删除链表中的第一个结点并以q返回。
  // 若链表为空(h指向尾结点)，q=NULL，返回FALSE
    *q = h->next;
    if (*q) // 链表非空
    {
        h->next = (*q)->next;
        if (!h->next) // 删除尾结点
            (*L).tail = h; // 修改尾指针
        (*L).len--;
        return OK;
    }
    else
        return FALSE; // 链表空
}//DelFirst

Status ListEmpty(LinkList L){
    // 若线性链表L为空表，则返回TRUE，否则返回FALSE
    if (L.len)
        return FALSE;
    else
        return TRUE;
}//ListEmpty

Status Append(LinkList *L, Link s){
  // 将指针s(s->data为第一个数据元素)所指(彼此以指针相链,以NULL结尾)的
  //  一串结点链接在线性链表L的最后一个结点之后,并改变链表L的尾指针指向新
  //  的尾结点
    int i = 1;
    (*L).tail->next = s;
    while (s->next)
    {
        s = s->next;
        i++;
    }
    (*L).tail = s;
    (*L).len += i;
    return OK;
}//Append

Status ListTraverse(LinkList L, void(*visit)(ElemType)){
    // 依次对L的每个数据元素调用函数visit()。一旦visit()失败，则操作失败
    Link p = L.head->next;
    int j;
    for (j = 1; j <= L.len; j++)
    {
        visit(p->data);
        p = p->next;
    }
    cout << "\b "; //退格，每次输出多项式后删掉的最后输出的"+"
    if (L.len == 0)
        cout << "0";
    return OK;
}//ListTraverse

void visit(ElemType e){
    if (e.coef > 0 && e.coef != 1 && e.expn != 0)
    {
        if(e.expn == 1)
            cout<< e.coef << "x+";
        else if (e.expn > 0)
            cout << e.coef << "x^" << e.expn << "+";
        else
            cout << e.coef << "x^(" << e.expn << ")+";
    }
    else if (e.coef < 0 && e.expn != 0)
    {
        if(e.expn == 1)
            cout<< "(" <<e.coef << ")x+";
        else if (e.expn > 0)
            cout << "(" << e.coef << ")x^" << e.expn << "+";
        else
            cout << "(" << e.coef << ")x^(" << e.expn << ")+";
    }
    else if (e.coef == 1 && e.expn != 0)
    {
        if(e.expn == 1)
            cout<< "x+";
        else if (e.expn > 0)
            cout << "x^" << e.expn << "+";
        else
            cout << "x^(" << e.expn << ")+";
    }
    else if (e.expn == 0 && e.coef != 0)
        cout << e.coef<<"+";
    else
        cout << "";//考虑用户输入可能有系数为0的情况
}//visit
