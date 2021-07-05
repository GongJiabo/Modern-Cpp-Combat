#include <iostream>

using namespace std;

// Can copy and move
class A
{
public:
    A() { cout << "Create A\n"; }
    ~A() { cout << "Destory A\n"; }
    A(const A&) { cout << "Copy A\n"; }
    A(A&&) { cout << "Move A\n"; }
};

A getA_unnamed()
{
    A a;
    return a;
    // return A();
}

int main()
{
    auto a = getA_unnamed();
    return 0;
}