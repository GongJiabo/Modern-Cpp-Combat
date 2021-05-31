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
