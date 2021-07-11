# Modern-Cpp-Combat
现代C++实战30讲（By GeekTime 吴永炜）

### 一、基础篇

#### 1、堆、栈、RAII：C++里该如何管理资源？

##### **RAII（Resouce Acquisition Is Initialization）：**

​	是 C++ 所特有的资源管理方式。有少量其他语言，如 D、Ada 和 Rust 也采纳了 RAII，但主流的编程语言中， C++是唯一一个依赖 RAII 来做资源管理的。

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

#### 6、异常：用还是不用，这是个问题

**异常的问题**：

- 异常违反了“你不用管就不需要付出代价”的C++原则。只要开启了异常，即使不使用异常你编译出的二进制代码通常也会膨胀。（牺牲可执行文件的大小、提高主流程的性能）
- 异常比较隐蔽，不容易看出来了哪些地方会发生异常和发生什么异常。（如果一个函数声明了不会抛出异常、结果却抛出了异常，C++运行时会调用std::terminate来终止应用程序）

#### 7、迭代器和好用的新for循环

**迭代器的类型关系：**

![1-7.1](.\pic\1-7.1.png)

迭代器通常是对象。但需要注意的是，指针可以满足上面所有的迭代器要求，因而也是迭代器。这应该并不让人惊讶，因为本来迭代器就是根据指针的特性，对其进行抽象的结果。事实上，vector的迭代器，在很多实现里就直接是使用指针的。

**常见迭代器：**

```c++
#include <algorithm>	// std::copy
#include <iterator>		// std::back_inserter
#include <vector>		// std::vector
#include <iostream>		// std::cout

vector<int> v1{1, 2, 3, 4, 5};
vector<int> v2;
copy(v1.begin(), v1.end(), back_inserter(v2));		// {1, 2, 3, 4, 5}

copy(v2.begin(), v2.end(), ostream_iterator<int>(cout, " "));	// 打印输出 1 2 3 4 5
```

**输入行迭代器：**

- istream_line_reader

```c++
// 基于范围的for循环
for (const string& line : istream_line_reader(is))
{
	// 示例循环体中仅进行简单输出
    cout << line << endl;
}

// 使用传统循环
{
    auto&& r = istream_line_reader(is);
    auto it = r.begin();
    auto end = r.end();
    for(; it != end; ++it)
    {
        const string& line = *it;
        cout << line << endl;
	}
}
```

对比一下以传统的方式写的C++代码，其中需要照顾不少细节：

```c++
string line;
for (;;)
{
	getline(is, line);
	if(!is)
		break;
	cout << line << endl;
}
```

从is读入输入行的逻辑，在前面的代码里一个语句就全部搞定了，在这儿用了5个语句...

**定义输入行迭代器：**

 

```c++
class istream_line_reader
{
public:
    class iterator		// 实现 InputIterator
    {
    public:
        typedef prdiff_t difference_type;
        typedef string value_type;
        typedef const value_type* pointer;
        typedef const value_type& referenc;
        typedef input_iterator_tag iterator_category;
        // ...
	};
    // ...
};
```

​	仿照一般的容器，我们把迭代器定义为istream_line_reader的套嵌类。它里面的这**五个类型是必须定义**的（其他的泛型C++代码可能会用到这五个类型；之前标准库定义了一个可以继承的类模板std::iterator来产生这些类型定义，但这个类目前已经被废弃）。其中：

- difference_type 是代表迭代器之间距离的类型，定义为ptrdiff_t只是标准做法（指针间差值的类型），对这个类型没有什么特别作用。
- value_type 是迭代器指向的对象的类值类型，我们使用string，表示迭代器指向的是字符串。
- pointer是迭代器指向的对象的指针类型，这儿就平淡无奇地定义为value_type的常指针了（我们可不希望别人来更改指针指向的内容）。
- 类似的，reference是value_type的常引用。
- iterator_category被定义为input_iterator_tag，标识这个迭代器的类型是input_iterator（输入迭代器）。

```c++
class istream_line_reader
{
public:
    class iterator
    {
        // ...
        iterator() noexcept : stream_(nullptr) {}
        
        explicit iterator(istream& is) : stream(&is)
        {
			++*this;
		}
        
        // reference?
        reference operator*() const noexcept
        {
            return line_;
		}
        
        // pointer?
        pointer operator->() const noexcept
        {
            return &line_;
		}
        
        // 前缀自增符
        iterator& operator++()
        {
            getline(*stream_, line_);
            if(!*stream_)
                stream_ = nullptr;
            return *this;
		}
        
        // 后缀自增符
        iterator operator++(int)
        {
            iterator temp(*this);
            ++*this;
            return temp;
		}
        
        bool operator==(const iterator& rhs) const noexcept
        {
            return stream_ == rhs.stream_;
		}
        
        bool operator!=(const iterator& rhs) const noexcept
        {
            return !operator==(rhs);
		}
        
    private:
    	istream* stream_;
    	string line;
	};
	// ....
    
   	istream_line_reader() noexcept : stream_(nullptr) {}
    
    explicit istream_line_reader(istream& is) noexcept : stream_(&is)
    {
        return iterator(*stream_);
	}
    
    iterator begin()
    {
        return iterator(*stream_);
	}
    
    iterator end() const noexcept
    {
        return iterator();
	}
    
private:
    istream* stream_;
};
```

#### 8、易用性改进 I：自动类型推断和初始化

##### **自动类型判断—auto**

​	auto并没有改变C++静态类型语言**这一实使用auto的变量(或函数返回值)的类型仍然是**编译时就确定了，只不过编译器能自动帮你填充而已。**

历史累赘，模板函数中：

```c++
template <typename T>
void foo(const T& container)
{
	for (typename T::const_iterator it = v.begin(), ..., ...)
    // ...
}
```

​	如果`begin`返回的类型不是该类型的`const_ iterator` 嵌套类型的话，那实际上不用自动类型推断就没法表达了。这还真不是假设。比如，如果我们的遍历函数要求支持C数组的话，不用自动类型推断的话，就只能使用两个不同的**重载**:

```c++
// 支持C数组
template <typename T, std::size_t N>
void foo(const T (&a)[N])
{
    typedef const T* ptr_t;
    for(ptr_t it = a, end = a + N; it != end; ++it)
    {
        // 循环体...
	}
}

template <typename T>
void foo(const T& c)
{
    for(typename T::const_iterator it = c.begin(), end = c.end();
       it != end; ++it)
    {
        // 循环体...
	}
}
```

如果使用类型推断的话，再加上 C++11提供的全局begin和end函数，上面的代码可以统一成：

```c++
template <typename T>
void foo(const T& c)
{
    using std::begin;
    using std::end;
    // 使用依赖参数查找(ADL)
    // begin、end函数参数可以是容器/数组指针
    for(auto it = begin(c), ite = end(c); it!=end; ++it)
    {
        // 循环体...
	}
}
```

##### decltype—获得一个表达式的类型

- decltype(变量名)可以获得变量的精确类型
- decltype(表达式)（表达式不是变量名，但包括decltype((变量名)）的情况）可以获得**表达式的引用类型**；除非表达式的结果是个**纯右值(prvalue)**，此时结果仍然是值类型。

如果我们有 `int a;`那么：

- decltype(a) 会获得`int`（因为a是int）

- decltype((a))会获得`int&`（因为a是**lvalue**）
- decltype(a + a)会获得`int`（因为a + a是**prvalue**）

##### decltype(auto)

​	通常情况下，能写`auto`来声明变量肯定是比较轻松的事。但这儿有个限制，你需要在写下`auto`时就决定你写下的是个**引用类型**还是**值类型**。根据类型推导规则，`auto`是值类型，`auto&`是左值引用类型，`auto&&`是转发引用（可以是左值引用，也可以是右值引用）。使用`auto`不能通用地根据表达式类型来决定返回值的类型。不过，`decltype(expr)`既可以是值类型，也可以是引用类型。因此我们可以这么写：

```c++
decltype(expr) a = expr;

// 对C++14
decltype(auto) a = expr;
```

##### 函数返回值类型推断

​	从C++14开始，**函数的返回值**也可以用`auto`或`decltype(auto)`来声明了。同样的，用`auto`可以得到值类型，用`auto&`或者`auto&&`可以得到引用类型；而用`decltype(auto)`可以根据返回表达式通用地决定返回的是值类型还是引用类型。

​	后置返回值类型声明：

```c++
auto foo(参数) -> 返回值类型声明
{
	// 函数体...
}
```

类模板的模板参数推导

```c++
// pair
pair<int, int> pr{1, 42};

// make_pair
auto pr = make_pair(1, 42);

// C++17后，因为函数模板有模板参数推导(编译器根据构造函数来自动生成的)，使得调用者不必手工指定参数类型
pair pr{1, 42};

// array
int a1[] = {1, 2, 3};
array<int, 3> a2{1, 2, 3};	// 啰嗦
// array<int> a3{1,2,3};	// 不行
array a{1, 2, 3} 			// C++17

```

##### 结构化绑定

​	在讲关联容器中的时候有过这样的一个例子：

```c++
multimap<string, int>::iterator lower, upper;
// 用两个变量来接收数值
std::tie(lower, upper) = mmp.equal_range("four");
// C++17简化为
auto [lower, upper] = mmp.equal_range("four");
```

##### 统一初始化(uniform initialization)

​	在代码里使用了大括号{}来进行对象的初始化。这当然也是C++11引入的新语法梦够代替很多小括号（）在变量初始化时使用。

**特点：当一个构造函数没有标成explicit时，你可以使用大括号不写类名来进行构造：**

```c++
Obj getObj()
{	
	return {1.0};
}
```

​	如果Obj类可以使用浮点数进行构造的话，上面的写法就是合法的。如果有无参数、多参数的构造函数，也可以使用这个形式。除了形式上的区别，他跟`Obj(1.0)`的主要区别是，后者可以用来调用`Obj(int)`，而使用大括号时编译器会拒绝“窄”转换，不接受以`{1.0}`或者`Obj{1.0}`的形式调用构造函数`Obj(int)`。

​	这个语法主要的限制是，如果一个构造函数**既有使用初始化列表的构造函数**，**又有不使用初始化列表的构造函数**，那编译器会**千方百计地试图调用使用初始化列表的构造函数**，导致各种意外。

##### 类数据成员的默认初始化

​	按照C+ +98的语法，**数据成员可以在构造函数里进行初始化**。这本身不是问题，但实践中，如果数据成员比较多、构造函数又有多个的话，逐个去初始化是个累赘，且很容易在增加数据成员时漏掉在某个构造函数中进行初始化。为此，C++11 增加了一个语法，**允许在声明数据成员时直接给予一个初始化表达式**。这样，当且仅当构造函数的初始化列表中不包含该数据成员时，这个数据成员就会自动使用初始化表达式进行初始化。

```c++
class Complex
{
private:
    float re_;
    float im_;
public:
	Complex() : re_(0), im_(0) {}
    Complex(float re) : re_(re), im_(0) {}
    Complex(float re, float im) : re_(re), im_(im) {}
    // ...
};

// 使用数据成员的默认初始化
class Complex
{
private:
	float re_{0};
    float im_{0};
public:
    Complex() {}
    Complex(float re) : re_(re) {}
    Complex(float re, float im) : re_(re), im_(im) {}
    // ...
}
```

- 第一个构造函数没有任何初始化列表，所以类数据成员的初始化全部由**默认初始化**完成，`re`_和`im_`都是0。

- 第二个构造函数提供了``re_`的初始化， `im_` 仍由默认初始化完成。
- 第三个构造函数则完全不使用默认初始化。

#### 9、易用性改进 II：字面量、静态断言和成员函数说明符

##### 自定义字面量

```c++
#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

static constexpr double factors[] =
    {1.0, 1000.0, 1e-3, 1e-2, 0.0254, 0.3048, 0.9144, 1609.344};
    
struct length
{
    double value;
    enum unit
    {
        metre,
        kilometre,
        millimetre,
        centimetre,
        inch,
        foot,
        yard,
        mile
    };

    explicit length(double v, unit u = metre)
    {
        value = v * factors[u];
    }
};


length operator+(length lhs, length rhs)
{
    return length(lhs.value + rhs.value);
}

length operator""   _m(long double v)
{
    return length(v, length::metre);
}

length operator"" _cm(long double v)
{
    return length(v, length::centimetre);
}

int main()
{
    // 虚数类
    // 输出 i * i = (-1,0)
    cout << "i * i = " << 1i * 1i << endl;
    
    // 自定义量
    length m1(1.0, length::metre);
    length m2 = m1 + 12.0_cm;
    return 0;
}
```

##### 二进制字面量

十六进制： 0xffffffff

八进制：     077777777

c++14中的二进制直接的字面量：`unsigned mask = 0b11100000`

##### 静态断言

C++11直接从语言层面提供了**静态断言机制**，不仅能输出更好的信息，而且适用性也更好，可以直接放在类的定义中，而不像之前用的特殊技巧只能放在函数体里。

```c++
// staic_assert(编译期条件表达式, 可选输出信息);
static_assert((alignment & (alignment - 1)) == 0, "Alignment must be power of two");
```

##### override和final说明符

**override**显示声明了函数成员是一个虚函数且**覆盖了基类中的该函数**。如果有override声明的函数不是虚函数，或者基类中不存在这个虚函数，编译期会报告错误。这个说明符的主要作用有两个：

- 给开发人员更明确的提示，这个函数覆写了基类的成员函数；

- 让编译期进行额外的检查，放置程序员由于拼写错误或代码改动没有让基类和派生类中成员名称完全一致。

**final**则声明了成员函数是一个虚函数，且该虚函数不可在派生类中覆盖。如果有一点没有得到满足的话，编译期就会报错。

final还有给作用是标志某个类或结构不可被派生。同样，这时应将其放在被定义的类或结构名后面。



### 二、提高篇

#### 10 、到底应不应该返回对象？

**之前的做法：调用者负责管理内存，接口负责生成**

一种常见的做法是，接口的调用者负责分配一个对象所需的内存并负责其声明周期，接口负责生成或修改对象。这种做法意味着**对象可以默认构造**（甚至只是一个结构），代码一般使用错误码而非异常。

（理解：函数接口内分配内存，并返回对应的对象，需要调用者在接口外自行管理内存）

**现在的做法：接口提供生成和销毁对象的函数，对象在堆上维护。**

fopen和fclose就是这样的接口的实例。注意使用这种方法一般不推荐由接口生成对象，然后右调用者通过调用delete来释放。在某些坏境里，比如windows上使用不同的运行时库时，这样做会引发问题。

PS：直接使用智能指针

```c++
shared_ptr<matrix> add(const shader_ptr<matrix>& lhs, const shared_ptr<matrix>& rhs);
shared_ptr<matrix> multiply(const shader_ptr<matrix>& lhs, const shared_ptr<matrix>& rhs);
// ...
auto r = add(multipy(a,b),c);
```

##### 如何返回一个对象？

一个用来**返回的对象**，通常应当是**可移动构造/赋值的**，一般也同时是可拷**贝构造/赋值**的。如果这样一个对象同时又可以**默认构造**，我们就称其为一个**半正则（semiregular） 的对象**。如果可能的话，我们应当尽量让我们的类满足半正则这个要求。

半正则意味着我们的matrix类提供下面的成员函数：

```c++
class matrix
{
public:
	// 普通构造
    matrix(size_t rows, sie_t cols);
    // 半正则要求的构造
    matrix();
    matrix(const matrix&);
    matrix(matrix&&);
    // 半正则要求的赋值
    matrix& operator=(const matrix&);
    matrix& operator=(matrix&&);
}
```

没有返回值优化的情况下C++返回对象，以矩阵乘法为例：

```c++
matrix operator*(const matrix& lhs, const matrix& lhs)
{
    if(lhs.cols() != rhs.rows())
    {
        throw runtime_error("size mismatch");
	}
    matrix result(lhs.rows(), rhs.cols());
    // 具体计算过程
    return result;
}
```

​	注意对于一个**本地变量，我们永远不应该返回其引用(或指针)**， 不管是作为左值还是右值。从标准的角度，这会导致未定义行为(undefined behavior)，从实际的角度，这样的对象一般放在栈上可以被调用者正常覆盖使用的部分，随便一个函数调用或变量定义就可能覆盖这个对象占据的内存。这还是这个对象的析构不做事情的情况：如果析构函数会释放内存或破坏数据的话，那你访问到的对象即使内存没有被覆盖，也早就不是有合法数据的对象了....

​	回到正题。我们需要回想起，在第3讲里说过的，返回**非引用类型的表达式结果是个纯右值(prvalue)** 。在执行`auto r = ..` 的时候，编译器会认为我们实际是在构造`matrixr(...)`，而“..”部分是一个纯右值。**因此编译器会首先试图匹配matrix (matrix&&)**，在没有时则试图匹配matrix (const matrix&) ;也就是说，**有移动支持时使用移动，没有移动支持时则拷贝**。

NOTE：函数返回局部对象时，有移动构造调用移动构造，如果没有则调用拷贝构造。

##### 返回值优化（拷贝消除）

再来看一个能显示生命期过程的对象的例子：

```c++
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
    // Method1
    A a;
    return a;	
    // Method2
    // return A();
}

int main()
{
    auto a = getA_unnamed();
}
```

VS2019中，Method1下输出

```c++
Create A
Move A
Destory A
Destory A
```

说明，返回内容被移动构造了，为什么destory了两次？

Method2下输出

```
Create A
Destory A
```

调用编译器默认生成的拷贝构造函数

##### 性能说明

- “对于非值类型， 比如返回值可能是**子对象的情况**，使用`unique_ ptr` 或`shared_ ptr`来返回对象。” 也就是**面向对象、工厂方法**这样的情况，像第1讲里给出的create_ shape 应该这样改造。
- “对于移动代价很高的对象， 考虑将其**分配在堆**上，然后返回一个句柄(如
  unique_ ptr)，或传递一个**非const的目标对象的引用来填充(用作输出参数**)。” 也就是说不方便移动的，那就只能使用一个**RAII**对象来管理生命周期，或者老办法输出参数了。
- “要在一个**内层循环里在多次函数调用中重用一个自带容量的对象**：将其当作输入/输出参数并将其按引用传递。” 这也是个需要继续使用老办法的情况。

#### 11 、Unicode：进入多文字支持的世界

##### ASCII

​	ASCII是一种创立于1963年的7位编码，用0到127的数值来表示最常用的字符，包含了控制字符（很多在今天已经不再使用）、数字、大小写拉丁字母、空格和基本标点。

##### GB2312

​	最早的中文字符集标准是1980年的国标GB2312，其中收录了6763个常用汉子和682个其他符号。我们平时会用到编码GB2312，其实更正确的名字是EUC-CN，它是一种与ASCII兼容的编码方式。它用单字节表示ASCII字符而用双字节表示GB2312中的字符；由于GB2312中本身也含有ASCII中包含的字符，在使用中逐渐就形成了“半角”和“全角”的区别。

​	国标字符集后面又有扩展，这个扩展后的字符集就是GBK，是中文版Windows使用的标准编码方式。

##### Unicode

​	Unicode作为一种统一编码的努力， 诞生于八十年代末九十年代初，标准的第一版出版于1991-1992年。由于最初发明者的目标放得太低，只期望对活跃使用中的现代文字进行编码，他们认为16比特的“宽ASCII"就够用了。这就导致了早期采纳Unicode的组织，特别是微软，在其操作系统和工具链中广泛采用了16比特的编码方式。在今天,微软的系统中宽字符类型`wchar_ t`仍然是16位的，操作系统底层接口大量使用16位字符编码的API，说到Unicode编码时仍然指的是16位的编码UTF-16 （这一不太正确的名字，跟中文GBK编码居然可以被叫做ANSI相比，实在是小巫见大巫了）。在微软以外的世界， Unicode本身不作编码名称用，并且最主流的编码方式并不是UTF-16，而是和ASCII全兼容的UTF-8。

​	Unicode在今天已经大大超出了最初的目标。到Unicode12.1为止，Unicode 已经包含了137,994个字符，囊括所有主要语言(使用中的和E R经不再使用的)，并包含了**表情符号、数学符号等各种特殊字符**。仍然要指出一下，Unicode 字符是根据**含义来区分的，而非根据字形**。除了前面提到过中日韩汉字没有分开，像斜体(italics) 、小大写字母(small caps)等排版效果在Unicode里也没有独立的对应。

​	Unicode的**编码点**是从`0x0到0x10FFFF`，一共1,114,112个位置。一般用"U+" 后面跟16进制的数值来表示一个Unicode字符，如`U+0020`表示空格，`U+6C49`表示“汉”等等(不足四位的一般写四位)。

 Unicode字符的常见编码方式有：

- UTF-32：32比特，是编码点的直接映射。
- UTF-16：对于从`U+0000`到`U+FFFF`的字符，使用16比特的直接映射；对于大于
  `U+FFFF`的字符，使用32比特的特殊映射关系一在 Unicode的16比特编码点中
  `0xD800` - `0xDFFF`是一段空隙, 使得这种变长编码成为可能。
- **UTF-8**：**1到4字节的变长编码**。在一个合法的UTF-8的序列中，如果看到一个**字节的最高位是0，那就是一个单字节的Unicode字符**；如果一个字节的**最高两比特是10，那这是一个Unicode字符在编码后的后续字节**；否则，这就是一个Unicode字符在编码后的首字节，且最高位开始连续1的个数表示了这个字符按UTF-8的方式编码有几个字节。

​    在上面三种编码方式里，**只有UTF-8完全保持了和ASCII的兼容性**，目前得到了最广泛的使用。

​    略。。。讲述UTF-8/16/32以及BOM（byte order mark 用来判断大小端表示）的区分方式，主要根据文件开头的字符判断。

##### C++中的Unicode字符类型

​	C+ +98中有char和wchar_ t两种不同的字符类型，其中char的长度是单字节，而wchar_ t的长度不确定。在Windows.上它是双字节，只能代表UTF-16，而在Unix上一般是四字节，可以代表UTF-32。为了解决这种混乱，目前我们有了下面的改进:

- C++11中引入了`char16_t`和`char32_t`两个独立的字符类型（不是类型别名），分别代表UTF-16和UTF-32。
- C++20将引入`char8_t`类型，进一步区分了可能使用传统编码的窄字符类型和UTF-8字符类型。
- 除了string和wstring，我们也相应地有了u16string、u32string（和将来的u8string）。
- 除了传统的窄字符/字符串字面量（如"hi"）和宽字符/字符串字面量（如L"hi"），引入了新的UTF-8、UTF-16和UTF-32字面量，分别形如u8"hi"、u"hi"和U"hi"。
- 为了确保非ASCII字符在源代码中可以简单地输入，引入了新的Unicode换码序列。（编码转换，在此不作详细介绍）

##### 平台区别

**Unix**：

​	现代Unix系统，包括Linux和macOS在内，已经全面**转向了UTF-8**。这样的系统中一般**直接使用char[]和string来代表UTF-8字符串**，包括输入、输出和文件名，非常简单。不过，由于一个字符单位不能代表一个完整的 Unicode字符，在需要真正进行文字处理的场合转换到UTF-32往往会更简单。在以前及需要和C兼容的场合，会使用wchar_ t、uint32_ t或某个等价的类型别名；在新的纯C++代码里，就没有理由不使用char32_ t和u32string了。

​	Unix下输出宽字符串需要使用`wcout` (这点和Windows相同)，并且需要进行区域设置，通常使用`setlocale(LC_ ALL，"en_ US.UTF-8")` ；即足够。由于没有什么额外好处，Unix平台下一般只用cout，不用wcout。

**Windows：**

​	Windows由于历史原因和保留向后兼容性的需要（Windows 为了向后兼容性已经到了大规模放弃优雅的程度了)，一直用 char表示传统编码(如， 英文Windows上是Windows-1252，简体中文Windows上是GBK)，用`wchar_ t`表示UTF-16。由于传统编码一次有一种、 且需要重启才能生效，要得到好的多语言支持，在和操作系统交互时必须使用UTF-16。

​	对于纯Windows编程，**全面使用宽字符(串)是最简单的处理方式**。当然，**源代码和文本很少用UTF-16存储**，**通常还是UTF-8** (除非是纯ASCII，否则需要加入BOM字符来和传统编码相区分)。这时可能会有一个小小的令人惊讶的地方：**微软的编译器会把源代码里窄字符串字面量中的非ASCII字符转换成传统编码**。换句话说，**同样的源代码在不同编码的Windows下编译可能会产生不同的结果**！如果你希望保留UTF-8序列的话，就应该使用UTF-8字面量(并在将来使用char8_ t字符类型)。（跨平台的时候会出现问题。。所以软件代码，Qt程序中一般都用宽字符如L“hi"）。

```c++
#include <stdio.h>

template<typename T>
void dump(const T& str)
{
    for(char ch : str)
    {
        printf("%.2x",
              static_cast<unsigned char>(ch));
	}
    putchar('\n');
}

int main()
{
    char str[] = "你好";
    char u8str[] = u8"你好";
    dump(str);
    dump(u8str);
}
```

##### 统一化处理

要想写出跨平台的处理字符串的代码，我们一般考虑两种方式之一:

- 源代码级兼容，但内码不同（微软推荐）
- 源代码和内码都完全兼容

​    微软推荐的方式一般是前者。做Windows开发的人很多都知道`tchar.h`和`_ _T 宏`，它们就起着类似的作用(虽然目的不同)。**根据预定义宏的不同，系统会在同一套代码下选择不同的编码方式及对应的函数**。拿一个最小的例子来说:

```c++
#include<stdio.h>
#include<tchar.h>

int _tmain(int argc, TCHAR* argv[])
{
	_putts(_T("Hello World!\n"));
}
```

如果用缺省的命令行参数进行编译，上面的代码相当于：

```c++
#include<stdio.h>

int tmain(int argc, char* argv[])
{
	puts(_T("Hello World!\n"));
}
```

而如果在命令行上加上了`/D_UNICODE`，那么代码则相当于：

```c++
#include<stdio.h>

int tmain(int argc, wchar_t* argv[])
{
	_putws(_T("Hello World!\n"));
}
```

​	当然，这个代码还是只能在Windows上用，并且仍然不漂亮(所有的字符和字符串字面量都得套上`_T`)。后者无解，前者则可以找到替代方案(甚至自己写也不复杂)。C++  RESTSDK中就提供了类似的封装，可以跨平台地开发网络应用。但可以说，这种方式是一种主要照顾Windows的开发方式。

​	相应的，对Unix开发者而言更自然的方式是**全面使用UTF-8**，**仅在跟操作系统、文件系统打交道时把字符串转换成需要的编码**。

略去windows中的支持Unicode及其转换的API介绍。。。
