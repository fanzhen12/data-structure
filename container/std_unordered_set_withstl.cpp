#include <vector>
#include <list>
#include <algorithm>
#include <functional> // 用于 std::hash？
#include <stdexcept>  // 用于 std::out_of_range

// 自定义哈希集合类模板
template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
class MyUnorderedSet {
private:
    using Bucket = std::list<T>; // 每个桶使用 std::list 存储元素
    std::vector<Bucket> buckets_; // 桶数组
    size_t size_;                 // 元素数量
    Hash hasher_;               // 哈希函数对象
    KeyEqual key_eq_;          // 相等比较函数对象
    float max_load_factor_;    // 最大负载因子

private:
    // 计算元素对应的桶索引
    size_t get_bucket_index(const T& key) const {
        return hasher_(key) % buckets_.size();
    }

    // 重新哈希，扩大桶数组并重新分配元素
    void rehash(size_t new_bucket_count) {
        if (new_bucket_count <= buckets_.size()) {
            return; // 只在新桶数大于当前桶数时重新哈希
        }

        // 创建新的桶数组
        std::vector<Bucket> new_buckets(new_bucket_count);

        // 将所有元素重新分配到新桶中
        for (auto& bucket : buckets_) {
            for (const T& elem : bucket) {
                size_t new_idx = hasher_(elem) % new_bucket_count;
                new_buckets[new_idx].push_back(elem);
            }
        }

        // 替换为新桶数组
        buckets_.swap(new_buckets);

    }

public:
    // 构造函数
    explicit MyUnorderedSet(size_t bucket_count = 11, float max_load_factor = 1.0f)
        : buckets_(bucket_count), size_(0), max_load_factor_(max_load_factor) {}

    // 插入元素
    bool insert(const T& key) {
        // 检查是否需要重新哈希
        if (load_factor() > max_load_factor_) {
            rehash(buckets_.size() * 2 + 1); // 扩大桶数组
        }
        size_t idx = get_bucket_index(key);
        Bucket& bucket = buckets_[idx];

        // 检查元素是否已存在
        auto it = std::find_if(bucket.begin(), bucket.end(),
            [&](const T& elem) { return key_eq_(elem, key); });
        
        if (it != bucket.end()) {
            return false; // 元素已存在，插入失败
        }

        // 插入新元素
        bucket.push_back(key);
        size_++;
        return true;
    }

    // 删除元素
    bool erase(const T& key) {
        size_t idx = get_bucket_index(key);
        Bucket& bucket = buckets_[idx];

        // 查找并删除元素
        auto it = std::find_if(bucket.begin(), bucket.end(),
            [&](const T& elem) { return key_eq_(elem, key); });
        
        if (it != bucket.end()) {
            bucket.erase(it); // 这里的it是list的迭代器，详情参见std::find_if返回值，它返回的是一个指向找到的元素的迭代器
            size_--;
            return true;
        }
        return false; // 元素不存在，删除失败
    }

    // 查找元素(返回是否存在)
    bool find(const T& key) const {
        size_t idx = get_bucket_index(key);
        const Bucket& bucket = buckets_[idx];

        auto it = std::find_if(bucket.begin(), bucket.end(),
            [&](const T& elem) { return key_eq_(elem, key); });

        return it != bucket.end();
    }

    // 清空集合
    void clear() {
        for (auto& bucket : buckets_) {
            bucket.clear();
        }
        size_ = 0;
    }

    // 获取元素数量
    size_t size() const {
        return size_;
    }

    // 判空
    bool empty() const {
        return size_ == 0;
    }

    // 获取当前负载因子
    float load_factor() const {
        return static_cast<float>(size_) / buckets_.size();
    }

    // 获取当前桶数量
    size_t bucket_count() const {
        return buckets_.size();
    }
};