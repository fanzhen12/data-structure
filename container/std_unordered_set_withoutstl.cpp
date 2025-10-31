// reference: https://www.doubao.com/chat/26923522862689282
#include <cstddef> // size_t
#include <cstring> // 用于字符串哈希计算
#include <cmath> // 用于浮点数哈希计算

// 自定义相等性比较：默认使用==运算符
template <typename T>
struct MyEqual {
    bool operator()(const T& a, const T& b) const {
        return a == b;
    }
};

// 自定义哈希函数：基础模板，需针对不同类型进行特化
template <typename T>
struct MyHash;

// 针对int类型的哈希函数特化
template <>
struct MyHash<int> {
    size_t operator()(int val) const {
        // 简单哈希：直接返回值，对于小整数效果较好
        return static_cast<size_t>(val);
    }
};

// 针对unsigned int类型的哈希函数特化
template <>
struct MyHash<unsigned int> {
    size_t operator()(unsigned int val) const {
        return static_cast<size_t>(val);
    }
};

// 针对long类型的哈希函数特化
template <>
struct MyHash<long> {
    size_t operator()(long val) const {
        // 处理负数：转为无符号值
        return static_cast<size_t>(val ^ (val >> 32)); // 如何理解这一句：
        // 这里假设long是64位，size_t也是64位。将long的高32位和低32位进行异或操作，混合高低位的信息，得到一个更均匀分布的哈希值。
    }
};

// 针对double类型的哈希函数特化
template <>
struct MyHash<double> {
    size_t operator()(double val) const {
        // 将double的二进制标识转为size_t，作为哈希值
        union {
            double d;
            unsigned long long u;
        } u{val};
        return static_cast<size_t>(u.u ^ (u.u >> 32)); // 如何理解这一句：
        // 这里假设unsigned long long是64位，size_t也是64位。将u.u的高32位和低32位进行异或操作，混合高低位的信息，得到一个更均匀分布的哈希值。
        // u.u是什么意思？u是一个联合体变量，u.u表示访问联合体中的成员u，它是一个unsigned long long类型，存储了double的二进制表示。
        // 为什么联合体u有两个成员？因为联合体的所有成员共享同一块内存空间，可以通过不同成员以不同类型访问同一块内存。
        // 其中的d成员用于存储double值，u成员用于以unsigned long long类型访问该double的二进制表示。
    }
};

// 针对const char*类型的哈希函数特化（C风格字符串）
template <>
struct MyHash<const char*> {
    size_t operator()(const char* str) const {
        if (str == nullptr) {
            return 0;
        }
        // 经典的字符串哈希算法：djb2
        size_t hash = 5381;
        int c;
        while ((c = *str++)) { // *str++如何理解？
            // 这里的*str++表示先解引用获取当前字符，然后将指针移动到下一个字符位置。
            // 等价于：
            // c = *str; c是当前字符，为什么用int类型？因为需要存储EOF等特殊值。
            // str = str + 1; 将指针移动到下一个字符。指针加1会根据指针类型自动移动相应的字节数。
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        return hash;
    }
};

// 前向声明：链表节点和链表类
template <typename T>
struct Node;

template <typename T>
class LinkedList;

// 链表节点：存储元素和指向下一个节点的指针
template <typename T>
struct Node {
    T data;
    Node* next;
    Node(const T& val) : data(val), next(nullptr) {}
};

// 自定义单向链表类（用于桶的实现）
template <typename T>
class LinkedList {
private:
    Node<T>* head;
    size_t len;

public:
    LinkedList() : head(nullptr), len(0) {}

    // 析构函数：释放所有节点内存
    ~LinkedList() {
        clear();
    }

    // 插入元素到链表尾部（不检查重复，由外部保证）
    void push_back(const T& val) {
        Node<T>* new_node = new Node<T>(val);
        if (!head) {
            head = new_node;
        } else {
            Node<T>* cur = head;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = new_node;
        }
        len++;
    }

    // 查找元素并删除（返回是否删除成功）
    template <typename KeyEqual>
    bool erase(const T& val, KeyEqual key_eq) {
        if (!head) {
            return false;
        }
        Node<T>* prev = nullptr;
        Node<T>* cur = head;

        // 遍历链表查找元素
        while (cur) {
            if (key_eq(cur->data, val)) {
                // 找到元素，调整指针并释放节点
                // 如果是头节点
                if (!prev) {
                    head = cur->next;
                } else {
                    prev->next = cur->next;
                }
                delete cur;
                len--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false; // 未找到元素
    }

    // 查找元素（返回是否存在）
    template <typename KeyEqual>
    bool contains(const T& val, KeyEqual key_eq) const {
        Node<T>* cur = head;
        while (cur) {
            if (key_eq(cur->data, val)) {
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    // 清空链表
    void clear() {
        Node<T>* cur = head; // 这一句有函数的调用吗？
        // 没有，这里只是声明了一个指针变量cur，用于遍历链表。
        Node<T>* tmp; // 这一句有函数的调用吗？
        // 没有，这里只是声明了一个指针变量tmp，用于在删除节点时临时保存当前节点，防止丢失对下一个节点的访问。
        while (cur) {
            Node<T>* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        head = nullptr;
        len = 0;
    }

    // 获取链表长度
    size_t size() const { return len; }

    // 链表是否为空
    bool empty() const { return len == 0; }

    // 遍历链表并执行回调函数，用于重hash时迁移元素
    template <typename Func>
    void for_each(Func func) const {
        Node<T>* cur = head;
        while (cur) {
            func(cur->data);
            cur = cur->next;
        }
    }

};

// 自定义动态数组（作为桶数组，存储多个头指针，这些指针是链表的头指针）
template <typename T>