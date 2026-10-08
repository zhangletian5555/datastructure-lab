#include <iostream>
#include "List.hpp"
using namespace std;

template <typename T>
void execute(const string &op, List<T> &L)
{
    if(op=="load")
    {
        string filename;
        cin >> filename;
        L.load(filename);
    }
    else if(op=="show")
    { L.show(); }
    else if(op=="push_back")
    {
        T e = 0;
        cin >> e;
        L.push_back(e);
        L.show();
    }
    else if(op=="push_front")
    {
        T e = 0;
        cin >> e;
        L.push_front(e);
        L.show();
    }
    else if(op=="size")
    { cout << L.size() << endl; }
    else if(op=="save")
    {
        string filename;
        cin >> filename;
        L.save(filename);
    }
    else if(op=="exit")
    { 
        cout << "End executing!" << endl;
        exit(0); 
    }
    else
        cerr << "Unrecognized operation!" << endl;
}

int main()
{
    List<int> L;
    string op;
    while (cin >> op) execute(op, L);

    return 0;
}