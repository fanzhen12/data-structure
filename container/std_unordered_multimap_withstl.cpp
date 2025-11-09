#include <vector>
#include <list>
#include <utility>
#include <functional>
#include <cstddef> 

// ------- 自定义无序多重map类模板 ------- //
template <typename Key,
          typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class MyUnorderedMultimap {
public:
    // 类型定义
    using value_type = std::pair<const Key, T>; // 键值对，键不可修改
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = std::size_t;

public:
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag; // 前向迭代器
        using value_type = MyUnorderedMultimap::value_type;
        using reference = MyUnorderedMultimap::reference;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;

    public:
        iterator(MyUnorderedMultimap* map, size_type bucket_idx, typename std::list<value_type>::iterator list_it)
            : map_(map), bucket_idx_(bucket_idx), list_it_(list_it) {}

        // 解引用运算符
        reference operator*() const { return *list_it_; } // 返回当前元素的引用
        pointer operator->() const { return &(*list_it_); } // 返回当前元素的指针
        /*
        pointer operator->()举例说明：
        int main() {
            // 创建一个存储 int->std::string 的 multimap
            MyUnorderedMultimap<int, std::string> map;
            
            // 插入元素
            map.insert(1, "apple");
            map.insert(2, "banana");
            
            // 获取迭代器（指向第一个元素）
            auto it = map.begin();
            
            // 1. 使用 operator*() 访问元素（解引用）
            std::pair<const int, std::string>& elem = *it;
            std::cout << "键：" << elem.first << "，值：" << elem.second << std::endl;
            // 输出：键：1，值：apple
            
            // 2. 使用 operator->() 访问元素的成员（更简洁）
            // it->first 相当于 (it.operator->())->first
            // (it.operator->())返回一个指向当前元素的指针，然后通过指针访问其first成员
            std::cout << "键：" << it->first << "，值：" << it->second << std::endl;
            // 输出：键：1，值：apple
            
            return 0;
        }
        */

        // 前置++，移动到下一个元素（跨桶处理）
        iterator& operator++() {
            ++list_it_;
            // 若当前桶内已经没有元素，就去寻找下一个非空桶
            if (list_it_ == map_->buckets_[bucket_idx_].end()) {
                ++bucket_idx_;
                while (bucket_idx_ < map_->buckets_.size() && map_->buckets_[bucket_idx_].empty()) {
                    ++bucket_idx_;
                }
                // 更新链表迭代器（若找到非空桶）
                if (bucket_idx_ < map_->buckets_.size()) {
                    list_it_ = map_->buckets_[bucket_idx_].begin();
                } else {
                    list_it_ = typename std::list<value_type>::iterator(); // 置为末尾迭代器
                    // 哪里体现了末尾：通过将迭代器置为默认构造的迭代器，表示已到达末尾
                }
            }
            return *this;
        }

        // 后置++，移动到下一个元素，这里返回的是一个迭代器对象
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /*
        前置后置++总结：
        ++(*this)（前置 ++）：相当于this->operator++(),先修改迭代器，再返回修改后的自身；
        (*this)++（后置 ++）：相当于this->operator++(int)先保存当前状态，再修改迭代器，最后返回旧状态。
        */

        // 相等性比较
        bool operator==(const iterator& other) const {
            return map_ == other.map_ && bucket_idx_ == other.bucket_idx_ && list_it_ == other.list_it_;
        }
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        MyUnorderedMultimap* map_; // 指向容器
        size_type bucket_idx_; // 当前桶的索引
        typename std::list<value_type>::iterator list_it_; // 当前桶内的迭代器，即链表迭代器
    };

public:
    // 构造函数：指向初始桶数量、哈希函数、相等性比较函数
    explicit MyUnorderedMultimap(size_type bucket_count = 16,
                                 const Hash& hash = Hash(),
                                 const KeyEqual& equal = KeyEqual())
                                 : buckets_(bucket_count), hash_func_(hash)
                                   , equal_func_(equal), size_(0) {}

    // 插入键值对(返回指向新元素的迭代器)
    iterator insert(const Key& key, const T& value) {
        return insert(value_type(key, value));
    }

    iterator insert(const value_type& val) {
        // 检查负载因子
        if (size_ + 1 > buckets_.size() * max_load_factor_) {
            rehash(buckets_.size() * 2);
        }

        // 计算哈希值并定位桶
        size_type hash_val = hash_func_(val.first); // first就是key
        size_type bucket_idx = hash_val % buckets_.size();

        // 插入元素到新桶的链表
        buckets_[bucket_idx].push_back(val);
        ++size_;

        // 返回指向新元素的迭代器
        auto list_it = buckets_[bucket_idx].end();
        --list_it; // 指向新插入的元素
        return iterator(this, bucket_idx, list_it);
    }

    // 查找所有匹配Key的元素（返回范围[begin, end)）
    std::pair<iterator, iterator> equal_range(const Key& key) {
        size_type hash_val = hash_func_(key);
        size_type bucket_idx = hash_val % bucket_.size();
        auto& bucket = buckets_[bucket_idx];

        // 找到桶中第一个匹配的元素
        auto begin_it = bucket.begin();
        while (begin_it != bucket.end() && !equal_func_(begin_it->first, key)) {
            ++begin_it;
        }

        // 找到桶中最后一个匹配元素的下一个位置
        auto end_it = begin_it;
        while (end_it != bucket.end() && equal_func_(end_it->first, key)) {
            ++end_it;
        }
        return {iterator(this, bucket_idx, begin_it), iterator(this, bucket_idx, end_it)};
    }

    // 通过迭代器删除元素

private:
    std::vector<std::list<value_type>> buckets_; // 桶数组（每个桶是链表）
    size_type size_; // 元素总数
    Hash hash_func_; // 哈希函数
    KeyEqual equal_func_; // 键比较函数
    const float max_load_factor_ = 1.0f; // 最大负载因子

private:
    // 重哈希
    void rehash(size_type new_bucket_count) {
        if (new_bucket_count <= buckets_.size()) return;

        // 创建新桶数组
        std::vector<std::list<value_type>> new_buckets(new_bucket_count);

        // 重新分配旧元素到新桶
        for (auto& bucket : buckets_) {
            for (auto& elem : bucket) {
                size_type hash_val = hash_func_(elem.first);
                size_type new_bucket_idx = hash_val % new_bucket_count;
                new_buckets[new_bucket_idx].push_back(std::move(elem)); // 可以使用std::move(elem)吗？
                // 是的，使用std::move可以避免不必要的拷贝，提高性能
            }
        }

        // 替换为新桶数组
        buckets_.swap(new_buckets);
    }
};