#include <iostream>
#include <vector>
#include <string>
#include <functional> // for std::equal_to

// ------- 哈希节点结构：存储键值对和链表指针 ------- //
template <typename Key, typename Value>
struct HashNode {
    Key key;
    Value value;
    HashNode* next;

    HashNode(const Key& k, const Value& v) : key(k), value(v), next(nullptr) {}
};

// 自定义哈希函数
template <typename T>
struct MyHash {
    size_t operator()(const T& val) const {
        return std::hash<T>()(val); // 复用std::hash处理基本类型
    }
};

// 字符串哈希函数特例化（简单实现）
template <>
struct MyHash<std::string> {
    size_t operator()(const std::string& s) const {
        size_t hash_val = 0;
        for (char c : s) {
            hash_val = hash_val * 31 + c;
        }
        return hash_val;
    }
};

// ------- 自定义unordered_map实现 ------- //
template <typename Key,
          typename Value,
          typename Hash = MyHash<key>,
          typename KeyEqual = std::equal_to<Key>>
class MyUnordered {
private:
    using Node = HashNode<Key, Value>;
    std::vector<Node*> buckets; // 桶数组（每个元素是链表头指针）
    size_t size_; // 当前元素数量
    size_t bucket_count_; // 桶的数量
    const float max_load_factor_; // 最大负载因子（默认0.75）
    Hash hash_func; // 哈希函数对象
    KeyEqual key_eq; // 键比较函数对象

private:
    // 辅助函数：获取下一个质数
    size_t next_prime(size_t n) const {
        auto is_prime = [](size_t x) {
            if (x <= 1) return false;
            if (x == 2) return true;
            if (x % 2 == 0) return false;
            for (size_t i = 3; i * i <= x; i += 2) {
                if (x % i == 0) return false;
            }
            return true;
        };
        while (!is_prime(n)) ++n;
        return n;
    }

    // 扩容：重新分配桶并迁移所有元素
    void rehash() {
        size_t new_bucket_count = next_prime(bucket_count_ * 2); // 新桶数为当前桶数的两倍的下一个质数
        std::vector<Node*> new_buckets(new_bucket_count, nullptr);

        // 迁移旧桶中的数据
        for (size_t i = 0; i < bucket_count_; ++i) {
            Node* curr = buckets[i];
            while (curr) {
                Node* next = curr->next;

                // 计算在新桶中的索引
                size_t new_idx = hash_func(curr->key) % new_bucket_count;

                // 插入新桶的头部
                curr->next = new_buckets[new_idx];
                new_buckets[new_idx] = curr;
                curr = next;
            }
        }
        
        // 替换为新桶
        buckets.swap(new_buckets);
        bucket_count_ = new_bucket_count;
    }

public:
    // 构造函数：初始化桶数量（默认11，小质数）
    MyUnordered(size_t initial_buckets = 11, float max_load = 0.75f) 
        : max_load_factor_(max_load) {
        bucket_count_ = next_prime(initial_buckets);
        buckets.resize(bucket_count_, nullptr); // resize函数解释：
        // 调用std::vector的resize函数，将buckets的大小调整为bucket_count_，
        // 并用nullptr初始化每个元素。
        size_ = 0;
    }

    // 析构函数：释放所有节点内存
    ~MyUnordered() {
        clear();
    }

    // 清空所有元素
    void clear() {
        for (size_t i = 0; i < bucket_count_; ++i) {
            Node* curr = buckets[i];
            while (curr) {
                Node* next = curr->next;
                delete curr;
                curr = next;
            }
            buckets[i] = nullptr; // 此句的含义：
            // 将每个桶的头指针设置为nullptr，表示该桶现在为空。
        }
        size_ = 0;
    }

    // 插入或更新键值对
    void insert(const Key& key, const Value& value) {
        // 检查是否需要扩容
        if (size_ >= max_load_factor_ * bucket_count_) {
            rehash();
        }

        size_t idx = hash_func(key) % bucket_count_;
        Node* curr = buckets[idx];

        // 查找是否已存在该键，存在则更新值
        while (curr) {
            if (key_eq(curr->key, key)) {
                curr->value = value;
                return;
            }
            curr = curr->next;
        }

        // 不存在则插入新节点（头部插入，效率更高）
        Node* new_node = new Node(key, value);
        new_node->next = buckets[idx];
        buckets[idx] = new_node;
        size_++;
    }

    // 查找键：返回值的指针，不存在则返回nullptr
    Value* find(const Key& key) {
        size_t idx = hash_func(key) % bucket_count_;
        Node* curr = buckets[idx];
        while (curr) {
            if (key_eq(curr->key, key)) {
                return &(curr->value);
            }
            curr = curr->next;
        }
        return nullptr;
    }

    // 删除键：成功返回true，否则返回false
    bool erase(const Key& key) {
        size_t idx = hash_func(key) % bucket_count_;
        Node* curr = buckets[idx];
        Node* prev = nullptr;

        // 遍历链表查找目标节点
        while (curr) {
            if (key_eq(curr->key, key)) {
                if (prev == nullptr) {
                    // 此时要删除的就是头节点
                    buckets[idx] = curr->next; // 删除头节点
                } else {
                    prev->next = curr->next;
                }
                delete curr;
                size_--;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false; // 未找到该键
    }

    // 重载[]操作符：方便访问或插入元素
    Value& operator[](const Key& key) {
        Value* val = find(key);
        if (val) {
            return *val;
        }
        insert(key, Value()); // 不存在则插入默认值，这里默认值调用Value的默认构造函数
        return *find(key);
    }

    // 获取当前元素数量
    size_t size() const {
        return size_;
    }

    // 获取当前桶数量
    size_t bucket_count() const {
        return bucket_count_;
    }
};