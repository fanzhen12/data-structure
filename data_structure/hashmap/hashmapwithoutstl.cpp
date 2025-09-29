#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#endif // MY_HASH_MAP_H

#include <iostream>
#include <cstring>
#include <cstdlib>

template<typename K, typename V>
struct HashNode {
    K key;
    V value;
    HashNode* next;

    HashNode(const K& k, cosnt V& v) : key(k), value(v), next(nullptr) {}

};

template<typename K, typename V>
class MyHashMap {
private:
    HashNode<K, V>** table; // 哈希表底层数组，存储链表头节点指针
    int capacity; // 哈希表容量
    int size; // 哈希表元素数量
    const float loadFactor; // 负载因子阈值
    const int initCapacity; // 初始容量

    // 哈希函数
    int hash(const K& key) const {
        size_t hashValue = 0;
        const char* data = reinterpret_cast<const char*>(&key);
        for (size_t i = 0; i < sizeof(K); ++i) {
            hashValue = hashValue * 31 + data[i];
        }
        return static_cast<int>(hashValue % capacity);
    }

    // 扩容/缩容
    void resize(int newCapacity) {
        if (newCapacity < initCapacity) {
            return;
        }
        // 创建新的哈希表
        HashNode<K, V>** newTable = new HashNode<K, V>*[newCapacity]();
        int oldCapacity = capacity;

        // 重新哈希所有元素到新表
        for (int i = 0; i < oldCapacity; ++i) {
            HashNode<K, V>* node = table[i];
            while (node) {
                // 先把下一个节点保存起来
                HashNode<K, V>* next = node->next;
                
                // 计算在新表中的位置
                int newIndex = hash(node->key) % newCapacity;
                // 插入到新表的对应位置
                node->next = newTable[newIndex];
                newTable[newIndex] = node;

                // 继续处理下一个节点
                node = next;
            }
        }

        // 释放旧表并更新指针
        delete[] table;
        table = newTable;
        capacity = newCapacity;
    }

    // 比较两个键是否相等
    bool equals(const K& a, const K& b) const {
        return memcmp(&a, &b, sizeof(K)) == 0;
    }

    ~MyHashMap() {
        clear();
        delete[] table;
    }

    // 清空哈希表
    void clear() {
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* node = table[i];
            while (node) {
                HashNode<K, V>* tmp = node;
                node = node->next;
                delete tmp;
            }
            table[i] = nullptr;
        }
        size = 0;
    }

    // 获取哈希表中元素的数量
    int getSize() const {
        return size;
    }

    // 判断哈希表是否为空
    bool isEmpty() const {
        return size == 0;
    }

    // 获取哈希表容量
    int getCapacity() const {
        return capacity;
    }

    // 是否包含指定键
    bool containsKey(const K& key) const {
        int index = hash(key);
        HashNode<K, V>* node = table[index];
        while (node) {
            if (equals(node->key, key)) {
                return true;
            }
            node = node->next;
        }
        return false;
    }

    // 插入与更新键值对
    void put(const K& key, const V& value) {
        // 检查是否需要扩容
        if (size >= capacity * loadFactor) {
            resize(capacity * 2);
        }

        int index = hash(key);
        HashNode<K, V>* node = table[index];

        while (node) {
            if (equals(node->key, key)) {
                node->value = value;
                return;
            }
            node = node->next;
        }

        // 如果不存在
        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        ++size;
    }

    // 获取键对应的值
    bool get(const K& key, V& value) const {
        int index = hash(key);
        HashNode<K, V>* node = table[index];
        while (node) {
            if (equals(node->key, key)) {
                value = node->value;
                return true;
            }
            node = node->next;
        }
        return false;
    }

    // 删除键值对
    bool remove(const K& key) {
        int index = hash(key);
        HashNode<K, V>* node = table[index];
        HashNode<K, V>* prev = nullptr;

        while (node) {
            if (equals(node->key, key)) {
                // 从链表中删除
                if (!prev) {
                    table[index] = node->next;
                } else {
                    prev->next = node->next;
                }
                delete node;
                --size;

                // 检查是否需要缩容
                if (size < capacity * loadFactor / 4) {
                    resize(capacity / 2);
                }
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }
};

