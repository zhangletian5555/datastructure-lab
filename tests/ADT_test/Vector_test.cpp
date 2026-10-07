#include <iostream>
#include "Vector.hpp"
using namespace std;

template <typename T>
void execute(const string &op, Vector<T> &V)
{
    if(op=="load")
    {
        string filename;
        cin >> filename;
        V.load(filename);
    }
    else if(op=="show")
    { V.show(); }
    else if(op=="push_back")
    {
        T x;
        cin >> x;
        V.push_back(x);
    }
    else if(op=="pop_back")
    { V.pop_back(); }
    else if(op=="insert")
    {
        T x; int p;
        cin >> p >> x;
        V.insert(p, x);
    }
    else if(op=="erase")
    {
        int p;
        cin >> p;
        V.erase(p);
    }
    else if(op=="update")
    {
        T x; int p;
        cin >> p >> x;
        V.update(p, x);
    }
    else if(op=="size")
    { cout << V.size() << endl; }
    else if(op=="save")
    {
        string filename;
        cin >> filename;
        V.save(filename);
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
    Vector<int> V;
    string op;
    while (cin >> op) execute(op, V);

    return 0;
}