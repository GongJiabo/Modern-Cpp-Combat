#include <iostream>
#include <vector>

using namespace std;

class Obj1
{
public:
    Obj1()
    {
        cout << "Obj1()\n";
    }
    Obj1(const Obj1&)
    {
        cout << "Obj1(const Obj1&)\n";
    }
    Obj1(Obj1&&)
    {
        cout << "Obj1(Obj1&&)\n";
    }
};

class Obj2
{
public:
    Obj2()
    {
        cout << "Obj2()\n";
    }
    Obj2(const Obj2&)
    {
        cout << "Obj2(const Obj2&)\n";
    }
    Obj2(Obj2&&) noexcept
    {
        cout << "Obj2(Obj2&&) noexcept\n";
    }
};

int main()
{
    vector<Obj1> v1;
    v1.reserve(2);
    v1.emplace_back();
    v1.emplace_back();
    cout << v1.capacity()<<endl;
    v1.emplace_back();  
    //v1.push_back(Obj1());
    cout << v1.capacity()<<endl;

    cout << endl;

    vector<Obj2> v2;
    v2.reserve(2);
    v2.emplace_back();
    v2.emplace_back();
    cout << v2.capacity()<<endl;
    v2.emplace_back();
    cout << v2.capacity()<<endl;
}