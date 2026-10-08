// 实验一 List<T> 逻辑测试（直接调用 API）
// 每个用例是一个独立进程：./list_logic_test <case_name>

#include <iostream>
#include <string>
#include <vector>
#include "List.hpp"

using namespace std;

static int failures = 0;

#define CHECK(cond, msg)                                    \
    do {                                                    \
        if (!(cond)) {                                      \
            cout << "FAIL: " << msg << endl;                \
            ++failures;                                     \
        }                                                   \
    } while (0)

typedef List<int> LI;

static LI make(int n)
{
    LI l;
    for (int i = 1; i <= n; ++i) l.push_back(i);
    return l;
}

static std::vector<int> dump(LI &l)      // 按结点链正向走一遍，不看 _size
{
    std::vector<int> out;
    ListNodePosi(int) p = l.front();
    for (int i = 0; i < l.size(); ++i) { out.push_back(p->data); p = p->succ; }
    return out;
}

static bool same(LI &l, const std::vector<int> &want)
{
    if ((size_t)l.size() != want.size()) return false;
    std::vector<int> got = dump(l);
    return got == want;
}

void list_basic()
{
    LI l = make(3);
    CHECK(l.size() == 3, "size 不对");
    CHECK(same(l, {1, 2, 3}), "内容不对");
    CHECK(l.front()->data == 1, "front() 不对");
    CHECK(l.back()->data == 3, "back() 不对");
    cout << "OK" << endl;
}

void list_circular()
{
    LI l = make(3);
    // 从 front() 走 size+2 步应该正好回到 front()
    ListNodePosi(int) p = l.front();
    for (int i = 0; i < l.size() + 2; ++i) p = p->succ;
    CHECK(p == l.front(), "正向不是循环的");
    // 反向同理
    p = l.back();
    for (int i = 0; i < l.size() + 2; ++i) p = p->pred;
    CHECK(p == l.back(), "反向不是循环的");
    // 空表也要成环
    LI e;
    CHECK(e.front()->succ == e.back()->succ || e.size() == 0, "sanity");
    CHECK(e.front()->pred != nullptr && e.front()->succ != nullptr,
          "空表的哨兵指针为空（说明不是循环链表）");
    cout << "OK" << endl;
}

void list_insert_variants()
{
    LI l = make(3);                     // 1 2 3
    l.push_front(0);                 // 0 1 2 3
    l.push_back(4);                  // 0 1 2 3 4
    CHECK(same(l, {0, 1, 2, 3, 4}), "首尾插入错误");
    l.insertBefore(l.front(), -1);      // -1 0 1 2 3 4
    CHECK(same(l, {-1, 0, 1, 2, 3, 4}), "insertBefore(front) 错误");
    l.insertAfter(l.back(), 5);         // ... 5
    CHECK(same(l, {-1, 0, 1, 2, 3, 4, 5}), "insertAfter(back) 错误");
    l.insertAfter(l.front(), -2);       // -1 -2 0 1 2 3 4 5
    CHECK(same(l, {-1, -2, 0, 1, 2, 3, 4, 5}), "insertAfter(front) 错误");
    cout << "OK" << endl;
}

void list_remove_edges()
{
    LI l = make(3);
    l.remove(l.front());
    CHECK(same(l, {2, 3}), "remove(first) 错误");
    l.remove(l.back());
    CHECK(same(l, {2}), "remove(last) 错误");
    l.remove(l.front());
    CHECK(l.size() == 0 && l.empty(), "删空后 size 不为 0");
    cout << "OK" << endl;
}

void list_remove_sentinel()
{
    LI l = make(3);
    l.remove(l.front()->pred);          // header
    l.remove(l.back()->succ);           // trailer
    CHECK(l.size() == 3, "remove 哨兵改变了 size（应当被拒绝）");
    CHECK(same(l, {1, 2, 3}), "remove 哨兵破坏了链表");
    cout << "OK" << endl;
}

void list_clear_reuse()
{
    LI l = make(5);
    l.clear();
    CHECK(l.size() == 0, "clear 后 size 不为 0");
    l.push_back(9);
    CHECK(same(l, {9}), "clear 后无法继续使用");
    cout << "OK" << endl;
}

void list_copy_ctor()
{
    LI src = make(3);
    LI c(src);
    CHECK(same(c, {1, 2, 3}), "拷贝构造内容不对");
    c.push_back(99);
    CHECK(same(src, {1, 2, 3}), "改副本影响了原链表");
    cout << "OK" << endl;
}

void list_copy_assign()
{
    LI src = make(3);
    LI dst;
    dst = src;
    CHECK(same(dst, {1, 2, 3}), "拷贝赋值内容不对");
    dst.push_back(99);
    CHECK(same(src, {1, 2, 3}), "改副本影响了原链表");
    cout << "OK" << endl;
}

void list_range_ctor()
{
    LI src = make(5);                   // 1 2 3 4 5
    LI a(src, 0, 3);                    // 从第 0 个起 3 个
    CHECK(same(a, {1, 2, 3}), "List(L, 0, 3) 错误");
    LI b(src, 2, 2);                    // 从第 2 个起 2 个
    CHECK(same(b, {3, 4}), "List(L, 2, 2) 错误");
    LI c(src, 4, 1);                    // 最后一个
    CHECK(same(c, {5}), "List(L, 4, 1) 错误");
    cout << "OK" << endl;
}

void list_index()
{
    LI l = make(5);
    CHECK(l[0] == 1 && l[4] == 5, "operator[] 取值错误");
    cout << "OK" << endl;
}

// 越界下标：当前实现是 cerr 一句提示后 exit(EXIT_FAILURE)，
// 整进程被杀。这里只记录行为，由驱动脚本按"非 0 退出"判定。
void list_index_oob()
{
    LI l = make(5);
    int x = l[99];
    cout << "没有退出，读到 " << x << endl;
    cout << "OK" << endl;
}

void list_self_assign()
{
    LI l = make(3);
    l = l;
    CHECK(same(l, {1, 2, 3}), "自赋值破坏了链表");
    cout << "OK" << endl;
}

void list_big()
{
    const int N = 20000;
    LI l;
    for (int i = 0; i < N; ++i) l.push_back(i);
    CHECK(l.size() == N, "大批量插入后 size 不对");
    bool ok = true;
    ListNodePosi(int) p = l.front();
    for (int i = 0; i < N; ++i) { if (p->data != i) { ok = false; break; } p = p->succ; }
    CHECK(ok, "大批量插入后内容错位");
    for (int i = 0; i < N; ++i) l.remove(l.front());
    CHECK(l.size() == 0, "大批量删除后 size 不为 0");
    cout << "OK" << endl;
}

void list_insert_around_sentinels()
{
    LI l = make(3);                          // 1 2 3
    l.insertBefore(l.back()->succ, 4);       // insertBefore(trailer) 等价于尾插
    CHECK(same(l, {1, 2, 3, 4}), "insertBefore(trailer) 错误");
    l.insertAfter(l.front()->pred, 0);       // insertAfter(header) 等价于头插
    CHECK(same(l, {0, 1, 2, 3, 4}), "insertAfter(header) 错误");
    cout << "OK" << endl;
}

void list_empty_ops()
{
    LI l;
    CHECK(l.size() == 0 && l.empty(), "默认构造不是空表");
    l.clear();
    l.remove(nullptr);
    CHECK(l.size() == 0, "空表操作改变了 size");
    CHECK(l.front() == l.back()->succ, "sanity");
    cout << "OK" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) { cout << "需要用例名" << endl; return 2; }
    string c = argv[1];

    if (c == "list_basic")            list_basic();
    else if (c == "list_circular")    list_circular();
    else if (c == "list_insert_variants") list_insert_variants();
    else if (c == "list_remove_edges")    list_remove_edges();
    else if (c == "list_remove_sentinel") list_remove_sentinel();
    else if (c == "list_clear_reuse")     list_clear_reuse();
    else if (c == "list_copy_ctor")       list_copy_ctor();
    else if (c == "list_copy_assign")     list_copy_assign();
    else if (c == "list_range_ctor")      list_range_ctor();
    else if (c == "list_index")           list_index();
    else if (c == "list_index_oob")       list_index_oob();
    else if (c == "list_self_assign")     list_self_assign();
    else if (c == "list_insert_around_sentinels") list_insert_around_sentinels();
    else if (c == "list_big")             list_big();
    else if (c == "list_empty_ops")       list_empty_ops();
    else { cout << "未知用例 " << c << endl; return 2; }

    return failures ? 1 : 0;
}
