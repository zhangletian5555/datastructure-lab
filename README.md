# datastructure-lab

数据结构与算法实验的代码仓库。容器全部自行实现，不使用 STL 容器；`std::string` 和
`iostream` / `fstream` / `sstream` 正常使用。

## 运行环境

| 项目 | 版本 |
| --- | --- |
| 系统 | Ubuntu 26.04.1 LTS on WSL 2（内核 6.18.33.2-microsoft-standard-WSL2） |
| 编译器 | g++ 15.2.0，`-std=c++17` |
| 构建 | CMake 4.2.3 + GNU Make 4.4.1 |

Windows 下按手册附录 A 在 WSL 里装工具链：

```bash
sudo apt install -y build-essential gdb cmake ninja-build
```

构建与运行：

```bash
cmake -S . -B build
cmake --build build -j
cd build/bin && ./Vector_test
```

可执行文件和测试数据都放在 `build/bin/`，要在该目录下运行，否则 `load Vector_data.txt`
这类相对路径找不到文件。

## 目录结构

```text
datastructure-lab/
├── CMakeLists.txt
├── container/          # 容器，供后续实验复用
│   ├── Vector.hpp      # 动态顺序表
│   └── List.hpp        # 带哨兵结点的双向链表
├── src/
│   └── main.cpp
├── tests/
│   ├── ADT_test/               # 交互式测试驱动（CMake 目标）
│   │   ├── Vector_test.cpp     # Vector 的指令解释器
│   │   ├── List_test.cpp
│   │   ├── Vector_data.txt
│   │   └── List_data.txt
│   └── logic_test/             # 逻辑测试，手动运行，不接 CMake
│       ├── cli_logic_test.py       # 命令行层：喂指令序列，比对最终状态
│       ├── container_logic_test.py # 容器层：ASan + UBSan，逐用例独立进程
│       ├── vector_logic_test.cpp
│       ├── list_logic_test.cpp
│       └── README.md
├── report/             # 实验报告与用户使用手册（LaTeX）
└── build/              # 构建产物
```

## 指令说明

从标准输入每次读一条指令并执行，位置参数 `p` 从 1 开始编号。无法识别的指令提示
`Unrecognized operation!` 后继续等待输入。

数据文件按扩展名区分：`.txt` 第一行为元素个数、其后为空白分隔的元素；`.bin` 为
4 字节元素个数加上元素的原始字节。扩展名大小写不敏感，其他扩展名提示
`Unsupported format!` 后退出。

### Vector

`./Vector_test`

| 指令 | 参数 | 说明 |
| --- | --- | --- |
| `load` | `filename.ext` | 读文件初始化向量，输出 `Loaded successfully from [filename]` |
| `size` | — | 输出元素个数 |
| `show` | — | 一行内输出全部元素 |
| `push_back` | `x` | 尾部插入 `x` |
| `pop_back` | — | 删除尾部元素 |
| `insert` | `p x` | 在第 `p` 个位置插入 `x` |
| `erase` | `p` | 删除第 `p` 个位置的元素 |
| `update` | `p x` | 把第 `p` 个位置的元素改为 `x` |
| `save` | `filename.ext` | 保存到文件，输出 `Saved successfully to [filename]` |
| `exit` | — | 结束程序，输出 `End executing!` |

除 `load`、`size`、`save`、`exit` 外，其余指令执行后都会输出操作后的完整向量。

### List

`./List_test`

| 指令 | 参数 | 说明 |
| --- | --- | --- |
| `load` | `filename.ext` | 读文件，元素依次追加到链表尾部 |
| `show` | — | 一行内输出全部元素 |
| `insertAsFirst` | `e` | 头部插入 `e` |
| `insertAsLast` | `e` | 尾部插入 `e` |
| `size` | — | 输出元素个数 |
| `save` | `filename.ext` | 保存到文件 |
| `exit` | — | 结束程序，输出 `End executing!` |
