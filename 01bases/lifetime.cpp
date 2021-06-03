#include<iostream>

class shapre
{
public:
    virtual ~shapre(){}
};

class circle : public shapre
{
public:
    circle() { puts("circle()"); }
    ~circle() { puts("~circe()"); }
};

class triangle : public shapre
{
public:
    triangle() { puts("triangle()"); }
    ~triangle() { puts("~triangle()"); }
};

class result
{
public:
    result() { puts("result()"); }
    ~result() { puts("~result()"); }
};

result process_shape(const shapre& shape1, const shapre& shape2)
{
    puts("process_shape()");
    return result();
}

int main()
{
    puts("main");
    //process_shape(circle(), triangle());
    //result&& r = process_shape(circle(), triangle());
    result&& r = std::move(process_shape(circle(), triangle()));
    puts("something else");
}
