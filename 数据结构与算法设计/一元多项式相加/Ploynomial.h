//Polynomial.h
//一元多项式基本操作头文件

typedef LinkList polynomial;

Status Remove_Polyn(LinkList *L, Link q){
    //由于项的指数为0，删除掉已有的项
    Link  h;
    h = L->head;
    while (h->next != q)
    {
        h = h->next;
    }
    //找到了
    if (q == L->tail)
    {//删除的如果是表尾，改变表尾
        L->tail = h;
    }
    h->next = q->next;
    free(q);
    L->len--;
    return OK;
}//Remove_Polyn

int cmp(term a, term b){// CreatPolyn()的实参
 // 依a的指数值<、=或>b的指数值，分别返回-1、0或+1
    if (a.expn == b.expn)
        return 0;
    else if(a.expn > b.expn)
        return 1;
    else
        return -1;
}//cmp

void CreatPolyn(polynomial &p,int m){
    //输入m项的系数和指数，建立表示一元多项式的有序链表P
    InitList(&p);//初始化-多项式链表
    Link h = GetHead(p);//设置头结点的数据元素
    ElemType e;//头结点设置
    Position q,s;
    e.coef = 0.0; e.expn = -1; SetCurElem(h, e);//设置头结点的元素
    for (int i = 1; i <= m; ++i)//依次输入m个非零项
    {
        cout << "第"<<i<<"项的系数:";
        cin >> e.coef;
        cout << "第" << i << "项的指数:";
        cin >> e.expn;
        if (!LocateElemP(p, e,&q, cmp))//当前链表中不存在该指数项
        {
            if (e.coef != 0)//不等于才插入
                if (MakeNode(&s, e))
                    InsFirst(&p,q,s);//生成结点并插入链表
        }
        else//当前链表中存在该指数项,增加其系数
        {
            q->data.coef = q->data.coef + e.coef;
            //如果合起来等于0，则删除掉
            if (q->data.coef == 0)
                Remove_Polyn(&p, q);//删除掉当前节点
        }
    }
}//CreatPolyn

void DestroyPolyn(polynomial &p){
    //销毁一元多项式
    Link h,s;
    h = p.head;
    while(h){
        s = h;
        h = h->next;
        FreeNode(&s);
    }
    p.head = p.tail = NULL;
}//DestroyPolyn


/*int PolynLength(polynomial p){
    //返回一元多项式的长度
    return p.len;
}*/

void PrintPolyn(polynomial p){
    //打印出一元多项式
    ListTraverse(p, visit);
}
void AddPolyn(polynomial &Pa, polynomial &Pb){
    //多项式加法:Pa = Pa+Pb,利用两个多项式的结点构成“和多项式”
    Position ha, hb, qa, qb;
    term a, b;
    ha = GetHead(Pa); hb = GetHead(Pb);//ha和hb分别指向Pa和Pb的头结点
    qa = NextPos(ha); qb = NextPos(hb);//qa和qb分别指向Pa和Pb中的当前结点
                                       //此时qa和qb都是指向多项式第一项
    while (qa && qb)//qa和qb非空
    {
        a = GetCurElem(qa);
        b = GetCurElem(qb); // a和b为两表中当前比较元素
        float sum;
        switch (cmp(a, b))//比较两者的指数值
        {
        case -1://多项式中PA中的结点的指数小
            ha = qa;
            qa = NextPos(ha);
            break;
        case 0://两者指数值相等
            sum = a.coef + b.coef;
            if (sum != 0)
            {
                //修改pa指向的该结点的系数值
                qa->data.coef = sum;
                //下一个
                ha = qa;
            }
            else
            {
                //删除结点
                DelFirst(&Pa, ha, &qa);
                FreeNode(&qa);
            }
            DelFirst(&Pb, hb, &qb);//也删除掉qb的结点
            FreeNode(&qb);//释放qb的空间
            //都往后移动一位
            qb = NextPos(hb);
            qa = NextPos(ha);
            break;
        case 1://多项式PB中的当前结点指数值小
            DelFirst(&Pb, hb, &qb);//把当前结点从PB中删除，并用qb指向当前结点用以插入
            InsFirst(&Pa, ha, qb);//插入在ha前
            qb = NextPos(hb);
            qa = NextPos(ha);
            break;
        }//switch
    }//while
    if (!ListEmpty(Pb))
        Append(&Pa, qb);//连接Pb中剩余结点
    FreeNode(&hb);//释放Pb的头结点
}//AddPolyn
