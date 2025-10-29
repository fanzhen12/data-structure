#include <stdexcept> // 用于异常处理
#include <new> // 用于 placement new

template <typename T>
class MyVector {
private:
    T* data_;          // 指向动态分配数组的指针
    size_t size_;      // 当前元素数量
    size_t capacity_;  // 当前分配的容量

public:
    // 默认构造（空容器）
    MyVector() : data_(nullptr), size_(0), capacity_(0) {}

    // 构造指定数量的元素（默认初始化）
    explicit MyVector(size_t n) : size_(n), capacity_(n) {
        if (n > 0) {
            data_ = static_cast<T*>(operator new[](n * sizeof(T))); // 分配原始内存（只分配内存，不构造对象）
            // 显式构造n个默认元素
            for (size_t i = 0; i < n; ++i) {
                new (data_ + i) T(); // 使用placement new构造默认对象
                // 上面这一行代码的解释如下：
                // data_ + i表示从data_这个地址处偏移i个T类型的大小，得到第i个元素的位置
                // new (data_ + i) T()表示在这个位置调用T的默认
            }
        } else {
            data_ = nullptr;
        }
    }

    // 拷贝构造（深拷贝）为什么不是浅拷贝？
    // 因为如果是浅拷贝，两个对象会共享同一块内存，当一个对象析构时会释放这块内存，另一个对象就会变成悬空指针，导致未定义行为
    // 同时还可能会引发双重释放的问题
    MyVector(const MyVector& other) : size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            data_ = static_cast<T*>(operator new[](capacity_ * sizeof(T))); // 分配内存
            // 拷贝构造每个元素
            for (size_t i = 0; i < size_; ++i) {
                new (data_ + i) T(other.data_[i]); // 使用placement new拷贝构造对象，调用T的拷贝构造函数
            }
        } else {
            data_ = nullptr;
        }
    }

    // 析构函数：销毁所有元素并释放内存
    ~MyVector() {
        // 先销毁所有元素
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T(); // 显式调用析构函数
        }
        // 然后释放内存
        operator delete[](data_);
        // 上一句代码的解释如下：
        // operator delete[]: 用于释放通过operator new[]分配的内存
        // data_: 指向要释放的内存块的指针，实际上是指向动态数组的首地址
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // 赋值运算符（深拷贝）
    MyVector& operator=(const MyVector& other) {
        if (this != &other) {
            // 销毁当前元素并释放内存
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~T(); // 显式调用析构函数
            }
            operator delete[](data_);
        }
        // 拷贝新数据
        size_ = other.size_;
        capacity_ = other.capacity_;
        if (capacity_ > 0) {
            data_ = static_cast<T*>(operator new[](capacity_ * sizeof(T))); // 分配内存
            for (size_t i = 0; i < size_; ++i) {
                new (data_ + i) T(other.data_[i]); // 拷贝构造每个元素
            }
        } else {
            data_ = nullptr;
        }
        return *this;
    }

    // 元素访问（支持随机访问）
    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index]; // 这里的[]是C++内置数组的下标访问操作符，是C++语言为指针类型内置的操作符，不需要手动重载
        // 上面这一句代码等价于：
        // return *(data_ + index);
        // 解释如下：
        // data_ + index表示从data_这个地址处偏移index个T类型的大小，得到第index个元素的位置，或者说第index个元素的地址
        // *(data_ + index)表示解引用这个指针，得到第index个元素的值
    }

    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index]; // 这里的[]是C++内置数组的下标访问操作符，是C++语言为指针类型内置的操作符，不需要手动重载
        // 上面这一句代码等价于：
        // return *(data_ + index);
        // 解释如下：
        // data_ + index表示从data_这个地址处偏移index个T类型的大小，得到第index个元素的位置，或者说第index个元素的地址
        // *(data_ + index)表示解引用这个指针，得到第index个元素的值
    }

    // 容量与大小操作
    size_t size() const noexcept {
        return size_;
    }

    size_t capacity() const noexcept {
        return capacity_;
    }

    // 预留容量(不改变元素数量，仅确保容量足够)
    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) {
            return; // 当前容量已足够
        }

        // 分配新内存
        T* new_data = static_cast<T*>(operator new[](new_cap * sizeof(T)));
        // 复制旧元素到新内存
        for (size_t i = 0; i < size_; ++i) {
            new (new_data + i) T(data_[i]); // 拷贝构造
        }
        // 销毁旧元素并释放旧内存
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T(); // 显式调用析构函数
        }
        operator delete[](data_);
        // 更新指针和容量
        data_ = new_data;
        capacity_ = new_cap;
    }

    // 调整大小
    void resize(size_t new_size) {
        if (new_size > capacity_) {
            reserve(new_size); // 若新大小超过容量，则先扩容
        }
        if (new_size > size_) {
            // 新增元素：默认构造
            for (size_t i = size_; i < new_size; ++i) {
                new (data_ + i) T(); // 使用placement new构造默认对象
            }
        } else if (new_size < size_) {
            // 多余元素，显式调用析构函数
            for (size_t i = new_size; i < size_; ++i) {
                data_[i].~T(); // 显式调用析构函数
            }
        }
        size_ = new_size;
    }

    // 向尾部添加元素（拷贝版本）
    void push_back(const T& value) {
        if (size >= capacity_) {
            size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reserve(new_cap); // 扩容
        }
        // 在末尾位置构造新元素
        new (data_ + size_) T(value); // 使用placement new拷贝构造对象
        ++size_;
    }

    // 向尾部添加元素（移动版本）
    void push_back(T&& value) {
        if (size_ >= capacity_) {
            size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reserve(new_cap); // 扩容
        }
        // 在末尾位置构造新元素
        new (data_ + size_) T(std::move(value)); // 使用placement new移动构造对象
        ++size_;
    }

    // 移除尾部元素
    void pop_back() {
        if (size_ > 0) {
            size_--;
            data_[size_].~T(); // 显式调用析构函数
        }
    }
};