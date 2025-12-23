#include <vector>
#include <list>
#include <utility>
#include <functional>
#include <cstddef>

// 提前声明容器类（供迭代器类引用）
template <typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultiMap;

// ------- 1. 独立的迭代器类 ------- //
template <typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultiMapIterator {
public:
    // 适配 STL 迭代器标准
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const Key, T>;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

public:
    // 构造函数
    MyUnorderedMultiMapIterator(
        MyUnorderedMultiMap<Key, T, Hash, KeyEqual>* map,
        size_type bucket_idx,
        typename std::list<value_type>::iterator list_it
    ) : map_(map), bucket_idx_(bucket_idx), list_it_(list_it) {}

    // 解引用操作符
    reference operator*() const { return *list_it_; } // 返回当前元素的引用
    pointer operator->() const { return &(*list_it_); } // 返回当前元素的指针
    /*
    实现逻辑：
    第一步：*list_it_ 解引用链表迭代器，拿到元素的引用；
    第二步：& 取该引用的地址，得到元素的指针；
    第三步：返回这个指针。

    C++ 规定：it->member 会被编译器自动解析为 (it.operator->())->member。
    即：先调用迭代器的 operator->() 拿到元素指针，再通过这个指针访问 member 成员 —— 这也是 operator->() 必须返回指针的原因。
    */

    // 前置++：移动到下一个元素（包含跨桶处理）
    MyUnorderedMultiMapIterator& operator++() {
        ++list_it_;
        // 当前桶遍历完，则去找下一个非空桶
        if (list_it_ == map_->get_bucket(bucket_idx_).end()) {
            ++bucket_idx_;
            while (bucket_idx_ < map_->bucket_count() && map_->get_bucket(bucket_idx_).empty()) {
                ++bucket_idx_;
            }
            // 更新链表迭代器
            if (bucket_idx_ < map_->bucket_count()) {
                list_it_ = map_->get_bucket(bucket_idx_).begin();
            } else {
                list_it_ = typename std::list<value_type>::iterator(); // 置为无效迭代器，或者说置为末尾迭代器
                    // 哪里体现了末尾：通过将迭代器置为默认构造的迭代器，表示已到达末尾
            }
        }
        return *this;
    }

    // 后置++：返回旧状态，再自增
    MyUnorderedMultiMapIterator operator++(int) {
        MyUnorderedMultiMapIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    /*
        前置后置++总结：
        ++(*this)（前置 ++）：相当于this->operator++(),先修改迭代器，再返回修改后的自身；
        (*this)++（后置 ++）：相当于this->operator++(int)先保存当前状态，再修改迭代器，最后返回旧状态。
    */

    // 相等性比较
    bool operator==(const MyUnorderedMultiMapIterator& other) const {
        return map_ == other.map_ && bucket_idx_ == other.bucket_idx_ && list_it_ == other.list_it_;
    }

    bool operator!=(const MyUnorderedMultiMapIterator& other) const {
        return !(*this == other);
    }
    
private:
    // 允许容器类访问迭代器私有成员
    friend class MyUnorderedMultiMap<Key, T, Hash, KeyEqual>;

    MyUnorderedMultiMap<Key, T, Hash, KeyEqual>* map_; // 指向关联容器的指针
    size_type bucket_idx_;                             // 当前桶的索引
    typename std::list<value_type>::iterator list_it_; // 当前桶中链表的迭代器
};

// ------- 2. 容器类核心（移除内部iterator，引用上面定义的独立的迭代器类）------- //
template <typename Key,
          typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultiMap {
public:
    // 类型定义
    using value_type = std::pair<const Key, T>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using iterator = MyUnorderedMultiMapIterator<Key, T, Hash, KeyEqual>; // 关联迭代器类型

public:
    // 构造函数
    explicit MyUnorderedMultiMap(size_type bucket_count = 16,
                                 const Hash& hash = Hash(),
                                 const KeyEqual& equal = KeyEqual())
                                 : buckets_(bucket_count), hash_func_(hash), equal_func_(equal), size_(0), max_load_factor_(1.0f) {}

    // 插入键值对
    iterator insert(const Key& key, const T& value) {
        return insert(value_type(key, value));
    }

    // 插入键值对(具体实现)
    iterator insert(const value_type& val) {
        // 检查负载因子，触发重哈希
        if (size_ + 1 > buckets_.size() * max_load_factor_) {
            rehash(buckets_.size() * 2);
        }

        // 计算桶索引
        size_type hash_val = hash_func_(val.first);
        size_type bucket_idx = hash_val % buckets_.size();

        // 插入元素到对应桶的链表
        buckets_[bucket_idx].push_back(val);
        ++size_;

        // 返回新插入元素的迭代器
        auto list_it = buckets_[bucket_idx].end();
        --list_it;
        return iterator(this, bucket_idx, list_it);
    }

    // 查找所有匹配key的元素范围[begin, end)
    std::pair<iterator, iterator> equal_range(const Key& key) {
        size_type hash_val = hash_func_(key);
        size_type bucket_idx = hash_val % buckets_.size();
        auto& bucket = buckets_[bucket_idx];

        // 找第一个匹配元素
        auto begin_it = bucket.begin();
        while (begin_it != bucket.end() && !equal_func_(begin_it->first, key)) {
            ++begin_it;
        }
        // 找范围终点（最后一个匹配元素的下一个）
        auto end_it = begin_it;
        while (end_it != bucket.end() && equal_func_(end_it->first, key)) {
            ++end_it;
        }
        return {iterator(this, bucket_idx, begin_it), iterator(this, bucket_idx, end_it)};
    }

    // 删除所有匹配key的元素，返回删除数量
    size_type erase(const Key& key) {
        size_type hash_val = hash_func_(key);
        size_type bucket_idx = hash_val % buckets_.size();
        auto& bucket = buckets_[bucket_idx];
        size_type count = 0;
        
        // 遍历桶来删除匹配的桶元素
        auto it = bucket.begin();
        while (it != bucket.end()) {
            if (equal_func_(it->first, key)) {
                it = bucket.erase(it);
                ++count;
                --size_;
            } else {
                ++it;
            }
        }
        return count;
    }

    // 通过迭代器删除元素，返回下一个迭代器
    iterator erase(iterator pos) {
        if (pos == end()) {
            return end();
        }
        size_type bucket_idx = pos.bucket_idx_;
        auto& bucket = buckets_[bucket_idx];
        auto list_it = pos.list_it_;

        // 保存下一个迭代器
        auto next_it = list_it;
        ++next_it;

        // 删除元素
        bucket.erase(list_it);
        --size_;
        
        // 返回下一个有效迭代器
        if (next_it != bucket.end()) {
            return iterator(this, bucket_idx, next_it);
        } else {
            // 找下一个非空桶
            size_type next_bucket = bucket_idx + 1;
            while (next_bucket < buckets_.size() && buckets_[next_bucket].empty()) {
                ++next_bucket;
            }
            return (next_bucket < buckets_.size()) ?
                iterator(this, next_bucket, buckets_[next_bucket].begin()) : end();
        }
    }

    // 迭代器接口
    iterator begin() {
        // 找第一个非空桶
        size_type bucket_idx = 0;
        while (bucket_idx < buckets_.size() && buckets_[bucket_idx].empty()) {
            ++bucket_idx;
        }
        return bucket_idx < buckets_.size() ? 
            iterator(this, bucket_idx, buckets_[bucket_idx].begin()) : end();
    }

    iterator end() {
        return iterator(this, buckets_.size(), typename std::list<value_type>::iterator());
    }

    // 基础属性接口
    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }
    size_type bucket_count() const { return buckets_.size(); }

    // 供迭代器访问桶的接口（解决友元访问问题）
    std::list<value_type>& get_bucket(size_type bucket_idx) {
        return buckets_[bucket_idx];
    }

private:
    // 重哈希
    void rehash(size_type new_bucket_count) {
        if (new_bucket_count <= buckets_.size()) {
            return; // 只增不减
        }

        // 创建新桶数组
        std::vector<std::list<value_type>> new_buckets(new_bucket_count);

        // 迁移旧元素到新桶（使用move提升性能）
        for (auto& bucket : buckets_) {
            for (auto& elem : bucket) {
                size_type hash_val = hash_func_(elem.first);
                size_type new_bucket_idx = hash_val % new_bucket_count;
                new_buckets[new_bucket_idx].push_back(std::move(elem));
            }
        }
        buckets_.swap(new_buckets);
    }

private:
    // 允许迭代器类访问容器私有成员
    friend class MyUnorderedMultiMapIterator<Key, T, Hash, KeyEqual>;

    std::vector<std::list<value_type>> buckets_; // 桶数组，每个桶是一个链表
    size_type size_;                             // 元素总数
    Hash hash_func_;                             // 哈希函数
    KeyEqual equal_func_;                        // 键比较函数
    const float max_load_factor_;                // 最大负载因子
};