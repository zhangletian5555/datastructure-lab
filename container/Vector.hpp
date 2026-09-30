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
    Rank _size;
    int _capacity;
    T *_elem;
    void expand();
    void shrink();
    void copyFrom(T const *A, Rank lo, Rank hi);

public:
    Vector(int c = DEFAULT_CAPACITY, Rank s = 0, int v = 0)
    {
        _elem = new T[_capacity = c];
        for(int i=0; i<s; i++)
        {
            _size++;
            _elem[i] = v;
        }
    }

    Vector(T const *A, Rank lo, Rank hi)
    { copyFrom(A, lo, hi); }
    Vector(T const *A, Rank n)
    { copyFrom(A, 0, n); }

    Vector(Vector<T> *V, Rank lo, Rank hi)
    { copyFrom(V->_elem, lo, hi); }
    Vector(Vector<T> *V)
    { copyFrom(V->_elem, 0, V->_size); }

    ~Vector()
    { delete[] _elem; }

    T &operator[](Rank r)
    { return _elem[r]; }
    int size()
    { return _size; }
    bool empty()
    { return !_size; }
    void load(string filename);
    void save(string filename);
    void show();
    void push_back(T const &x);
    void pop_back();
    void insert(int p, T const &x);
    void erase(int p);
    void update(int p, T const &x);
    void reverse();
};

template <typename T>
void Vector<T>::copyFrom(T const *A, Rank lo, Rank hi)
{
    _capacity = (hi-lo) * 2;
    _elem = new T[_capacity];
    _size = 0;
    for(Rank i=lo; i<hi; i++)
    {
        _elem[i] = A[i];
        _size++;
    }
}

template <typename T>
void Vector<T>::expand()
{
    if(_size < _capacity) return;
    if(_capacity < DEFAULT_CAPACITY) _capacity = DEFAULT_CAPACITY;
    T *_oldElem = _elem;
    _elem = new T[_capacity <<= 1];
    for(int i=0; i<_size; i++)
        _elem[i]= _oldElem[i];
    delete[] _oldElem;
}

template <typename T>
void Vector<T>::shrink()
{
    if(_capacity < DEFAULT_CAPACITY<<1) return;
    if(_size<<2 > _capacity) return;
    T *_oldElem = _elem;
    _elem = new T[_capacity >>= 1];
    for(int i=0; i<_size; i++)
        _elem[i] = _oldElem[i];
    delete[] _oldElem;
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
    Rank r = --p;
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
    Rank r = --p;
    for(int i=r; i<_size-1; i++)
        _elem[i] = _elem[i+1];
    _size--;
    show();
}

template <typename T>
void Vector<T>::push_back(T const &x)
{
    insert(_size+1, x);
}

template <typename T>
void Vector<T>::pop_back()
{
    erase(_size+1);
}

template <typename T>
void Vector<T>::update(int p, T const &x)
{
    Rank r = --p;
    _elem[r] = x;
    show();
}

template <typename T>
void Vector<T>::reverse()
{
    T *_oldElem = _elem;
    _elem = new T[_capacity];
    for(int i=0; i<_size; i++)
        _elem[i] = _oldElem[_size-1-i];
    delete[] _oldElem;
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
        if (!(infile >> _size)) 
        { 
            cerr << "Broken file!\n"; 
            exit(EXIT_FAILURE); 
        }

        _capacity = _size << 1;
        _elem = new T[_capacity];
        for(int i=0; i<_size; i++)
            infile >> _elem[i];
        cout << "Loaded successfully from [" << filename << "]" << endl;
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
        infile.read(reinterpret_cast<char*>(&_size), sizeof(_size));

        _capacity = _size << 1;
        _elem = new T[_capacity];
        infile.read(reinterpret_cast<char*>(_elem), static_cast<streamsize>(_size * sizeof(T)));

        cout << "Loaded successfully from [" << filename << "]" << endl;
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
        cout << "Saved successfully to [" << filename << "]" << endl;
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

        cout << "Saved successfully to [" << filename << "]" << endl;
        onfile.close();
    }
    else
    {
        cout << "Unsupported format!" << endl;
        exit(EXIT_FAILURE);
    }
}