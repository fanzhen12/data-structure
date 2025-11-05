#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <iterator>

template <typename T,
          typename Hash = std::hash<T>,
          typename KeyEqual = std::equal_to<T>>
class MyUnorderedMultiSet {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    // 迭代器实现（双向迭代器）
    class iterator {
    private:
        typename std::vector<std::list<T>>::iterator m_bucket_it; // 指向当前桶的迭代器
        typename std::list<T>::iterator m_elem_it; // 指向当前元素的迭代器
        MyUnorderedMultiSet* m_container; // 关联容器

    public:
        using iterator_category = std::forward_iterator_tag; // 前向迭代器
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t; // 用于指针运算

        // 构造函数
        iterator() : m_bucket_it(nullptr), m_elem_it(), m_container(nullptr) {}
        iterator(typename std::vector<std::list<T>>::iterator bucket_it,
                 typename std::list<T>::iterator elem_it,
                 MyUnorderedMultiSet* container)
            : m_bucket_it(bucket_it), m_elem_it(elem_it), m_container(container) {}

        // 解引用
        reference operator*() const {
            return *m_elem_it; // m_elem_it的类型是std::list<T>::iterator，不是指针，是迭代器，std里面的list迭代器重载了*操作符，返回引用
        }

        pointer operator->() const {
            return &(*m_elem_it); // 返回元素地址
            // 这里的*操作符是调用上面的operator*函数，返回引用，再取地址
            // 举一个例子：
            // MyUnorderedMultiSet<int>::iterator it = ...;
            // int* pValue = it.operator->(); // 获取值的地址
        }

        // 前置递增
        iterator& operator++() {
            ++m_elem_it;
            // 若当前桶内元素遍历完毕，寻找下一个非空桶
            if (m_elem_it == m_bucket_it->end()) {
                ++m_bucket_it;
                while (m_bucket_it != m_container->buckets.end() && m_bucket_it->empty()) {
                    ++m_bucket_it;
                }
                // 若找到非空桶，指向其第一个元素
                if (m_bucket_it != m_container->buckets.end()) {
                    m_elem_it = m_bucket_it->begin();
                }
            }
            return *this;
        }

        // 后置递增
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // 比较操作符
        bool operator==(const iterator& other) const {
            return m_bucket_it == other.m_bucket_it && m_elem_it == other.m_elem_it;
            // 为什么这里不写作：return *this == other;  因为这样会导致无限递归调用，最终引发栈溢出错误。
            // 解释如下：
            // 当我们编写a == b时（其中a和b是iterator对象），编译器会自动转换为对a.operator==(b)的调用（这一句很重要，对于理解透彻很重要）
            // 那么，同理，如果在operator==的实现中写成return *this == other;，这实际上又会调用operator==函数本身，因为
            // return *this == other;这一句等价于return this->operator==(other); 也等价于(*this).operator==(other)这里自身调用自身，无限递归
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        
    };

    // 构造函数
    explicit MyUnorderedMultiSet(size_type bucket_count = 16,
                                 const Hash& hash = Hash(), // Hash()调用了谁的默认构造函数？答：调用了Hash模板参数的默认构造函数，即std::hash<T>的默认构造函数
                                 const KeyEqual& key_eq = KeyEqual()) // KeyEqual()调用了谁的默认构造函数？答：调用了KeyEqual模板参数的默认构造函数，即std::equal_to<T>的默认构造函数
        : m_buckets(bucket_count), m_size(0), m_hash(hash), m_key_eq(key_eq), m_max_load_factor(0.7f) {}

    // 插入元素
    iterator insert(const T& val) {
        if (m_buckets.empty()) {
            m_buckets.resize(16); // 初始桶数量
        }
        // 负载因子超限，进行rehash
        if (load_factor() > m_max_load_factor) {
            rehash(m_buckets.size() * 2); // 桶数翻倍
        }

        // 计算哈希值和桶索引
        size_type hash_val = m_hash(val);
        size_type bucket_idx = hash_val % m_buckets.size();

        // 插入元素到对应桶
        m_buckets[bucket_idx].push_back(val); // 调用了std::list的push_back方法
        ++m_size; // 可以使用m_size++，也可以使用++m_size，两者在这里没有区别

        // 返回插入元素的迭代器
        auto bucket_it = m_buckets.begin() + bucket_idx;
        return iterator(bucket_it, std::prev(bucket_it->end()), this); // 这一句解释：
        // std::prev(bucket_it->end())返回桶中最后一个元素的迭代器
    }

    // 计数：返回等于val的元素个数
    size_type count(const T& val) const {
        if (m_buckets.empty()) {
            return 0;
        }
        size_type hash_val = m_hash(val);
        size_type bucket_idx = hash_val % m_buckets.size();
        const auto& bucket = m_buckets[bucket_idx];

        size_type cnt = 0;
        for (const auto& elem : bucket) {
            if (m_key_eq(elem, val)) {
                ++cnt;
            }
        }
        return cnt;
    }

    // 删除所有等于val的元素，返回删除个数
    size_type erase(const T& val) {
        if (m_buckets.empty()) {
            return 0;
        }

        size_type hash_val = m_hash(val);
        size_type bucket_idx = hash_val % m_buckets.size();
        auto& bucket = m_buckets[bucket_idx]; // bucket的类型是std::list<T>&，bucket.erase调用的是std::list的erase方法

        size_type cnt = 0;
        auto it = bucket.begin();
        while (it != bucket.end()) {
            if (m_key_eq(*it, val)) {
                it = bucket.erase(it); // 调用了std::list的erase方法，返回被删除元素的下一个元素的迭代器，而不是MyUnorderedMultiSet的erase方法
                ++cnt;
                --m_size;
            } else {
                ++it;
            }
        }
        return cnt;
    }

    // 通过迭代器删除，返回下一个迭代器
    iterator erase(iterator pos) {
        if (pos == end()) {
            return end();
        }
        iterator next = pos;
        ++next; // 获取下一个迭代器
        pos.m_bucket_it->erase(pos.m_elem_it); // 调用了std::list的erase方法，删除当前元素
        --m_size;
        return next;
    }

    // 重哈希：重新分配桶并迁移所有元素
    void rehash(size_type new_bucket_count) {
        if (new_bucket_count <= m_buckets.size()) {
            return; // 新桶数必须大于当前桶数
        }

        std::vector<std::list<T>> new_buckets(new_bucket_count);
        // 迁移旧桶元素到新桶
        for (auto& bucket : m_buckets) {
            for (auto& elem : bucket) {
                size_type hash_val = m_hash(elem);
                size_type new_idx = hash_val % new_bucket_count;
                new_buckets[new_idx].push_back(elem);
            }
        }
        m_buckets.swap(new_buckets); // 交换新旧桶
    }

    // 负载因子（元素总数/桶数）
    float load_factor() const {
        return m_buckets.empty() ? 0.0f : static_cast<float>(m_size) / m_buckets.size();
    }

    // 最大负载因子
    float max_load_factor() const {
        return m_max_load_factor;
    }
    void max_load_factor(float lf) {
        m_max_load_factor = lf;
    }

    // 迭代器起始和结束
    iterator begin() {
        auto it = m_buckets.begin();
        // 寻找第一个非空桶
        while (it != m_buckets.end() && it->empty()) {
            ++it;
        }
        return (it != m_buckets.end()) ? iterator(it, it->begin(), this) : end();
    }
    iterator end() {
        return iterator(m_buckets.end(), typename std::list<T>::iterator(), this);
    }

    // 容器大小
    size_type size() const {
        return m_size;
    }

    bool empty() const {
        return m_size == 0;
    }

private:
    std::vector<std::list<T>> m_buckets; // 桶数组，每个桶是一个链表
    size_type m_size; // 元素数量
    Hash m_hash; // 哈希函数对象
    KeyEqual m_key_eq; // 键比较函数对象
    float m_max_load_factor; // 最大负载因子
};