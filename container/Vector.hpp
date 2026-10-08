#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <filesystem>
#include <algorithm>
using namespace std;

typedef int Rank;
#define DEFAULT_CAPACITY 10

template <typename T>
class Vector
{
protected:
    // 向量内容标志
    Rank _size;
    int _capacity;
    T *_elem;
    // 内部操作函数
    void expand();
    void shrink();
    void copyFrom(T const *A, Rank lo, Rank hi);

public:
    // 构造和析构
    Vector(int c = DEFAULT_CAPACITY, Rank s = 0, int v = 0)
    {
        _elem = new T[_capacity = c];
        _size = s;
        for(int i=0; i<s; i++)
            _elem[i] = v;
    }

    Vector(T const *A, Rank lo, Rank hi)
    { copyFrom(A, lo, hi); }
    Vector(T const *A, Rank n)
    { copyFrom(A, 0, n); }

    Vector(Vector<T> const &V, Rank lo, Rank hi)
    { copyFrom(V._elem, lo, hi); }
    Vector(Vector<T> const &V)
    { copyFrom(V._elem, 0, V._size); }

    ~Vector()
    { delete[] _elem; }

    // 对外操作函数
    T &operator[](Rank r)             { return _elem[r]; }
    T const &operator[](Rank r) const { return _elem[r]; }
    Vector<T>& operator=(Vector<T> const&);
    int size()                          // 返回向量大小
    const { return _size; }
    bool empty()                        // 返回向量是否为空
    const { return !_size; }
    int capacity()                      // 返回向量容量
    const { return _capacity; }
    void reserve(int c);                // 扩容至指定容量
    void clear();                       // 清空向量
    void load(string filename);         // 加载文件
    void save(string filename);         // 保存文件
    void show();                        // 顺序展示向量
    void push_back(T const &x);         // 末元素压入
    void pop_back();                    // 末元素弹出
    void insert(int p, T const &x);     // 指定位置插入
    void erase(int p);                  // 指定位置删除
    void update(int p, T const &x);     // 指定位置修改
    void reverse();                     // 向量倒置
}; 

template <typename T>
void Vector<T>::copyFrom(T const *A, Rank lo, Rank hi)
{
    if(hi < lo) hi = lo;
    _capacity = (hi-lo) * 2;
    _elem = new T[_capacity];
    _size = 0;
    for(Rank i=lo; i<hi; i++)
    {
        _elem[i - lo] = A[i];
        _size++;
    }
}

template <typename T>
Vector<T> &Vector<T>::operator=(Vector<T> const &V)
{
    if(this == &V) return *this;
    if(_elem) delete[] _elem;
    copyFrom(V._elem, 0, V.size());
    return *this;
}

template <typename T>
void Vector<T>::expand()
{
    if(_size < _capacity) return;
    if(_capacity < DEFAULT_CAPACITY) _capacity = DEFAULT_CAPACITY;   // 扩容不小于默认
    T *_oldElem = _elem;
    _elem = new T[_capacity <<= 1];   // 扩容为两倍
    for(int i=0; i<_size; i++)
        _elem[i]= _oldElem[i];
    delete[] _oldElem;
}

template <typename T>
void Vector<T>::shrink()
{
    if(_capacity < DEFAULT_CAPACITY<<1) return;   // 缩容不小于默认一半
    if(_size<<2 > _capacity) return;   // 缩容后不小于一半
    T *_oldElem = _elem;
    _elem = new T[_capacity >>= 1];   //缩容为一半
    for(int i=0; i<_size; i++)
        _elem[i] = _oldElem[i];
    delete[] _oldElem;
}

template <typename T>
void Vector<T>::reserve(int c)
{
    if(c <= _capacity) return;   // 容量已经够用
    T *_oldElem = _elem;
    _elem = new T[_capacity = c];
    for(int i=0; i<_size; i++)
        _elem[i] = _oldElem[i];
    delete[] _oldElem;
}

template <typename T>
void Vector<T>::clear()
{
    _size = 0;
}

template <typename T>
void Vector<T>::show()
{
    for(int i=0; i<_size; i++)
        cout << _elem[i] << ' ';
    cout << '\n';
}

template <typename T>
void Vector<T>::insert(int p, T const &x)
{
    expand();
    if (p < 1 || p > _size + 1) { return; }
    Rank r = p - 1;
    for(int i=_size; i>r; i--) 
        _elem[i] = _elem[i-1];
    _elem[r] = x;
    _size++;
    show();
}

template <typename T>
void Vector<T>::erase(int p)
{
    shrink();
    if (p < 1 || p > _size) { return; }
    Rank r = p - 1;
    for(int i=r; i<_size-1; i++)
        _elem[i] = _elem[i+1];
    _size--;
    show();
}

template <typename T>
void Vector<T>::push_back(T const &x)
{
    expand(); 
    _elem[_size++] = x;
    show();
}

template <typename T>
void Vector<T>::pop_back()
{
    if (_size > 0) 
    { 
        --_size; shrink(); 
    }
    show();
}

template <typename T>
void Vector<T>::update(int p, T const &x)
{
    if (p < 1 || p > _size) { return; }
    Rank r = p - 1;
    _elem[r] = x;
    show();
}

template <typename T>
void Vector<T>::reverse()
{
    for (int i = 0; i < _size / 2; ++i)
    {
        T tmp = _elem[i];
        _elem[i] = _elem[_size - 1 - i];
        _elem[_size - 1 - i] = tmp;
    }
}

template <typename T>
void Vector<T>::load(string filename)
{
    filesystem::path p = filename;
    string ext = p.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), 
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if(ext == ".txt")
    {
        ifstream infile(filename, ios::in);
        if(!infile)
        {
            cerr << "Fail loading the file!\n";
            exit(EXIT_FAILURE);
        }
        int n = 0;
        if (!(infile >> n) || n < 0) 
        { 
            cerr << "Broken file!\n"; 
            return; 
        }
        _size = n;

        _capacity = _size << 1;
        delete[] _elem;
        _elem = new T[_capacity];
        for(int i=0; i<_size; i++)
            infile >> _elem[i];
        cout << "Loaded successfully from " << filename << " (text)!" << endl;
        infile.close();
    }
    else if(ext == ".bin")
    {
        ifstream infile(filename, ios::binary);
        if (!infile)
        {
            cerr << "Fail loading the file!\n";
            exit(EXIT_FAILURE);
        }
        int n = 0;
        infile.read(reinterpret_cast<char*>(&n), sizeof(n));
        if (n < 0) 
        { 
            cerr << "Broken file!\n"; 
            return; 
        }
        _size = n;

        _capacity = _size << 1;
        delete[] _elem;
        _elem = new T[_capacity];
        infile.read(reinterpret_cast<char*>(_elem), static_cast<streamsize>(_size * sizeof(T)));

        cout << "Loaded successfully from " << filename << " (binary)!" << endl;
        infile.close();
    }
    else
    {
        cout << "Unsupported format!" << endl;
        exit(EXIT_FAILURE);
    }
}

template <typename T>
void Vector<T>::save(string filename)
{
    filesystem::path p = filename;;
    string ext = p.extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), 
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if(ext == ".txt")
    {
        ofstream onfile(filename, ios::out);
        if(!onfile)
        {
            cerr << "Fail saving the file!\n";
            exit(EXIT_FAILURE);
        }
        onfile << _size << "\n";
        for(int i=0; i<_size; i++)
            onfile << _elem[i] << " ";
        onfile << "\n";
        cout << "Saved successfully to " << filename << " (text)!" << endl;
        onfile.close();
    }
    else if(ext == ".bin")
    {
        ofstream onfile(filename, ios::out | ios::binary);
        if(!onfile)
        {
            cerr << "Fail saving the file!\n";
            exit(EXIT_FAILURE);
        }

        onfile.write(reinterpret_cast<const char*>(&_size), sizeof(_size));
        onfile.write(reinterpret_cast<const char*>(_elem), static_cast<streamsize>(_size * sizeof(T)));

        cout << "Saved successfully to " << filename << " (binary)!" << endl;
        onfile.close();
    }
    else
    {
        cout << "Unsupported format!" << endl;
        exit(EXIT_FAILURE);
    }
}