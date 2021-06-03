# Modern-C-Combat
现代C++实战30讲（By GeekTime 吴永炜）

### 一、基础篇

#### 1、堆、栈、RAII：C++里该如何管理资源？

**RAII（Resouce Acquisition Is Initialization）：**是 C++ 所特有的资源管理方式。有少量其他语言，如 D、Ada 和 Rust 也采纳了 RAII，但主流的编程语言中， C++是唯一一个依赖 RAII 来做资源管理的。

```c++
// 工厂方法的简单演示（函数返回值为基类）
enum class shape_type
{
    circle,
    triangle,
    rectangle.
    ...
};

class shapre {...};
class circle: public shape {...};
class triangle: public shapre {...};
class rectangle: public shapre {...};

shape* create_shapre(shapre_type type)
{
    ...
    switch(type)
    {
        case shape_type::circle:
            return new circle(...);
        case shapre_type::triangle:
            return new triangle(...);
        case shapre_type::rectangle:
            return new rectangle(...);
        ...
	}
}
```

​	该create_shapre方法会返回一个shape对象，但是对象的实际类型其实是shape的子类（circle、triangle、rectangle...）。此时，编译器不会报错，但结果多半是错的。这种现象叫**对象切片**(object slicing)，是C++特有的一种编码错误。不是语法错误，而是一个对象赋值相关的语义错误。

​	如何确保在使用create_shape的返回值时不会发生内存泄露呢？

```C++
// 将返回值放到一个本地变量里，并确保其析构函数会删除该对象即可
class shape_wrapper
{
public:
    explicit shape_wrapper(shapre* ptr = nullptr): ptr_(ptr){}
    ~shape_wrapper()
    {
        delete ptr_;
    }
    shape* get() const { return ptr_; }
private:
    shape* ptr_;
}

void foo()
{
    ...
    // 用函数返回值构造一个shape_wrapper对象，用类的构造与析构函数管理内存
    shape_wrapper ptr_wrapper(create_shapre(...));
    ...
}
```





在 new一个对象和 delete 一个指针时编译器需要干不少活的，它们大致可以如下翻译：

```C++
// new circle(…)
{
    // 申请内存空间
	void* temp = operator new(sizeof(circle));
	try 
    {
        // 在tmp指针上构造对象
		circle* ptr = static_cast<circle*>(temp);
		ptr->circle(…);
		return ptr;
	}
	catch (...) 
    {
		operator delete(ptr);
		throw;
	}
}

// delete circle
{
	if (ptr != nullptr) 
    {
		ptr->~shape();
		operator delete(ptr);
	}
}
```



#### 2、自己动手，实现C++的智能指针

拷贝构造函数，赋值运算符，移动构造函数

注意在拷贝时转移指针的所有权的写法：

```C++
template<typename T>
class smart_ptr
{
	...
    smart_ptr(smart_ptr& other)
    {
        ptr_ = other.release();
	}
    smart_ptr& operator=(smart_ptr& rhs)
    {
        // !!!!
        // 先用rhs够早了一个临时变量，与*this交换
        smart_ptr(rhs).swap(*this);
        return *this;
	}
    ...
    T* release()
    {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
	}
    void swap(smart_ptr& rhs)
    {
        using std::swap;
        swap(ptr_, rhs.ptr_);
	}
    ...
};
```

​	在拷贝构造函数中，通过调用 other 的 release 方法来释放它对指针的所有权。在赋值函数中，则通过拷贝构造产生一个临时对象并调用 swap 来交换对指针的所有权。实现上是不复杂的。

#### 3、右值和移动究竟解决了什么问题？

值分左右：

![1-2.1](.\pic\1-2.1.png)

- glvalue：generalized lvalue，广义左值
  - lvalue：通常只能放在等号左边的表达式，左值
- rvalue：通常只能放在等号右边的表达式，右值
  - prvalue：pure rvalue，纯右值
  - xvalue：expiring lvalue，将亡值

左值lvalue是有标识符、可取地址的表达式，最常见的情况有：

1、变量、函数或数据成员的名字

2、返回左值引用的表达式，如++x、x=1、count<<" "

3、字符串字面量如"hello world"

在数调用时，左值可以绑定到左值引用的参数，如`T&`。一个常量只能绑定到常左值引用，如`const T&`。

反之，纯右值**prvalue**是没有标识符、不可取渎职的表达式，一般也称之为“临时对象“。

最常见的情况有：

1、返回非引用类型的表达式，如x++，x+1，make_shared<int>(42)

2、除字符串字面量之外的字面量，如42、true

#### 4、容器汇编I：比较简单的若干容器

**vector：**

当`push_ back`、`insert`、 `reserve`、 `resize` 等函数导致内存重分配时，或当`insert`、`erase`导致元素位置移动时，`vector` 会试图把元素”移动”到新的内存区域。`vector` 通常保证**强异常安全性**，如元素类型**没有提供一个保证不抛异常的移动构造函数**，**`vector` 通常会使用拷贝构造函数**。因此,对于拷贝代价较高的自定元类型，我们应当定义移动构造函数，并标为noexcept，或只在容器中放置对象的智能指针。这就是为什么我之前需要在smart_ ptr的实现中标上noexcept的原因。

见 `emplace.cpp`

#### 5、容器汇编 II：需要函数对象的容器

**函数对象及其特化**：less、hash，见func_hash.cpp

**priority_queue**：优先队列（默认比较函数对象是less，即降序排列，大顶堆），见priority_queue.cpp。如果需要最小的数值出现在容器顶部（小顶堆），则可以传递greater作为其Compare模板参数。

**关联容器**:

- set（集合）
- map（映射）
- multiset（多重集）
- multimap（多重映射）

与序列容器相比，关联容器没有前、后的概念以及相关的成员函数，但同样提供insert、emplace等成员函数。此外，关联容器都有find、lower_bound、upper_bound等查找函数，返回结果是一个迭代器。

- find(k)    可以找到任何一个等价于查找键k的元素 (!(x < k || k < x))
- lower_bound(k)    找到第一个不小于查找键k的元素 (!(x < k ))
- upper_bound(k)    找到第一个大于查找键k的元素 (k < x)
- equal_range(k)    找到满足某个键的区间
  - multimap<string, int>::iteraotr lower, upper;
  - std::tie(lower, upper) = mmp.equal_range("four");

**无关联容器**：

- unordered_set

- unordered_map

- unordered_multiset

- unordered_multimap

  这些容器和关联容器非常相似，主要的区别就在于它们是**”无序”**的。这些容器**不要求提供一个排序的函数对象**，而要求一个可以计算哈希值的函数对象。你当然可以在声明容器对象时手动提供这样一个函数对象类型, 但更常见的情况是,我们使用标准的hash函数对象及其特化。见unordered_stl.cpp

**array:**

- 如果数组较大的话，应该考虑vector. vector 有最大的灵活性和不错的性能。
- 对于字符串数组，当然应该考虑string。
- 如果数组大小固定(C 的数组在C++里体来就是大小固定的)并且较小的话，应该考虑array。array 保留了C数组在栈上分配的特点。同时，提供了begin、end、size等通用成员函数。
