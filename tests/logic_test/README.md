# 实验一 逻辑测试

两个测试脚本都不做文本比对，只看逻辑结果：指令执行完向量/链表里到底是什么。

```bash
cd tests/logic_test

# 1) 命令行层：把指令序列喂给真实的 Vector_test / List_test
python3 cli_logic_test.py ../../build/bin

# 2) 容器层：直接调 API，带 ASan + UBSan，每个用例独立进程
python3 container_logic_test.py ../..
```

## cli_logic_test.py

在你自己的 `Vector_data.txt` / `List_data.txt` 之外，还覆盖：

- 手册 2.4 节的样例指令序列，逐条比对元素序列
- 空向量上的各种操作、空表上的各种操作
- 头插 / 尾插 / 删头 / 删尾 / 中间增删
- 越界位置参数（`0`、`size+1`、`1000000`）
- 连续 `push_back` 300 个（跨多次倍增）后内容是否错位
- 删到只剩 10 个触发缩容后，数据是否还在
- `.txt` / `.bin` 往返、txt↔bin 交叉、扩展名大小写
- 10 万元素的 `.txt` / `.bin` 载入并另存，逐元素比对
- 不存在的文件、不支持的扩展名、空文件、负数元素个数、截断的文件

判定方式：在指令序列末尾追加 `show` + `size`，取最后一次的元素序列和个数作为最终状态。

## container_logic_test.py

用 `-fsanitize=address,undefined` 编译，检查 CLI 覆盖不到的部分：

- `Vector` 拷贝构造 / 拷贝赋值是深拷贝还是浅拷贝、自赋值
- `copyFrom` 的区间取值、`hi < lo` 的边界
- `expand` / `shrink` 之后元素有没有错位
- `const` 对象能不能下标访问
- `List` 是否真的是循环链表（正向、反向各走 `size+2` 步回到起点）
- `insertBefore` / `insertAfter` 落在哨兵附近的语义
- `remove` 传入哨兵结点是否被拒绝
- 拷贝构造 / 拷贝赋值 / 自赋值
- `List(L, r, n)` 从中间开始拷
- 2 万个结点的插入与删除
- 两个头文件同时包含、重复包含

每个用例都是独立进程，所以崩溃、越界、泄漏都能单独定位，不会互相影响。
