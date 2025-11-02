// reference: https://www.doubao.com/chat/26923522862689282
#include <cstddef> // size_t
#include <cstring> // 用于字符串哈希计算
#include <cmath> // 用于浮点数哈希计算

// 自定义相等性比较：默认使用==运算符
template <typename T>
struct MyEqual {
    bool operator()(const T& a, const T& b) const {
        return a == b;
    }
};

// 自定义哈希函数：基础模板，需针对不同类型进行特化
template <typename T>
struct MyHash;

// 针对int类型的哈希函数特化
template <>
struct MyHash<int> {
    size_t operator()(int val) const {
        // 简单哈希：直接返回值，对于小整数效果较好
        return static_cast<size_t>(val);
    }
};

// 针对unsigned int类型的哈希函数特化
template <>
struct MyHash<unsigned int> {
    size_t operator()(unsigned int val) const {
        return static_cast<size_t>(val);
    }
};

// 针对long类型的哈希函数特化
template <>
struct MyHash<long> {
    size_t operator()(long val) const {
        // 处理负数：转为无符号值
        return static_cast<size_t>(val ^ (val >> 32)); // 如何理解这一句：
        // 这里假设long是64位，size_t也是64位。将long的高32位和低32位进行异或操作，混合高低位的信息，得到一个更均匀分布的哈希值。
    }
};

// 针对double类型的哈希函数特化
template <>
struct MyHash<double> {
    size_t operator()(double val) const {
        // 将double的二进制标识转为size_t，作为哈希值
        union {
            double d;
            unsigned long long u;
        } u{val};
        return static_cast<size_t>(u.u ^ (u.u >> 32)); // 如何理解这一句：
        // 这里假设unsigned long long是64位，size_t也是64位。将u.u的高32位和低32位进行异或操作，混合高低位的信息，得到一个更均匀分布的哈希值。
        // u.u是什么意思？u是一个联合体变量，u.u表示访问联合体中的成员u，它是一个unsigned long long类型，存储了double的二进制表示。
        // 为什么联合体u有两个成员？因为联合体的所有成员共享同一块内存空间，可以通过不同成员以不同类型访问同一块内存。
        // 其中的d成员用于存储double值，u成员用于以unsigned long long类型访问该double的二进制表示。
    }
};

// 针对const char*类型的哈希函数特化（C风格字符串）
template <>
struct MyHash<const char*> {
    size_t operator()(const char* str) const {
        if (str == nullptr) {
            return 0;
        }
        // 经典的字符串哈希算法：djb2
        size_t hash = 5381;
        int c;
        while ((c = *str++)) { // *str++如何理解？
            // 这里的*str++表示先解引用获取当前字符，然后将指针移动到下一个字符位置。
            // 等价于：
            // c = *str; c是当前字符，为什么用int类型？因为需要存储EOF等特殊值。
            // str = str + 1; 将指针移动到下一个字符。指针加1会根据指针类型自动移动相应的字节数。
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        return hash;
    }
};

// 前向声明：链表节点和链表类
template <typename T>
struct Node;

template <typename T>
class LinkedList;

// ------- 链表节点：存储元素和指向下一个节点的指针 ------- //
template <typename T>
struct Node {
    T data;
    Node* next;
    Node(const T& val) : data(val), next(nullptr) {}
};

// ------- 自定义单向链表类（用于桶的实现） ------- //
template <typename T>
class LinkedList {
private:
    Node<T>* head;
    size_t len;

public:
    LinkedList() : head(nullptr), len(0) {}

    // 析构函数：释放所有节点内存
    ~LinkedList() {
        clear();
    }

    // 插入元素到链表尾部（不检查重复，由外部保证）
    void push_back(const T& val) {
        Node<T>* new_node = new Node<T>(val);
        if (!head) {
            head = new_node;
        } else {
            Node<T>* cur = head;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = new_node;
        }
        len++;
    }

    // 查找元素并删除（返回是否删除成功）
    template <typename KeyEqual>
    bool erase(const T& val, KeyEqual key_eq) {
        if (!head) {
            return false;
        }
        Node<T>* prev = nullptr;
        Node<T>* cur = head;

        // 遍历链表查找元素
        while (cur) {
            if (key_eq(cur->data, val)) {
                // 找到元素，调整指针并释放节点
                // 如果是头节点
                if (!prev) {
                    head = cur->next;
                } else {
                    prev->next = cur->next;
                }
                delete cur;
                len--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false; // 未找到元素
    }

    // 查找元素（返回是否存在）
    template <typename KeyEqual>
    bool contains(const T& val, KeyEqual key_eq) const {
        Node<T>* cur = head;
        while (cur) {
            if (key_eq(cur->data, val)) {
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    // 清空链表
    void clear() {
        Node<T>* cur = head; // 这一句有函数的调用吗？
        // 没有，这里只是声明了一个指针变量cur，用于遍历链表。
        Node<T>* tmp; // 这一句有函数的调用吗？
        // 没有，这里只是声明了一个指针变量tmp，用于在删除节点时临时保存当前节点，防止丢失对下一个节点的访问。
        while (cur) {
            Node<T>* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        head = nullptr;
        len = 0;
    }

    // 获取链表长度
    size_t size() const { return len; }

    // 链表是否为空
    bool empty() const { return len == 0; }

    // 遍历链表并执行回调函数，用于重hash时迁移元素
    template <typename Func>
    void for_each(Func func) const {
        Node<T>* cur = head;
        while (cur) {
            func(cur->data);
            cur = cur->next;
        }
    }

};

// ------- 自定义动态数组（作为桶数组，存储多个头指针，这些指针是链表的头指针，1个链表就是一个桶） ------- //
template <typename T>
class DynamicArray {
private:
    T* data; // 数据指针
    size_t cap; // 容量
    size_t len; // 实际元素数

public:
    // 构造函数：初始容量为n，默认构造每个元素
    DynamicArray(size_t n) : cap(n), len(n) {
        data = new T[cap]; // 解释：
        // new T[cap]表示动态分配一个包含cap个T类型元素的数组
        // 返回值是指向数组首元素的指针，赋值给data指针变量
    }

    // 析构函数
    ~DynamicArray() {
        delete[] data;
    }

    // 禁止拷贝（避免浅拷贝导致双重释放和指针悬空）
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    // 交换两个动态数组的数据
    void swap(DynamicArray& other) {
        T* temp_data = data;
        data = other.data;
        other.data = temp_data;

        size_t temp_cap = cap;
        cap = other.cap;
        other.cap = temp_cap;

        size_t temp_len = len;
        len = other.len;
        other.len = temp_len;
    }

    // 访问元素（下标操作）
    T& operator[](size_t index) {
        return data[index]; // 这里的[]操作符是C++中
        // 指针的偏移加上解引用的简写。
    }

    const T& operator[](size_t idx) const {
        return data[idx];
    }

    // 获取容量
    size_t capacity() const {
        return cap;
    }

    // 获取长度
    size_t size() const {
        return len;
    }
};

// 到目标位置已经自定义了链表节点，链表以及动态数组，注意：
// 动态数组中存储的类型不是指针，而是链表对象本身，为什么呢？
// 因为链表对象已经包含了头指针head，可以直接通过head来方便地对链表进行遍历

// ------- 自定义哈希集合类 ------- //
template <typename T,
          typename Hash = MyHash<T>,
          typename KeyEqual = MyEqual<T>>
class MyUnordered {
private:
    using Bucket = LinkedList<T>; // 每个桶是自定义链表
    DynamicArray<Bucket> buckets_; // 桶的动态数组
    size_t size_; // 元素总数
    Hash hasher_; // 哈希函数对象
    KeyEqual key_eq_; // 相等性比较对象
    float max_load_factor_; // 最大负载因子

private:
    // 计算桶索引
    size_t get_bucket_index(const T& key) const {
        return hasher_(key) % buckets_.capacity();
    }

    // 重哈希：扩容并且迁移元素
    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= buckets_.capacity()) {
            return; // 此时不需要扩容
        }
        DynamicArray<Bucket> new_buckets(new_bucket_count);
        // 上一句代码的解释：调用DynamicArray的DynamicArray(size_t n) 构造函数

        // 迁移所有元素到新桶
        for (size_t i = 0; i < buckets_.capacity(); ++i) {
            const Bucket& old_bucket = buckets_[i];
            old_bucket.for_each([&](const T& elem) {
                size_t new_idx = hasher_(elem) % new_bucket_count;
                new_buckets[new_idx].push_back(elem);
            });
        }

        // 交换新旧桶数组
        buckets_.swap(new_buckets);
    }

public:
    // 构造函数
    explicit MyUnordered(size_t bucket_count = 11, float max_load_factor = 1.0f) 
        : buckets_(bucket_count), size_(0), max_load_factor_(max_load_factor) {}

    // 析构函数：依赖DynamicArray和LinkedList的析构函数自动释放内存
    ~MyUnordered() = default;

    // 插入元素（返回是否成功）
    bool insert(const T& key) {
        // 检查是否需要重哈希
        if (load_factor() > max_load_factor_) {
            rehash(buckets_.capacity() * 2 + 1); // 新桶数：原大小*2+1（保证质数特性）
        }

        size_t idx = get_bucket_index(key);
        Bucket& bucket = buckets_[idx];

        // 检查元素是否已经存在
        if (bucket.contains(key, key_eq)) {
            return false; // 插入失败，因为元素已经存在
        }

        // 插入新的元素
        bucket.push_back(key);
        size_++;
        return true;
    }

    // 删除元素（返回是否成功）
    bool erase(const T& key) {
        size_t idx = get_bucket_index(key);
        Bucket& bucket = buckets_[idx];

        // 尝试删除元素
        if (bucket.erase(key, key_eq_)) { // 调用LinkedList的erase()方法
            size_--;
            return true;
        }
        return false;
    }

    // 查找元素（返回是否成功）
    bool find(const T& key) const {
        size_t idx = get_bucket_index(key);
        const Bucket& bucket = buckets_[idx];
        return bucket.contains(key, key_eq_);
    }

    // 清空集合
    void clear() {
        for (size_t i = 0; i < buckets_.capacity(); ++i) {
            buckets_[i].clear();
        }
        size_ = 0;
    }

    // 获取元素数量
    size_t size() const {
        return size_;
    }

    // 检查链表是否为空
    bool empty() const {
        return size_ == 0;
    }

    float load_factor() const {
        return static_cast<float>(size_) / buckets_.capacity();
    }

    // 获取当前桶的数量
    size_t bucket_count() const {
        return buckets_.capacity();
    }
};