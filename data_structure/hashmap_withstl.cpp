#include <iostream>
#include <vector>
#include <list>
#include <type_traits> // 用于类型特性检测
#include <cmath>       // 用于abs函数

using namespace std;

template <typename K, typename V>
class HashTable {
private:
    vector<list<pair<K, V>>> buckets; // 桶数组，每个桶是存储键值对的链表
    int size;                         // 当前元素数量
    int capacity;                     // 哈希表容量
    const double loadFactorThreshold; // 负载因子阈值

private:
    // 哈希函数
    int hashFunc(const K& key) const {
        if constexpr (is_intergral<K>:: value) {
            return abs(key) % capacity;
        } else if constexpr (is_same<K, string>::value) {
            int hash = 0;
            for (char c : key) {
                hash = (hash * 31 + c) % capacity; // 31 是一个常用的质数，可以用于减少冲突
            }
            return hash;
        } else {
            static_assert(always_false<K>::value, "Unsupported key type(only integral and string)");
        }
    }

    // 重哈希
    void rehash() {
        int oldCapacity = capacity;
        capacity *= 2; // 扩容为原来的两倍
        if (capacity == 0) {
            capacity = 10; // 初始容量
        }

        // 新桶数组
        vector<list<pair<K, V>>> newBuckets(capacity);

        // 将旧桶元素重新映射到新桶
        for (int i = 0; i < oldCapacity; ++i) {
            for (auto& pair : buckets[i]) {
                int newIndex = hashFunc(pair.first);
                newBuckets[newIndex].push_back(pair);
            }
        }

        // 交换新旧桶
        buckets.swap(newBuckets);
    }

public:
    HashTable(int initialCapacity = 10, double threshold = 0.7)
        : capacity(initialCapacity), loadFactorThreshold(threshold), size(0) {
        if (capacity <= 0) {
            capacity = 10;
        }
        buckets.resize(capacity);
    }

    // 插入或者更新键值对（有就更新，无就插入）
    void put(const K& key, const V& value) {
        // 检查是否需要扩容
        if (size * 1.0 / capacity >= loadFactorThreshold) {
            rehash();
        }

        int index = hashFunc(key);
        // 如果键已经存在，则更新值
        for (auto& pair : buckets[index]) {
            if (pair.first == key) {
                pair.second = value;
                return;
            }
        }

        // 如果键不存在，则插入新键值对
        buckets[index].emplace_back({key, value});
        size++;
    }

    // 查找键对应的值（找到返回true，通过value引用输出）
    bool get(const K& key, V& value) const {
        int index = hashFunc(key);
        for (const auto& pair : buckets[index]) {
            if (pair.first == key) {
                value = pair.second;
                return true;
            }
        }
        return false; // 未找到
    }

    // 删除键值对
    bool remove(const K& key) {
        int index = hashFunc(key);
        auto& bucket = buckets[index]; // 当前链表
        // 遍历当前链表查找并删除键
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                size--;
                return true; // 删除成功
            }
        }
        return false; // 未找到键，键不存在
    }

    // 获取当前元素数量
    int getSize() const {
        return size;
    }

    // 判断哈希表是否为空
    bool isEmpty() const {
        return size == 0;
    }
};