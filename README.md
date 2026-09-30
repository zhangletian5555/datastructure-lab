# datastructure-lab
## 项目说明
本项目全程在linux环境下开发，使用cpp17标准；全程不使用cpp自带的数据结构，采用了文件读写库的引入。

## 文件结构
datastructure-lab/
├── CMakeLists.txt                  # 根 CMake
├── README.md
├── .gitignore
│
├── container/                      # 容器/数据结构
│   ├── Vector.hpp
│   └── List.hpp
│
├── src/                            # 源文件 + 主程序
│   ├── 
│   └── main.cpp                    # 主程序入口
│
├── tests/                          # 测试代码
│   ├── ADT_test/
│   │   ├── Vector_test.cpp
│   │   ├── List_test.cpp
│   │   ├── Vector_data.txt
│   │   └── List_data.txt
│
└── build/                          # 编译产物

## 流程记录
### 第一次lab
完成容器Vector、List并测试，搭建了CMake的基本框架。
目前支持的文件读写是仅以.txt为后缀的文本文件和以.bin为后缀的二进制文件（考虑windows文件命名的特性，大小写不敏感）。
经简单的测试容器功能展现正常。