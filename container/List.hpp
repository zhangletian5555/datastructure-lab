#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <filesystem>
#include <algorithm>
using namespace std;

typedef int Rank;
#define ListNodePosi(T) ListNode<T>*

template <typename T>
struct ListNode
{
    T data;
    ListNodePosi(T) pred;
    ListNodePosi(T) succ;

    ListNode() {}
    ListNode(T const &e, ListNodePosi(T) p = nullptr, ListNodePosi(T) s = nullptr)
        : data(e), pred(p), succ(s) {}
    
    ListNodePosi(T) insertAsPred(T const &e);
    ListNodePosi(T) insertAsSucc(T const &e);
};

template <typename T>
ListNodePosi(T) ListNode<T>::insertAsPred(T const &e)
{
    ListNodePosi(T) x = new ListNode<T>(e, pred, this);
    pred->succ = x;
    pred = x;
    return x;
}
template <typename T>
ListNodePosi(T) ListNode<T>::insertAsSucc(T const &e)
{
    ListNodePosi(T) x = new ListNode<T>(e, this, succ);
    succ->pred= x;
    succ = x;
    return x;
}

template <typename T>
class List
{
private:
    int _size;
    ListNodePosi(T) header = nullptr;
    ListNodePosi(T) trailer = nullptr;

protected:
    void init();
    void clear();
    void copyNodes(ListNodePosi(T) p, int n);

public:
    List() { init(); }
    List(List<T> const &L);
    List(List<T> const &L, Rank r, int n);
    List(ListNodePosi(T) p, int n);
    ~List();

    Rank size() const { return _size; }
    bool empty() const { return !_size; }
    T &operator[](Rank r);
    ListNodePosi(T) first() { return header->succ; }
    ListNodePosi(T) last() { return trailer->pred; }
    bool valid(ListNodePosi(T) p) { return p && (trailer != p) && (header != p); }

    void remove(ListNodePosi(T) p);
    void insertAsFirst(T const &e);
    void insertAsLast(T const &e);
    void insertBefore(ListNodePosi(T) p, T const &e);
    void insertAfter(ListNodePosi(T) p, T const &e);
    void load(string filename);
    void save(string filename);
    void show();
};

template <typename T>
void List<T>::init()
{
    header = new ListNode<T>;
    trailer = new ListNode<T>;
    header->pred = nullptr; header->succ = trailer;
    trailer->pred = header; trailer->succ = nullptr;
    _size = 0;
}

template <typename T>
T &List<T>::operator[](Rank r)
{
    ListNodePosi(T) p = first();
    while(r > 0)
    {
        p = p->succ;
        --r;
    }
    return p->data;
}

template <typename T>
void List<T>::insertAsFirst(T const &e)
{
    header->insertAsSucc(e);
    ++_size;
}
template <typename T>
void List<T>::insertAsLast(T const &e)
{
    trailer->insertAsPred(e);
    ++_size;
}
template <typename T>
void List<T>::insertBefore(ListNodePosi(T) p, T const &e)
{
    p->insertAsPred(e);
    ++_size;
}
template <typename T>
void List<T>::insertAfter(ListNodePosi(T) p, T const &e)
{
    p->insertAsSucc(e);
    ++_size;
}

template <typename T>
void List<T>::copyNodes(ListNodePosi(T) p, int n)
{
    init();
    while(n>0)
    {
        insertAsLast(p->data);
        p = p->succ;
        --n;
    }
}

template <typename T>
List<T>::List(List<T> const &L)
{ copyNodes(L.first(), L._size); }
template <typename T>
List<T>::List(List<T> const &L, Rank r, int n)
{ copyNodes(L[r], n); }
template <typename T>
List<T>::List(ListNodePosi(T) p, int n)
{ copyNodes(p, n); }

template <typename T>
void List<T>::remove(ListNodePosi(T) p)
{
    // T e = p->data;
    p->pred->succ = p->succ;
    p->succ->pred = p->pred;
    delete p;
    --_size;
}

template <typename T>
void List<T>::clear()
{
    while(_size > 0)
    { remove(header->succ); }
}

template <typename T>
List<T>::~List()
{
    clear();
    delete header;
    delete trailer;
}

template <typename T>
void List<T>::load(string filename)
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
        if (!(infile >> n)) 
        { 
            cerr << "Broken file!\n"; 
            exit(EXIT_FAILURE); 
        }

        init();
        while(n > 0)
        {
            T next_e = 0;
            infile >> next_e;
            insertAsLast(next_e);
            --n;
        }
        cout << "Loaded successfully from [" << filename << "]" << endl;
        infile.close();
    }
    else if(ext ==".bin")
    {
        ifstream infile(filename, ios::binary);
        if(!infile)
        {
            cerr << "Fail loading the file!\n";
            exit(EXIT_FAILURE);
        }

        init();
        int n = 0;
        infile.read(reinterpret_cast<char*>(&n), sizeof(n));
        while(n > 0)
        {
            T next_e = 0;
            infile.read(reinterpret_cast<char*>(&next_e), sizeof(next_e));
            insertAsLast(next_e);
            --n;
        }
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
void List<T>::save(string filename)
{
    filesystem::path p = filename;
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
        ListNodePosi(T) p = first();
        for(int i=0; i<_size; i++)
        {
            onfile << p->data << " ";
            p = p->succ;
        }
        cout << "Saved successfully to [" << filename << "]" << endl;
        onfile.close();
    }
    else if(ext ==".bin")
    {
        ofstream onfile(filename, ios::out | ios::binary);
        if(!onfile)
        {
            cerr << "Fail saving the file!\n";
            exit(EXIT_FAILURE);
        }
        onfile.write(reinterpret_cast<const char*>(&_size), sizeof(_size));
        ListNodePosi(T) p = first();
        for(int i=0; i<_size; i++)
        {
            onfile.write(reinterpret_cast<const char*>(&p->data), sizeof(T));
            p = p->succ;
        }
        cout << "Saved successfully to [" << filename << "]" << endl;
        onfile.close();
    }
    else
    {
        cout << "Unsupported format!" << endl;
        exit(EXIT_FAILURE);
    }
}

template <typename T>
void List<T>::show()
{
    ListNodePosi(T) p = first();
    for(int i=0; i<_size; i++)
    {
        cout << p->data << " ";
        p = p->succ;
    }
    cout << "\n";
}