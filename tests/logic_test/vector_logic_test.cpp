// 实验一 容器逻辑测试（直接调用 API，不经过命令行）
//
// 每个用例是一个独立进程：./container_logic_test <case_name>
// 正常结束打印 OK；断言失败打印 FAIL: ...；崩溃则由驱动脚本按退出码判定。
//
// 用法：python3 container_logic_test.py        （见同目录驱动脚本）

#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>
#include "Vector.hpp"

// List.hpp 与 Vector.hpp 都会 typedef Rank / define DEFAULT_CAPACITY，
// 两个头无法同时包含，所以 List 的用例放在单独的翻译单元里。
#define ONLY_VECTOR 1

using namespace std;

static int failures = 0;

#define CHECK(cond, msg)                                    \
    do {                                                    \
        if (!(cond)) {                                      \
            cout << "FAIL: " << msg << endl;                \
            ++failures;                                     \
        }                                                   \
    } while (0)

typedef Vector<int> VI;

// ---------- Vector ----------

static VI make(int n)  // 造一个 1..n 的向量
{
    VI v;
    for (int i = 1; i <= n; ++i) v.push_back(i);
    return v;
}

static bool same(VI &v, const std::vector<int> &want)
{
    if ((size_t)v.size() != want.size()) return false;
    for (size_t i = 0; i < want.size(); ++i)
        if (v[i] != want[i]) return false;
    return true;
}

void vec_copy_ctor()
{
    int a[3] = {1, 2, 3};
    VI src(a, 3);
    VI copy(src);                 // 拷贝构造
    CHECK(same(copy, {1, 2, 3}), "拷贝构造内容不一致");
    copy.update(1, 99);           // 改副本，原对象不能受影响
    CHECK(same(src, {1, 2, 3}), "改副本影响了原对象（浅拷贝）");
    cout << "OK" << endl;
}

void vec_copy_assign()
{
    int a[3] = {1, 2, 3};
    VI src(a, 3), dst;
    dst = src;
    CHECK(same(dst, {1, 2, 3}), "拷贝赋值内容不一致");
    dst.update(1, 99);
    CHECK(same(src, {1, 2, 3}), "改副本影响了原对象（浅拷贝）");
    cout << "OK" << endl;
}

void vec_self_assign()
{
    int a[3] = {1, 2, 3};
    VI v(a, 3);
    v = v;
    CHECK(same(v, {1, 2, 3}), "自赋值破坏了内容");
    cout << "OK" << endl;
}

void vec_copyFrom_range()
{
    int a[6] = {10, 11, 12, 13, 14, 15};
    VI v(a, 2, 5);                // 取下标的 [2,5)
    CHECK(same(v, {12, 13, 14}), "copyFrom(lo,hi) 取值错误");
    cout << "OK" << endl;
}

void vec_copyFrom_invalid_range()
{
    int a[3] = {1, 2, 3};
    VI v(a, 3);
    VI bad(v, 3, 1);              // hi < lo：应当是空向量且不崩
    CHECK(bad.size() == 0, "hi<lo 时应该是空向量");
    cout << "OK" << endl;
}

// 把对象建在一块预先填成 0xAA 的内存上，用来暴露"某个成员没被初始化"。
// 只要构造函数真正写了 _size/_capacity/_elem，这个用例就会通过。
void vec_copyFrom_invalid_dirty()
{
    int a[3] = {1, 2, 3};
    VI src(a, 3);

    unsigned char buf[sizeof(VI)];
    for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = 0xAA;
    VI *p = new (buf) VI(src, 3, 1);      // hi < lo

    long long sz = p->size();
    cout << "  size() 读到 " << sz;
    if (sz != 0) cout << "  <- 未初始化（原始字节 0xAAAAAAAA...）";
    cout << endl;
    CHECK(sz == 0, "copyFrom(hi<lo) 提前 return，_size/_capacity/_elem 没被初始化");
    // 故意不析构：_elem 是 0xAAAA...，析构会 delete[] 一个非法指针
    cout << "OK" << endl;
}

void vec_assign_different_sizes()
{
    VI big = make(100), small = make(2);
    small = big;
    CHECK(small.size() == 100 && small[99] == 100, "小对象 = 大对象 失败");
    big = make(3);
    CHECK(big.size() == 3 && big[2] == 3, "大对象 = 小对象 失败");
    cout << "OK" << endl;
}

void vec_expand_integrity()
{
    const int N = 1000;
    VI v;
    for (int i = 1; i <= N; ++i) v.push_back(i);
    CHECK(v.size() == N, "push_back 后 size 不对");
    for (int i = 0; i < N; ++i)
        if (v[i] != i + 1) { CHECK(false, "扩容后元素错位"); break; }
    cout << "OK" << endl;
}

void vec_shrink_integrity()
{
    VI v = make(400);
    for (int i = 0; i < 390; ++i) v.pop_back();
    CHECK(v.size() == 10, "pop_back 后 size 不对");
    bool ok = true;
    for (int i = 0; i < 10; ++i) if (v[i] != i + 1) ok = false;
    CHECK(ok, "缩容后元素错位");
    v.push_back(999);             // 缩容后继续写
    CHECK(v.size() == 11 && v[10] == 999, "缩容后写入失败");
    cout << "OK" << endl;
}

void vec_insert_erase()
{
    VI v = make(5);               // 1 2 3 4 5
    v.insert(1, 0);               // 头插
    CHECK(same(v, {0, 1, 2, 3, 4, 5}), "insert(1) 不是头插");
    v.insert(7, 6);               // 尾插（size+1）
    CHECK(same(v, {0, 1, 2, 3, 4, 5, 6}), "insert(size+1) 不是尾插");
    v.erase(1);
    CHECK(same(v, {1, 2, 3, 4, 5, 6}), "erase(1) 没删头");
    v.erase(v.size());
    CHECK(same(v, {1, 2, 3, 4, 5}), "erase(size) 没删尾");
    v.erase(3);
    CHECK(same(v, {1, 2, 4, 5}), "erase(3) 中间删除错误");
    cout << "OK" << endl;
}

void vec_out_of_range_noop()
{
    VI v = make(3);
    v.insert(0, 9);
    v.insert(99, 9);
    v.erase(0);
    v.erase(99);
    v.update(0, 9);
    v.update(99, 9);
    CHECK(same(v, {1, 2, 3}), "越界位置参数改变了向量");
    cout << "OK" << endl;
}

void vec_reverse()
{
    VI v = make(4);
    v.reverse();
    CHECK(same(v, {4, 3, 2, 1}), "偶数长度 reverse 错误");
    VI w = make(5);
    w.reverse();
    CHECK(same(w, {5, 4, 3, 2, 1}), "奇数长度 reverse 错误");
    cout << "OK" << endl;
}

void vec_const_index()
{
    int a[3] = {1, 2, 3};
    const VI cv(a, 3);
    CHECK(cv[0] == 1 && cv[2] == 3, "const 对象下标访问失败");
    cout << "OK" << endl;
}

void vec_empty_ops()
{
    VI v;
    CHECK(v.size() == 0 && v.empty(), "默认构造不是空的");
    v.pop_back();
    v.erase(1);
    v.update(1, 9);
    CHECK(v.size() == 0, "空向量上的操作改变了 size");
    v.push_back(7);
    CHECK(v.size() == 1 && v[0] == 7, "空向量 push_back 后内容错误");
    cout << "OK" << endl;
}

void vec_load_zero_then_push()
{
    // _capacity 会被设成 0，检查后续 push_back 是否还能扩容
    VI v;
    v.push_back(1);
    CHECK(v.size() == 1, "sanity");
    cout << "OK" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) { cout << "需要用例名" << endl; return 2; }
    string c = argv[1];

    if (c == "vec_copy_ctor")                 vec_copy_ctor();
    else if (c == "vec_copy_assign")          vec_copy_assign();
    else if (c == "vec_self_assign")          vec_self_assign();
    else if (c == "vec_copyFrom_range")       vec_copyFrom_range();
    else if (c == "vec_copyFrom_invalid")     vec_copyFrom_invalid_range();
    else if (c == "vec_copyFrom_invalid_dirty") vec_copyFrom_invalid_dirty();
    else if (c == "vec_assign_different_sizes") vec_assign_different_sizes();
    else if (c == "vec_expand_integrity")     vec_expand_integrity();
    else if (c == "vec_shrink_integrity")     vec_shrink_integrity();
    else if (c == "vec_insert_erase")         vec_insert_erase();
    else if (c == "vec_out_of_range_noop")    vec_out_of_range_noop();
    else if (c == "vec_reverse")              vec_reverse();
    else if (c == "vec_const_index")          vec_const_index();
    else if (c == "vec_empty_ops")            vec_empty_ops();
    else if (c == "vec_load_zero_then_push")  vec_load_zero_then_push();
    else { cout << "未知用例 " << c << endl; return 2; }

    return failures ? 1 : 0;
}
