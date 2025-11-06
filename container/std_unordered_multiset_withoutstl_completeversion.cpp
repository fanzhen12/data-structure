// reference: https://www.doubao.com/chat/27669708128781826
#include <cstddef> // 用于size_t、ptrdiff_t等类型定义
#include <cstdlib> // 用于malloc、free等内存管理函数
#include <utility>    // std::move、std::forward
#include <type_traits> // std::is_nothrow_move_constructible（可选）

// 前置声明：哈希函数默认实现（脱离std::hash）
template <typename T>
struct DefaultHash;

// 前置声明：相等比较器默认实现（脱离std::equal_to）
template <typename T>
struct DefaultEqual;

// ------- 节点结构：双向链表节点 ------- //
template <typename T>
struct HashNode {
    T data; // 存储的数据
    HashNode* next; // 后继节点
    HashNode* prev; // 前驱节点
    size_t bucket_idx; // 所在桶的索引

    // 复制构造函数（左值）
    explicit HashNode(const T& val, size_t idx) 
        : data(val), prev(nullptr), next(nullptr), bucket_idx(idx) {}

    // 移动构造函数（右值）
    explicit HashNode(T&& val, size_t idx)
        : data(std::move(val)), prev(nullptr), next(nullptr), bucket_idx(idx) {}

    // 原地构造数据（完美转发参数）
    template <typename... Args>
    explicit HashNode(Args&&... args, size_t idx)
        : data(std::forward<Args>(args)...), prev(nullptr), next(nullptr), bucket_idx(idx) {}

};

// ------- 自定义unordered_multiset ------- //
template <typename T,
          typename Hash = DefaultHash<T>,
          typename KeyEqual = DefaultEqual<T>>
class MyUnorderedMultiSet {
public:
    // 类型定义
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    // 迭代器（前向迭代器）
    class iterator {
    private:
        HashNode<T>* m_node; // 指向当前节点的指针
        MyUnorderedMultiSet* m_container; // 指向所属容器的指针
        friend class MyUnorderedMultiSet;

    public:
        using iterator_category = std::forward_iterator_tag; // 这里用到了std中的标签，
        // 什么是std中的标签呢？标签是一种空类型，用于标识迭代器的类别，比如前向迭代器、双向迭代器等。
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t;

        iterator() : m_node(nullptr), m_container(nullptr) {}
        iterator(HashNode<T>* node, MyUnorderedMultiSet* container)
            : m_node(node), m_container(container) {}

        // 移动迭代器
        iterator(iterator&& other) noexcept
            : m_node(other.m_node), m_container(other.m_container) {
            other.m_node = nullptr;
            other.m_container = nullptr;
        }

        reference operator*() const {
            return m_node->data;
        }

        pointer operator->() const {
            return &(m_node->data);
        }
        // 地址就地址吧

        // 前置递增
        iterator& operator++() {
            if (m_node->next) {
                m_node = m_node->next;
            } else {
                size_t current_bucket = m_node->bucket_idx;
                for (size_t i = current_bucket + 1; i < m_container->m_buckets.size(); ++i) {
                    if (m_container->m_buckets[i]) {
                        m_node = m_container->m_buckets[i]; // 这是桶的头节点
                        return *this;
                    }
                }
                m_node = nullptr; // 到达容器末尾
            }
            return *this;
        }

        // 后置递增
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this); // 这里的++调用了上面的前置递增
            return tmp;
        }

        // 比较操作符
        bool operator==(const iterator& other) const {
            return m_node == other.m_node && m_container == other.m_container;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other); // 利用上面的==操作符
        }
    };

    // 常量迭代器（与iterator类似，只读，返回const引用）
    class const_iterator {
        // 略，类似iterator，只是返回const引用
    };

    // 构造函数
    explicit MyUnorderedMultiSet(size_type bucket_count = 16,
                                 const Hash& hash = Hash(),
                                 const KeyEqual& key_eq = KeyEqual())
        : m_bucket_count(bucket_count), m_size(0), m_hash(hash), m_key_eq(key_eq), m_max_load_factor(0.7f) {
        m_buckets = allocate_buckets(m_bucket_count);
    }

    // 移动构造函数（转移资源，不复制节点）
    MyUnorderedMultiSet(MyUnorderedMultiSet&& other) noexcept
        : m_buckets(other.m_buckets),
          m_bucket_count(other.m_bucket_count),
          m_size(other.m_size), // std::move 的作用是触发对象的移动构造 / 赋值，仅对具有资源的对象有意义；而指针和基本类型的 “移动” 本质是浅拷贝，无需 std::move。
          m_hash(std::move(other.m_hash)),
          m_key_eq(std::move(other.m_key_eq)),
          m_max_load_factor(other.m_max_load_factor) {
        // 将other置于有效但空状态
        other.m_buckets = nullptr;
        other.m_bucket_count = 0;
        other.m_size = 0;
    }

    // 移动赋值运算符
    // 解释一下返回值：
    //this 指针：指向当前对象的指针（地址），类型为 MyUnorderedMultiSet*。
    //*this：对 this 指针解引用，得到当前对象本身（不是副本，是对象在内存中的实际存在）。
    //返回值类型 MyUnorderedMultiSet&：表示返回的是 “当前对象的引用”（即指向当前对象的一个别名）
    // 返回引用的好处是可以支持链式赋值（a = b = c;），并且避免了不必要的对象拷贝，提高性能。

    //2. “对象本身” 与 “对象的引用” 的核心区别
    //（1）内存层面
    //对象本身：是一块实际的内存区域，存储了对象的所有成员变量（如 m_buckets、m_hash 等）。
    //对象的引用：是对象的 “别名”，本身不占用额外内存（或仅在编译层面作为标识），它直接关联到对象本身的内存区域。
    //（2）操作语义层面
    //操作对象本身：如果直接返回对象本身（即返回值类型为 MyUnorderedMultiSet 而非 MyUnorderedMultiSet&），会触发对象的复制（调用复制构造函数），产生一个新的对象副本。这会导致额外的性能开销（尤其是大对象），且违背移动赋值 “高效转移资源” 的设计初衷。
    //操作对象的引用：返回对象的引用（MyUnorderedMultiSet&）时，不会产生任何复制，返回的是对原对象的直接关联。此时，对返回值的操作会直接作用于原对象。

    MyUnorderedMultiSet& operator=(MyUnorderedMultiSet&& other) noexcept {
        if (this != &other) {
            clear(); // 释放当前资源
            free(m_buckets);

            // 转移源对象资源
            m_buckets = other.m_buckets;
            m_bucket_count = other.m_bucket_count;
            m_size = other.m_size;
            m_hash = std::move(other.m_hash);
            m_key_eq = std::move(other.m_key_eq);
            m_max_load_factor = other.m_max_load_factor;

            // 源对象置空
            other.m_buckets = nullptr;
            other.m_bucket_count = 0;
            other.m_size = 0;
        }
        return *this; // 返回当前对象的引用，this是指针，*this是对象本身，对象本身和对象本身的引用有什么含义？
        // 对象本身表示对象的值，而对象本身的引用表示对该对象的引用，可以用于链式赋值等场景。
    }

    // 禁止复制构造函数和复制赋值运算符
    MyUnorderedMultiSet(const MyUnorderedMultiSet&) = delete;
    MyUnorderedMultiSet& operator=(const MyUnorderedMultiSet&) = delete;

    // 析构函数
    ~MyUnorderedMultiSet() {
        clear();
        if (m_buckets) {
            free(m_buckets); // 使用free是因为allocate_buckets中使用了malloc分配内存
        }
    }

    // 1. 插入左值（复制语义）
    iterator insert(const T& val) {
        return emplace_node(val); // 复用emplace_node，传入左值
        // 这里为什么不能使用 std::move(val) 呢？
    }

    // 2. 插入右值（移动语义）
    iterator insert(T&& val) {
        return emplace_node(std::move(val)); // 复用emplace_node，传入右值
    }

    // 3. 原地构造元素（完美转发，避免临时对象）
    template <typename... Args>
    iterator emplace(Args&&... args) {
        // 先构造临时对象计算哈希（实际STL可能优化此步骤以避免临时对象）
        T tmp(std::forward<Args>(args)...); // 完美转发参数构造临时对象，这里调用了T的构造函数，至于是哪个构造函数取决于Args的类型和数量
        size_t hash_val = m_hash(tmp);
        size_t bucket_idx = hash_val % m_bucket_count;

        // 检查重哈希
        if (load_factor() > m_max_load_factor) {
            rehash(m_bucket_count * 2);
            bucket_idx = hash_val % m_bucket_count; // 重新计算桶索引
        }

        // 直接在节点中构造函数（完美转发参数）
        HashNode<T>* new_node = new HashNode<T>(std::forward<Args>(args)..., bucket_idx); // 调用了HashNode的完美转发构造函数，
        // 如果HashNode没有定义完美转发构造函数，则会报错
        link_node(new_node, bucket_idx);
        ++m_size;

        return iterator(new_node, this);
    }

    // 计数元素
    size_type count(const T& val) const {
        if (empty()) return 0;
        siz_t bucket_idx = m_hash(val) % m_bucket_count;
        size_type cnt = 0;
        for (HashNode<T>* p = m_buckets[bucket_idx]; p; p = p->next) {
            if (m_key_eq(p->data, val)) {
                ++cnt;
            }
        }
        return cnt;
    }

    // 查找元素
    iterator find(const T& val) {
        if (empty()) {
            return end();
        }
        size_t bucket_idx = m_hash(val) % m_bucket_count;
        for (HashNode<T>* p = m_buckets[bucket_idx]; p; p = p->next) {
            if (m_key_eq(p->data, val)) {
                return iterator(p, this);
            }
        }
        return end();
    }

    // 按值删除，返回删除个数（因为是multiset，可能删除多个）
    size_type erase(const T& val) {
        if (empty()) return 0;
        size_t bucket_idx = m_hash(val) % m_bucket_count;
        size_type cnt = 0;
        HashNode<T>* p = m_buckets[bucket_idx];
        while (p) {
            HashNode<T>* next = p->next;
            if (m_key_eq(p->data, val)) {
                unlink_node(p, bucket_idx);
                delete p;
                --m_size;
                ++cnt;
            }
            p = next;
        }
        return cnt;
    }

    // 重哈希
    void rehash(size_type new_bucket_count) {
        if (new_bucket_count <= m_bucket_count) {
            return; // 新桶数必须大于当前桶数
        }
        HashNode<T>** new_buckets = allocate_buckets(new_bucket_count);
        // 迁移节点到新桶
        for (size_t i = 0; i < m_bucket_count; ++i) {
            HashNode<T>* p = m_buckets[i];
            while (p) {
                HashNode<T>* next = p->next;
                size_t new_idx = m_hash(p->data) % new_bucket_count;
                p->bucket_idx = new_idx;
                // 插入到新桶头部
                p->prev = nullptr;
                p->next = new_buckets[new_idx];
                if (new_buckets[new_idx]) {
                    new_buckets[new_idx]->prev = p;
                }
                new_buckets[new_idx] = p;
                p = next;
            }
        }
        // 替换桶数组
        free(m_buckets);
        m_buckets = new_buckets;
        m_bucket_count = new_bucket_count;
    }

    // 清空容器
    void clear() {
        for (size_t i = 0; i < m_bucket_count; ++i) {
            HashNode<T>* p = m_buckets[i];
            while (p) {
                HashNode<T>* next = p->next;
                delete p;
                p = next;
            }
            m_buckets[i] = nullptr;
        }
        m_size = 0;
    }

    // 迭代器相关
    iterator begin() {
        if (empty()) {
            return end();
        }
        for (size_t i = 0; i < m_bucket_count; ++i) {
            if (m_buckets[i]) {
                return iterator(m_buckets[i], this); // 返回第一个非空桶的头节点
            }
        }
        return end();
    }

    iterator end() {
        return iterator(nullptr, this); // 结束迭代器，指向空节点
    }

    // 容量相关
    size_type size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
    float load_factor() const {
        return m_bucket_count == 0 ? 0.0f : static_cast<float>(m_size) / m_bucket_count;
    }
    float max_load_factor() const {
        return m_max_load_factor;
    }
    void max_load_factor(float lf) {
        m_max_load_factor = lf;
    }

private:
    HashNode<T>** m_buckets = nullptr; // 桶数组，指向每个桶的头节点指针
    size_type m_bucket_count; // 桶数量
    size_type m_size; // 元素数量
    Hash m_hash; // 哈希函数对象
    KeyEqual m_key_eq; // 键比较函数对象
    float m_max_load_factor; // 最大负载因子

    // 辅助函数：分配并初始化桶数组
    HashNode<T>** allocate_buckets(size_type n) {
        if (n == 0) return nullptr;
        auto buckets = static_cast<HashNode<T>**>(malloc(n * sizeof(HashNode<T>*))); // buckets是指向指针的指针，是一个指针数组，
        // 在这里的含义是：每个元素都是一个HashNode<T>*类型的指针，指向对应桶的头节点
        for (size_type i = 0; i < n; ++i) {
            buckets[i] = nullptr; // 初始化每个桶为空
        }
        return buckets;
    }

    // 辅助函数：将节点插入桶链表头部
    void link_node(HashNode<T>* node, size_type bucket_idx) {
        if (m_buckets[bucket_idx]) {
            m_buckets[bucket_idx]->prev = node;
            node->next = m_buckets[bucket_idx];
        }
        m_buckets[bucket_idx] = node;
        node->prev = nullptr; // 这一句需要吗？答：需要，确保新节点的前驱为空
    }

    // 辅助函数：将节点从桶链表中移除
    void unlink_node(HashNode<T>* node, size_type bucket_idx) {
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            m_buckets[bucket_idx] = node->next; // 更新桶头指针
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
    }

    // 辅助函数：插入节点（统一处理左值和右值）
    template <typename U>
    iterator emplace_node(U&& val) {
        if (m_bucket_count == 0) {
            m_bucket_count = 16;
            m_buckets = allocate_buckets(m_bucket_count);
        }

        // 计算哈希和桶索引
        size_t hash_val = m_hash(val);
        size_t bucket_idx = hash_val % m_bucket_count;

        // 检查重哈希
        if (load_factor() > m_max_load_factor) {
            rehash(m_bucket_count * 2);
            bucket_idx = hash_val % m_bucket_count; // 重新计算桶索引
        }

        // 构造节点（根据val是左值还是右值，调用复制/移动构造函数）
        HashNode<T>* new_node = new HashNode<T>(std::forward<U>(val), bucket_idx); // 完美转发，
        // 完美转发的两个条件：
        // 1. 模板参数U是通过类型推导得到的，这是转发引用的必要条件
        // 2. 传递给构造函数的参数val是通过std::forward<U>(val)传递的
        link_node(new_node, bucket_idx);
        ++m_size;

        return iterator(new_node, this);
    }
};

// 工具类
// 哈希函数实现
template <typename T>
struct DefaultHash {
    size_t operator()(const T& val) const {
        return static_cast<size_t>(val); // 整数哈希
    }
};

// 字符串哈希特化
template <>
struct DefaultHash<const char*> {
    size_t operator()(const char* str) const {
        size_t hash = 5381; // 初始值
        while (*str) {
            hash = ((hash << 5) + hash) + static_cast<size_t>(*str++);
            // 上一句代码的*str++是什么意思？
            // *str++表示先取str指向的字符，然后str指针自增，指向下一个字符
            // str指向的字符指的是字符串中的当前字符，自增后指向下一个字符
        }
        return hash;
    }
};

// 相等比较器实现
template <typename T>
struct DefaultEqual {
    bool operator()(const T& a, const T& b) const {
        return a == b;
    }
};

// 字符串相等比较特化
template <>
struct DefaultEqual<const char*> {
    bool operator()(const char* a, const char* b) const {
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
            // 指针的++是什么意思？答：指针的++表示指针向后移动一个位置，指向下一个内存地址
            // 如果这是一个int类型的指针，则会移动4个字节（假设int是4字节）
            // 如果这是一个char类型的指针，则会移动1个字节
            // 也就是说：指针的++会根据指针类型移动相应的字节数
        }
        return *a == *b;
    }
};