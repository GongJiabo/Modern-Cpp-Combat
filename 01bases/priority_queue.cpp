#include <functional>   // std::less/greater/hash
#include <iostream>     // std::cout/endl
#include <memory>       // std::pair
#include <queue>        // std::priority_queue
#include <vector>       // std::vector
#include <set>
#include "output_container.h"

using namespace std;

int main()
{
    // greater -- 升序排列, 小顶堆
    priority_queue< pair<int,int>, 
        vector<pair<int,int>>, greater<pair<int,int>> > q;
    // priority_queue< pair<int,int> > q;
    q.push({1, 1});
    q.push({2, 2});
    q.push({0, 3});
    q.push({4, 7});
    q.push({9, 4});

    while(!q.empty())
    {
        cout << q.top() << endl;
        q.pop();
    }

    multiset<int, greater<int>> ms{1,1,2,2,5,3,4,3,4,7,2};
    for(auto it = ms.begin(); it!=ms.end(); ++it)
        cout << *it <<"    ";
    return 0;
}