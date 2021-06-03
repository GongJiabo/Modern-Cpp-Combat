#include <utility>  // std::swap
#include <iostream>
#include <algorithm>

// 引用计数类
class shared_count
{
private:
    long count_;
public:
    shared_count() noexcept: count_(1)
    {
    }

    void add_count() noexcept
    {
        ++count_;
    }

    long reduce_count() noexcept
    {
        --count_;
        return count_;
    }

    long get_count() const noexcept
    {
        return count_;
    }
};

// 智能指针类
template <typename T>
class smart_ptr
{
private:
    T* ptr_;
    shared_count* shared_count_;
public:
    // 声明友元，访问其他实体类型的引用变量
    template <typename U>
    friend class smart_ptr;
    //
    // 构造函数
    explicit smart_ptr(T* ptr = nullptr): ptr_(ptr)
    {
        if(ptr)
        {
            shared_count_ = new shared_count();
        }
    }
    // 析构函数
    ~smart_ptr()
    {
        if(ptr_ && shared_count_->reduce_count()==0)
        {
            delete ptr_;
            delete shared_count_;
        }
    }
    // 拷贝构造函数
    smart_ptr(const smart_ptr& other) noexcept
    {
        ptr_ = other.ptr_;
        if(ptr_)
        {
            // 如果指针存在，other的引用计数器+1
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }

    template <typename U>
    smart_ptr(const smart_ptr<U>& other) noexcept
    {
        ptr_ = other.ptr_;
        if(ptr_)
        {
            // 如果指针存在，other的引用计数器+1
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }
    // 移动构造函数
    template <typename U>
    smart_ptr(const smart_ptr<U>&& other) noexcept
    {
        ptr_ = other.ptr_;
        if(ptr_)
        {
            shared_count_ = other.shared_count_;
            other.ptr_ = nullptr;
        }
    }
    // 智能指针需要实现类似的函数模板。实现本身并不复杂，但为了实现这些转换，
    // 需要添加构造函数，允许在对智能指针内部的指针对象赋值时，使用一个现有的智能指针的共享计数。
    // 用于实现类型转换的函数模板!!!
    template <typename U>
    smart_ptr(const smart_ptr<U>& other, T* ptr) noexcept
    {
        ptr_ = ptr;
        if(ptr_)
        {
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }

    // swap函数
    void swap(smart_ptr& rhs) noexcept
    {
        using std::swap;
        swap(ptr_, rhs.ptr_);
        swap(shared_count_, rhs.shared_count_);
    }

    // 重载赋值运算符(通过交换实现，形参本来就是传值，不影响原来传入的rhs)
    smart_ptr& operator=(smart_ptr rhs) noexcept
    {
        rhs.swap(*this);
        return *this;
    }

    // 返回智能指针ptr_成员变量
    T* get() const noexcept
    {
        return ptr_;
    }

    // 返回引用计数
    long use_count() const noexcept
    {
        if(ptr_)
        {
            return shared_count_->get_count();
        }
        else
        {
            return 0;
        }
    }

    // 重载运算符
    // * 解引用
    T& operator*() const noexcept
    {
        return *ptr_;
    }
    // -> 箭头(返回指针)
    T* operator->() const noexcept
    {
        return ptr_;
    }
    // bool()
    operator bool() const noexcept
    {
        return ptr_;
  
    }
};

// swap全局函数
template <typename T>
void swap(smart_ptr<T>& lhs, smart_ptr<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

// C++强制类型转换
// static_cast
template <typename T, typename U>
smart_ptr<T> static_pointer_cast(const smart_ptr<U>& other) noexcept
{
    T* ptr = static_cast<T*> (other.get());
    return smart_ptr<T>(other, ptr);
}
// reinterpret_cast
template <typename T, typename U>
smart_ptr<T> reinterpret_pointer_cast(const smart_ptr<U>& other) noexcept
{
    T* ptr = reinterpret_cast<T*> (other.get());
    return smart_ptr<T>(other, ptr);
}
// const_cast
template <typename T, typename U>
smart_ptr<T> const_pointer_cast(const smart_ptr<U>& other) noexcept
{
    T* ptr = const_cast<T*> (other.get());
    return smart_ptr<T>(other, ptr);
}
// dynamic_cast
template <typename T, typename U>
smart_ptr<T> dynamic_pointer(const smart_ptr<U>& other) noexcept
{
    T* ptr = dynamic_cast<T*> (other.get());
    return smart_ptr<T>(other, ptr);
}
