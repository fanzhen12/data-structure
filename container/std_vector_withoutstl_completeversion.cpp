// reference: https://www.doubao.com/chat/26654534756795138
#include <stdexcept> // for std::out_of_range
#include <utility> // for std::move, std::swap,std::forward
#include <new> // for placement new
#include <initializer_list> // for std::initializer_list(初始化列表)
#include <iterator> // 迭代器相关类型

template <typename T>
class MyVector {
public:
    // 迭代器类型定义（随机访问迭代器，兼容 STL 迭代器要求）
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>; // 反向迭代器
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; // 反向迭代器

    // 成员类型定义
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t; // 用于指针差值

    // ------- 构造与析构函数 -------
    // 默认构造（空容器）
    MyVector() noexcept
        : data_(nullptr), size_(0), capacity_(0) {}

    // 构造指定数量的元素（用value初始化）
    MyVector(size_type n, const T& value)
        : size_(n), capacity_(n) {
        if (n > 0) {
            data_ = allocate(capacity_);
            construct_n(data_, size_, value); // 用value拷贝构造n个元素
        } else {
            data_ = nullptr;
        }
    }

    // 迭代器范围构造（从[first, last)）
    template <typename InputIt>
    MyVector(InputIt first, InputIt last) {
        const size_type n = std::distance(first, last); // 计算范围内元素数量
        size_ = capacity_ = n;
        if (n > 0) {
            data_ = allocate(capacity_);
            construct_range(data_, first, last); // 从迭代器范围构造元素
        } else {
            data_ = nullptr;
        }
    }

    // 初始化列表构造
    MyVector(std::initializer_list<T> init) 
        : MyVector(init.begin(), init.end()) {} // 委托给迭代器范围构造函数
    // 上面这个函数中的init.begin()和init.end()分别返回初始化列表的起始和结束迭代器。
    // 这样可以方便地使用初始化列表来构造MyVector对象。
    // std::initializer_list是C++标准库提供的一个模板类，用于表示初始化列表。
    // 它允许我们以列表的形式初始化容器类，如std::vector、std::array等。

    // 拷贝构造（深拷贝，强异常安全）
    MyVector(const MyVector& other) 
        : size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            data_ = allocate(capacity_);
            try {
                // 若拷贝构造抛出异常，则进入catch块释放已分配内存
                construct_range(data_, other.data_, other.data_ + size_);
            } catch (...) {
                deallocate(data_);
                throw; // 重新抛出异常，保证强异常安全
            }
        } else {
            data_ = nullptr;
        }
    }

    // 移动构造（资源转移，不抛异常）
    MyVector(MyVector&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
            // 原对象置为空状态
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
    }

    // 析构函数：销毁元素并释放内存
    ~MyVector() noexcept {
        destroy_range(data_, data_ + size_); // 销毁元素
        deallocate(data_); // 释放内存
    }

    // ------- 赋值运算符 -------
    

private:
    T* data_; // 指向连续内存块的指针
    size_t size_; // 当前元素数量
    size_t capacity_; // 容量

private:
    // ------- 内存管理辅助函数 -------

    // 分配原始内存（不构造元素）
    static pointer allocate(size_type n) {
        if (n == 0) {
            return nullptr;
        }
        return static_cast<pointer>(::operator new[](n * sizeof(T)));
    }

    // 释放原始内存（不销毁元素）
    static void deallocate(pointer p, size_type n) {
        if (p) {
            ::operator delete[](p); // 释放以p为起始地址的内存块，释放多少个呢？
            // C++标准规定，delete[]不需要知道具体释放多少个元素
            // 因为编译器会在分配内存时记录大小信息
        }
    }

    // 构造n个值初始化的元素(T())
    static void contruct_n(pointer p, size_type n) {
        for (size_type i = 0; i < n; ++i) {
            new (p + i) T(); // value-initialize，调用T类型的默认构造函数
        }
    }

    // 构造n个用value拷贝初始化的元素
    static void contruct_n(pointer p, size_type n, const T& value) {
        for (size_type i = 0; i < n; ++i) {
            new (p + i) T(value); // copy-construct,调用T类型的拷贝构造函数
        }
    }

    // 从迭代器范围[first, last)构造元素
    template <typename InputIt>
    static void construct_range(pointer p, InputIt first, InputIt last) {
        for (; first != last; ++first, ++p) {
            new (p) T(*first); // 用迭代器指向的元素拷贝构造。为什么这里要解引用first？因为对迭代器进行解引用可以获取其指向的元素值。
            // 迭代器相当于一个智能指针，解引用操作符*用于获取迭代器所指向的元素。
            // 迭代器是地址吗？不是，迭代器是一种抽象的概念，它封装了对容器中元素的访问方式。
            // 所以虽然和指针类似，但迭代器并不直接表示内存地址，而是提供了一种统一的接口来访问容器中的元素。
        }
    }

    // 移动元素范围[first, last)到目标位置p
    static void move_range(pointer p, pointer first, pointer last) {
        for (; first != last; ++first, ++p) {
            // 若T的移动构造可能抛异常，则使用拷贝构造以保证强异常安全
            new (p) T(std::move_if_noexcept(*first));
        }
    }

    // 销毁元素范围[first, last)
    static void destroy_range(pointer first, pointer last) {
        for (; first != last; ++first) {
            first->~T(); // 显式调用析构函数
        }
    }


};

// 全局swap函数（支持ADL查找）
// ADL是指Argument-Dependent Lookup，即基于参数的查找机制。
template <typename T>
void swap(MyVector<T>& a, MyVector<T>& b) noexcept {
    a.swap(b);
}
// 为什么上面这个函数是noexcept的？
// 因为它只是交换两个指针和两个size_t变量的值，这些操作不会抛出异常。