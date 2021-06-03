#include "myshared_ptr.h"

int main()
{
    int *a = new int(2);
    smart_ptr<int> p(a);
    smart_ptr<int> pp(p);
    smart_ptr<int> ppp(p);
    std::cout<<pp.use_count()<<std::endl;
    return 0;
}