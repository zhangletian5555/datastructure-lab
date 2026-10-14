# datastructure-lab

## 运行环境

| 项目 | 版本 |
| --- | --- |
| 系统 | Ubuntu 26.04.1 LTS on WSL 2（内核 6.18.33.2-microsoft-standard-WSL2） |
| 编译器 | g++ 15.2.0，`-std=c++17` |
| 构建 | CMake 4.2.3 + GNU Make 4.4.1 |

## 目录结构

```text
datastructure-lab/
├── .gitignore
├── CMakeLists.txt
├── container/                  # 容器实现，供各实验复用
│   ├── Vector.hpp              # 动态顺序表
│   └── List.hpp                # 带哨兵结点的双向链表
├── data/                       # 实验输入数据
│   ├── flight-data.csv         # 航班数据
│   └── id2name.csv             # 机场内部编号与拼音名的映射
├── src/
│   ├── main.cpp                # 主程序
│   └── dataProcess.hpp         # 数据读取、统计、排序、查询
├── tests/
│   ├── ADT_test/               # 交互式测试驱动
│   │   ├── Vector_test.cpp     # Vector 的指令解释器
│   │   ├── List_test.cpp       # List 的指令解释器
│   │   ├── Vector_data.txt
│   │   └── List_data.txt
│   └── logic_test/             # 逻辑测试，手动运行
│       ├── cli_logic_test.py       # 命令行层：喂指令序列给真实测试驱动
│       ├── container_logic_test.py # 容器层：直接调 API，带 ASan + UBSan
│       ├── vector_logic_test.cpp
│       ├── list_logic_test.cpp
│       └── README.md
├── report/                     # 实验报告与用户使用手册
├── tmp/                        # 临时草稿
└── build/                      # 构建产物
```

## 指令说明

从标准输入每次读一条指令并执行，位置参数 `p` 从 1 开始编号。

数据文件按扩展名区分：`.txt` 第一行为元素个数、其后为空白分隔的元素；`.bin` 为
4 字节元素个数加上元素的原始字节。

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

### List

`./List_test`

| 指令 | 参数 | 说明 |
| --- | --- | --- |
| `load` | `filename.ext` | 读文件，元素依次追加到链表尾部 |
| `show` | — | 一行内输出全部元素 |
| `push_front` | `e` | 头部插入 `e` |
| `push_back` | `e` | 尾部插入 `e` |
| `size` | — | 输出元素个数 |
| `save` | `filename.ext` | 保存到文件 |
| `exit` | — | 结束程序，输出 `End executing!` |
