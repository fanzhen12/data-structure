#include <utility> // for std::pair，这一点之后会改，实现完全不依赖STL
#include <functional>
#include <cstddef> // size_t
#include <cstdlib> // for std::malloc, std::free

// ------- 自定义的独立节点结构体 ------- //
template <typename Key, typename T>
struct MyUnorderedMultimapNode {
    using value_type = std:::pair<const Key, T>;
    value_type value; // 键值对
    MyUnorderedMultimapNode* next;
    MyUnorderedMultimapNode* next;

    // 完美转发构造：支持左值/右值键值对
    template <typename K, typename V>
    MyUnorderedMultimapNode(K&& key, V&& val)
        : value(std::forward<K>(key), std::forward<V>(val)), prev(nullptr), next(nullptr) {}
        // 上一句代码中，调用了std::pair的构造函数，利用完美转发将key和val传递进去
    
    // 移动构造：转移键值对资源
    MyUnorderedMultimapNode(value_type&& val)
        : value(std::move(val)), prev(nullptr), next(nullptr) {}
    
        
    // 移动赋值：转移资源
    MyUnorderedMultimapNode& operator=(MyUnorderedMultimapNode&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value); // 这里调用std::pair的移动赋值
            prev = other.prev;
            next = other.next;
            other.prev = other.next = nullptr;
            // 对于上一句的深入探讨：
            // 连续赋值深层基础解析：
            // 1. 赋值运算符的返回值是一个引用，指向被赋值的对象本身（this指针）。
            // 2. 因此，连续赋值 a = b = c 实际上是 a = (b = c)。
            // 3. 首先执行 b = c，返回 b 的引用，然后将该引用赋值给 a。
        }
        return *this;
    }

    // 禁止拷贝构造/赋值，因为有指针成员
    MyUnorderedMultimapNode(const MyUnorderedMultimapNode&) = delete;
    MyUnorderedMultimapNode& operator=(const MyUnorderedMultimapNode&) = delete;
};

// 前置声明（提前声明MyUnorderedMultimap类，以便在iterator中使用）
template <typename Key, typename T, typename Hash, typename KeyEqual>
class MyUnorderedMultimap;

// ------- 自定义独立的迭代器类 ------- //
template <typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultimapIterator {
private:
    // 指向当前节点（某个桶中的节点）
    MyUnorderedMultimapNode<Key, T>* curr_; // 当前节点指针，即某个桶中的节点
    // 指向所属容器（需要访问容器的私有成员，通过友元关系实现）
    MyUnorderedMultimap<key, T, Hash, KeyEqual>* map_;

private:
    // 跨桶查找下一个非空桶的头节点
    void find_next_non_empty_bucket(size_t curr_bucket) {
        size_t i = curr_bucket + 1;
        while (i < map_->buckets_.size() && map_->buckets_[i] == nullptr) {
            ++i;
        }
        curr_ = (i < map_->buckets_.size()) ? map_->buckets_[i] : nullptr;
    }

public:
    // 迭代器类型定义（符合前向迭代器要求）
    // 为何一定要前向迭代器？因为unordered_multimap的迭代器至少要支持多次遍历
    // 为什么后向迭代器不行？因为unordered_multimap不支持逆序遍历？？？？？
    // ？？？？？
    using iterator_category = std::forward_iterator_tag; // 前向迭代器
    using value_type = std::pair<const Key, T>;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;

public:
    // 构造函数（仅允许容器类调用以创建迭代器）
    MyUnorderedMultimapIterator(
        MyUnorderedMultimapNode<Key, T>* curr,
        MyUnorderedMultimap<Key, T, Hash, KeyEqual>* map)
        : curr_(curr), map_(map) {}
    
    // 解引用
    reference operator*() const { return curr_->value; } // 返回当前节点的值引用
    pointer operator->() const { return &(curr_->value); } // 返回当前节点的值指针

    // 前置++
    MyUnorderedMultimapIterator& operator++() {
        if (curr_ && curr_->next != nullptr) {
            curr_ = curr_->next;
        } else {
            size_t curr_bucket = map_->get_bucket_idx(curr_->value.first);
            find_next_non_empty_bucket(curr_bucket);
        }
        return *this;
    }

    // 后置++
    MyUnorderedMultimapIterator operator++(int) {
        MyUnorderedMultimapIterator tmp = *this;
        ++(*this); // 等同于*this.operator++();，也等同于 this->operator++();
        return tmp;
    }

    // 相等性比较
    bool operator==(const MyUnorderedMultimapIterator& other) const {
        return curr_ == other.curr_ && map_ == other.map_;
    }
    bool operator!=(const MyUnorderedMultimapIterator& other) const {
        return !(*this == other);
    }

    // 允许容器类访问迭代器的私有成员
    friend class MyUnorderedMultimap<Key, T, Hash, KeyEqual>;
};

// ------- 自定义独立的无序多重映射类std_unordered_multimap ------- //
template <
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultimap {
private:
    // 桶数组：存储节点指针（使用独立的Node类型）
    MyUnorderedMultimapNode<Key, T>** buckets_;
    size_t bucket_count_; // 桶的数量
    size_t size_;         // 元素数量
    Hash hash_func_;      // 哈希函数
    KeyEqual equal_func_; // 键相等比较函数
    const float max_load_factor_ = 1.0f; // 最大负载因子

private:
    // 计算桶索引
    size_t get_bucket_idx(const Key& key) const {
        return hash_func_(key) % bucket_count_;
    }

    // 重哈希（扩容动态数组）
    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= bucket_count_) {
            return;
        }
        // 分配新桶数组
        auto* new_buckets = static_cast<MyUnorderedMultimapNode<Key, T>**>(
            std::malloc(new_bucket_count * sizeof(MyUnorderedMultimapNode<Key, T>*)));
        for (size_t i = 0; i < new_bucket_count; ++i) {
            new_buckets[i] = nullptr;
        }

        // 数据迁移
        for (size_t i = 0; i < bucket_count_; ++i) {
            auto* curr = buckets_[i];
            while (curr) {
                auto* next_node = curr->next; // 保存下一个节点

                // 计算新桶索引
                size_t new_idx = hash_func_(curr->value.first) % new_bucket_count;
                // 头插法
                curr->next = new_buckets[new_idx];
                if (new_buckets[new_idx]) {
                    new_buckets[new_idx]->prev = curr;
                }
                curr->prev = nullptr;
                new_buckets[new_idx] = curr;

                curr = next_node; // 继续处理下一个节点
            }
        }

        // 释放旧桶数组
        free(buckets_);
        buckets_ = new_buckets;
        bucket_count_ = new_bucket_count;
    }

public:
    // 迭代器类型别名（使用独立的迭代器类型）
    using iterator = MyUnorderedMultimapIterator<Key, T, Hash, KeyEqual>;
    using value_type = std::pair<const Key, T>;

    // 构造函数
    explicit MyUnorderedMultimap(
        size_t bucket_count = 16,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual()) : bucket_count_(bucket_count),
            size_(0),
            hash_func_(hash),
            equal_func_(equal) {
        buckets_ = static_cast<MyUnorderedMultimapNode<Key, T>**>(
            malloc(bucket_count_ * sizeof(MyUnorderedMultimapNode<Key, T>*)));
        for (size_t i = 0; i < bucket_count_; ++i) {
            buckets_[i] = nullptr;
        }
    }

    // 移动构造
    MyUnorderedMultimap(MyUnorderedMultimap&& other) noexcept
        : buckets_(other.buckets_), bucket_count_(other.bucket_count_),
          size_(other.size_),
          hash_func_(std::move(other.hash_func_),
          equal_func_(std::move(other.equal_func_)) {
        other.buckets_ = nullptr;
        other.bucket_count_ = 0;
        other.size_ = 0;
    }

    // 移动赋值
    MyUnorderedMultimap& operator=(MyUnorderedMultimap&& other) noexcept {
        if (this != &other) {
            clear();
            free(buckets_);

            buckets_ = other.buckets_;
            bucket_count_ = other.bucket_count_;
            size_ = other.size_;
            hash_func_ = std::move(other.hash_func_);
            equal_func_ = std::move(other.equal_func_);

            other.buckets_ = nullptr;
            other.bucket_count_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    // 析构函数
    ~MyUnorderedMultimap() {
        clear();
        if (buckets_) {
            free(buckets_);
        }
    }

    // 禁止拷贝（因为有指针成员）
    MyUnorderedMultimap(const MyUnorderedMultimap&) = delete;
    MyUnorderedMultimap& operator=(const MyUnorderedMultimap&) = delete;

    // 插入（完美转发）
    template <typename K, typename V>
    iterator insert(K&& key, V&& value) {
        if (size_ + 1 > buckets_count_ * max_load_factor_) {
            rehash(bucket_count_ * 2);
        }

        size_t bucket_idx = get_bucket_idx(key);
        auto* new_node = new MyUnorderedMultimapNode<Key, T>(
            std::forward<K>(key), std::forward<V>(value)
        );
        // 上面这句解释：使用完美转发构造节点，避免不必要的拷贝或移动

        // 头插法
        new_node->next = buckets_[bucket_idx];
        if (buckets_[bucket_idx]) {
            buckets_[bucket_idx]->prev = new_node;
        }
        buckets_[bucket_idx] = new_node;
        ++size_;

        return iterator(new_node, this);
    }

    // 重载insert，接受value_type右值
    iterator insert(value&& val) {
        return insert(std::move(val.first), std::move(val.second));
    }

    // 查找键的范围
    std::pair<iterator, iterator> equal_range(const Key& key) {
        size_t bucket_idx = get_bucket_idx(key);
        auto* curr = buckets_[bucket_idx];

        // 找到第一个匹配节点
        while (curr && !equal_func_(curr->value.first, key)) {
            curr = curr->next;
        }
        iterator begin_it(curr, this);

        // 找到范围终点
        auto* end_curr = curr;
        while (end_curr && equal_func_(end_curr->value.first, key)) {
            end_curr = end_curr->next;
        }
        iterator end_it(end_curr, this);

        return {begin_it, end_it};
    }
    
    // 按迭代器删除
    iterator erase(iterator pos) {
        if (pos == end()) return end();

        auto* to_delete = pos.curr_;
        iterator next_it(to_delete->next, this);

        if (to_delete->prev) {
            to_delete->prev->next = to_delete->next;
        } else {
            size_t bucket_idx = get_bucket_idx(to_delete->value.first);
            next_it.find_next_non_empty_bucket(bucket_idx);
        }

        delete to_delete;
        --size_;

        return next_it;
    }

    // 迭代器接口
    iterator begin() {
        for (size_t i = 0; i < bucket_count_; ++i) {
            if (buckets_[i]) {
                return iterator(buckets_[i], this);
            }
        }
        return end();
    }

    iterator end() {
        return iterator(nullptr, this);
    }

    // 基础属性
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // 清空元素
    void clear() {
        for (size_t i = 0; i < bucket_count_; ++i) {
            auto* curr = buckets_[i];
            while (curr) {
                auto* next_node = curr->next;
                delete curr;
                curr = next_node;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    // 允许迭代器访问容器的私有成员（桶数组、桶数量等）
    friend class MyUnorderedMultimapIterator<Key, T, Hash, KeyEqual>;
};