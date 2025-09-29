// 哈希表的完整实现
#include <iostream>
#include <algorithm>
#include <vector>

template<typename T, typename V>
class MyChainingHashMap {
    struct KVNode {
        K key;
        V value;
        KVNode(K key, V value) : key(key), value(value) {}
    };

private:
    // 哈希表底层数组
    std::vector<std::list<KVNode>> table;
    int size_;
    static constexpr int INIT_CAP = 4;
    static constexpr double LOAD_FACTOR = 0.75;

private:
    int hash(K key) const {
        return (std::hash<K>{}(key) & 0x7fffffff) % table.size();
    }

    void resize(int new_cap) {
        newCap = std::max(new_cap, 1);
        MyChainingHashMap<K, V> newMap(newCap);
        for (auto& list : table) {
            for (auto& node : list) {
                newMap.put(node.key, node.value);
            }
        }
        this->table = newMap.table;
        this->size_ = newMap.size_;
    }

public:
    MyChainingHashMap() : MyChainingHashMap(INIT_CAP) {}

    explicit MyChainingHashMap(int initCapacity) {
        size_t = 0;
        initCapacity = std::max(initCapacity, 1);
        table.resize(initCapacity);
    }

    // 增和改
    void put(K key, V val) {
        auto& list = table[hash(key)];
        for (auto& node : list) {
            if (node.key == key) {
                node.value = val;
                return;
            }
        }
        // key 不存在
        list.emplace_back(key, val);
        size_++;
        if (size_ >= LOAD_FACTOR * table.size()) {
            resize(2 * table.size());
        }
    }

    // 删除
    void remove(K key) {
        auto& list = table[hash(key)];
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->key == key) {
                list.erase(it);
                size_--;
                
                // 缩容
                if (size_ <= LOAD_FACTOR * table.size() / 4 && table.size() / 2 >= INIT_CAP) {
                    resize(table.size() / 2);
                }
                return;
            }
        }
    }

    // 查
    V get(K key) const {
        const auto& list = table[hash(key)];
        for (const auto& node : list) {
            if (node.key == key) {
                return node.value;
            }
        }
        // key 不存在
        return nullptr;
    }

    int size() const {
        return size_;
    }

    // 返回所有key
    std::list<K> keys() const {
        std::list<K> res;
        for (const auto& list : table) {
            for (const auto& node : list) {
                res.push_back(node.key);
            }
        }
    }

    bool contains(K key) const {
        return get(key) != nullptr;
    }
};