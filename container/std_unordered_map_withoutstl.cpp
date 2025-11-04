// reference: https://www.doubao.com/chat/27464450228393730
#include <cstdlib> // 提供malloc/free、rand等
#include <cstring> // memcpy（仅用于字符串复制）
#include <iostream> // 用于调试输出

// 字符串工具函数（提供std::string相关功能）
// 计算字符串长度
size_t my_strlen(const char* str) {
    if (str == nullptr) {
        return 0;
    }
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// 复制字符串（返回新分配的内存，需手动释放）
char* my_strdup(const char* str) {
    if (str == nullptr) {
        return nullptr;
    }
    size_t len = my_strlen(str);
    char* new_str = (char*)malloc(len + 1); // +1 留给'\0'
    if (new_str == nullptr) {
        std::cerr << "内存分配失败！" << std::endl;
        exit(1);
    }
    // 复制字符（包括'\0'）
    for (size_t i = 0; i <= len; ++i) {
        new_str[i] = str[i];
    }
    return new_str;
}

// 字符串比较（0表示相等，>0表示str1>str2，<0表示str1<str2）
int my_strcmp(const char* str1, const char* str2) {
    if (str1 == nullptr && str2 == nullptr) {
        return 0;
    }
    if (str1 == nullptr) {
        return -1;
    }
    if (str2 == nullptr) {
        return 1;
    }
    size_t i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        i++;
    }
    return (unsigned char)str1[i] - (unsigned char)str2[i];
}

// ------- 哈希节点结构（存储键值对及链表指针） ------- //
template <typename Key, typename Value>
struct HashNode {
    Key key;
    Value value;
    HashNode* next;
    
    HashNode(const Key& k, const Value& v) : key(k), value(v), next(nullptr) {}
};

// 针对char*的哈希节点结构特化（需手动管理字符串内存）
template <typename Value>
struct HashNode<char*, Value> {
    char* key;
    Value value;
    HashNode* next;

    // 构造时复制字符串
    HashNode(const char* k, const Value& v) : value(v), next(nullptr) {
        key = my_strdup(k);
    }

    // 析构时释放字符串内存
    ~HashNode() {
        if (key) {
            free(key);
            key = nullptr;
        }
    }
};

// 哈希函数模板
template <typename Key>
struct MyHash {
    size_t operator()(const Key& key) const;
};

// 整型哈希函数特化
template <>
struct MyHash<int> {
    size_t operator()(const int& key) const {
        return (size_t)key; // 简单哈希，直接返回整数值
    }
};

// char* 类型哈希函数特化
template <>
struct MyHash<char*> {
    size_t operator()(const char* key) const {
        if (key == nullptr) {
            return 0;
        }
        size_t hash_val = 0;
        for (size_t i = 0; key[i] != '\0'; ++i) {
            hash_val = hash_val * 31 + (unsigned char)key[i];
        }
        return hash_val;
    }
};

// 键比较函数模板
template <typename Key>
struct KeyEqual {
    bool operator()(const Key& a, const Key& b) const;
};

// 整型键比较函数特化
template <>
struct KeyEqual<int> {
    bool operator()(const int& a, const int& b) const {
        return a == b;
    }
};

// char* 类型键比较函数特化
template <>
struct KeyEqual<char*> {
    bool operator()(const char* a, const char* b) const {
        return my_strcmp(a, b) == 0;
    }
};

// ------- 自定义unordered_map实现 ------- //
template <typename Key,
          typename Value,
          typename Hash = MyHash<Key>,
          typename KeyEqual = KeyEqual<Key>>
class MyUnorderedMap {
private:
    using Node = HashNode<Key, Value>;
    Node** buckets; // 桶数组（每个元素是链表头指针）
    size_t size_; // 当前元素数量
    size_t bucket_count_; // 桶的数量
    const float max_load_factor_; // 最大负载因子（默认0.75）
    Hash hash_func; // 哈希函数对象
    KeyEqual key_eq; // 键比较函数对象

private:
    // 判断质数（用于扩容）
    bool is_prime(size_t n) const {
        if (n <= 1) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        for (size_t i = 3; i * i <= n; i += 2) {
            if (n % i == 0) return false;
        }
        return true;
    }

    // 获取下一个质数
    size_t next_prime(size_t n) const {
        while (!is_prime(n)) {
            n++;
        }
        return n;
    }

    // 扩容：重新分配桶并迁移所有元素
    void rehash() {
        size_t old_bucket_count = bucket_count_;
        Node** old_buckets = buckets;

        // 计算新桶数量
        bucket_count_ = next_prime(bucket_count_ * 2);
        // 分配新桶数组（初始化为nullptr）
        buckets = (Node**)calloc(bucket_count_, sizeof(Node*)); // 使用calloc初始化为0
        // calloc是C标准库函数，分配内存并初始化为0
        if (buckets == nullptr) {
            std::cerr << "内存分配失败！" << std::endl;
            exit(1);
        }

        // 迁移旧桶中的数据
        for (size_t i = 0; i < old_bucket_count; ++i) {
            Node* curr = old_buckets[i];
            while (curr) {
                Node* next = curr->next; // 保存下一个节点

                // 计算新桶索引
                size_t new_idx = hash_func(curr->key) % bucket_count_;
                // 插入新桶的头部
                curr->next = buckets[new_idx];
                buckets[new_idx] = curr;
                curr = next;
            }
        }

        // 释放旧桶数组（注意：节点已迁移，此处仅释放桶数组本身）
        free(old_buckets);
    }

public:
    // 构造函数（初始桶数量为11，小质数）
    MyUnorderedMap(size_t initial_buckets = 11, float max_load = 0.75f)
        : max_load_factor_(max_load), size_(0) {
        bucket_count_ = next_prime(initial_buckets);
        // 分配桶数组并初始化为nullptr（calloc会初始化为0）
        buckets = (Node**)calloc(bucket_count_, sizeof(Node*));
        if (buckets == nullptr) {
            std::cerr << "内存分配失败！" << std::endl;
            exit(1);
        }
    }

    // 析构函数，释放所有节点和桶数组内存
    ~MyUnorderedMap() {
        clear();
        free(buckets); // 释放桶数组本身
        buckets = nullptr;
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
            buckets[i] = nullptr;
        }
        size_ = 0;
    }

    // 插入或更新键值对
    void insert(const Key& key, const Value& value) {
        // 检查是否需要扩容
        if (size_ >= max_load_factor_ * bucket_count_) {
            rehash();
        }

        // 计算桶索引
        size_t idx = hash_func(key) % bucket_count_;
        Node* curr = buckets[idx];
        // 检查键是否已存在，若存在则更新值
        while (curr) {
            if (key_eq(curr->key, key)) {
                curr->value = value; // 更新值
                return;
            }
            curr = curr->next;
        }
        // 不存在则插入新节点到链表头部
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

    // 删除键，成功返回true，失败返回false
    bool erase(const Key& key) {
        size_t idx = hash_func(key) % bucket_count_;
        Node* curr = buckets[idx];
        Node* prev = nullptr;

        // 遍历链表查找目标节点
        while (curr) {
            if (key_eq(curr->key, key)) {
                if (prev == nullptr) {
                    buckets[idx] = curr->next; // 删除头节点
                } else {
                    prev->next = curr->next; // 删除中间或尾节点
                }
                delete curr;
                size_--;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    // 重载操作符[]，用于插入或访问元素
    Value& operator[](const Key& key) {
        Value* val = find(key);
        if (val) {
            return *val;
        }
        // 不存在则插入默认值（假设Value类型可默认构造）
        insert(key, Value());
        return *find(key); // *用于解引用指针，获得值引用
    }

    // 获取当前元素数量
    size_t size() const {
        return size_;
    }

    // 获取桶数量
    size_t bucket_count() const {
        return bucket_count_;
    }
};