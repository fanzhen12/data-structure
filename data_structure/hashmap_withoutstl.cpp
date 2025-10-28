#include <iostream>
#include <type_traits> // 用于类型特性检测
#include <cmath> // 用于abs函数
#include <cstring> // 字符串处理（不属于STL容器）

using namespace std;

template <typename K, typename V>
class HashTable {
private:
    // 自定义链表节点（存储键值对和下一个节点指针）
    struct Node {
        K key; // 键
        V value; // 值
        Node* next; // 指向下一个节点的指针
        // 构造函数
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    Node** buckets; // 哈希表的桶数组（为什么用二级指针？因为每个桶存储的是链表的头节点指针）
    int size; // 当前元素数量
    int capacity; // 哈希表容量（桶的数量）
    const double loadFactorThreshold; // 负载因子阈值

private:
    int hashFunc(const K& key) const {
        if constexpr (is_integral<K>::value) {
            return abs(static_cast<long long>(key)) % capacity; // 避免溢出
        } else if constexpr (is_same<K, string>::value) {
            int hash = 0;
            for (size_t i = 0; i < key.size(); ++i) {
                hash = (hash * 31 + key[i]) % capacity; // 简单的字符串哈希函数,31是常用的质数,可以减少冲突
            }
            return hash;
        } else {
            static_assert(always_false<K>::value, "Unsupported key type, only integral and string are supported.");
            // 不需要返回值，因为static_assert会在编译时触发
        }
    }

    // 重新哈希函数
    void rehash() {
        int oldCapacity = capacity;
        Node** oldBuckets = buckets; // 保存旧的桶数组
        // 为什么是二级指针？因为每个桶存储的是链表的头节点指针
        // oldBuckets是指向Node*的指针数组

        // 计算新容量
        capacity *= 2;
        if (capacity == 0) {
            capacity = 10;
        }

        // 初始化新的桶数组
        buckets = new Node*[capacity];
        for (int i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }

        // 将旧桶中所有节点重新映射到新桶
        for (int i = 0; i < oldCapacity; ++i) {
            Node* current = oldBuckets[i];
            while (current) {
                Node* nextNode = current->next; // 保存下一个节点
                int newIndex = hashFunc(current->key);
                current->next = buckets[newIndex];
                buckets[newIndex] = current;
                current = nextNode; // 继续处理下一个节点
            }
        }
        delete[] oldBuckets; // 释放旧桶数组内存
    }

public:
    HashTable(int initialCapacity = 10, double threshold = 0.7)
        : capacity(initialCapacity), loadFactoryThreshold(threshold), size(0) {
        if (capacity <= 0) {
            capacity = 10;
        }
        buckets = new Node*[capacity];
        for (int i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }
    }

    ~HashTable() {
        // 释放每个桶中的链表节点
        for (int i = 0; i < capacity; ++i) {
            Node* current = buckets[i];
            while (current) {
                Node* tmp = current;
                current = current->next;
                delete tmp;
            }
        }
        delete[] buckets; // 释放桶数组内存
        // 注意：一个是释放每个桶中的链表节点，另一个是释放桶数组本身
    }

    // 禁用拷贝构造函数和赋值运算符
    // 因为类中包含原始指针，默认的拷贝行为会导致浅拷贝问题
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // 插入或更新键值对
    void put(const K& key, const V& value) {
        // 检查负载因子，超过阈值则扩容
        if (static_cast<double>(size) / capacity >= loadFactorThreshold) {
            rehash();
        }

        int index = hashFunc(key);
        // 遍历链表，检查键是否存在（存在则更新，不存在则插入新节点）
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                current->value = value; // 更新值
                return;
            }
            current = current->next;
        }

        // 键不存在，则插入新节点（链表头部插入，效率更高）
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        size++;
    }

    // 查找键对应的值
    bool get(const K& key, V& value) const {
        int index = hashFunc(key);
        // 遍历链表查找键
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                value = current->value; // 找到键，返回对应值
                return true;
            }
        }
        return false; // 未找到键
    }

    // 删除键值对
    bool remove(const K& key) {
        int index = hashFunc(key);
        Node* current = buckets[index];
        Node* prev = nullptr; // 记录前一个节点

        // 遍历链表查找目标节点
        while (current) {
            if (current->key == key) {
                // 找到目标节点，开始执行删除
                if (!prev) {
                    // 目标就是链表头节点
                    buckets[index] = current->next;
                } else {
                    // 目标是中间/尾部节点
                    prev->next = current->next;
                }
                delete current; // 释放节点内存
                size--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false; // 未找到键
    }

    // 获取当前元素数量
    int getSize() const {
        return size;
    }

    // 判空
    bool isEmpty() const {
        return size == 0;
    }
};