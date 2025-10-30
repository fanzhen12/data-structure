#include <iterator> // 迭代器相关类型
#include <cstddef> // 用于 size_t, ptrdiff_t，ptrdiff是指针差值类型
#include <algorithm>

// ------- 节点结构：存储数据及前后指针 ------ //
template <typename T>
struct MyListNode {
    T data; // 数据域
    MyListNode* prev; // 前驱指针
    MyListNode* next; // 后继指针

    // 构造函数：元素节点（带数据）
    MyListNode(const T& val) : data(val), prev(nullptr), next(nullptr) {}
    // 构造函数：哨兵节点（不带数据）
    MyListNode() : prev(nullptr), next(nullptr) {}
};

// ------- 双向链表类 ------- //
template <typename T>
class MyList {
public:
    // 嵌套迭代器类（双向迭代器）
    class iterator {
    public:
        // 迭代器特性定义（符合 STL 规范）
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        // 构造函数
        iterator(MyListNode<T>* node) : m_node(node) {}

        // 解引用操作符
        reference operator*() const { return m_node->data; }
        pointer operator->() const { return &(m_node->data); } // &取地址，->用于访问成员，通过&获取data的地址
        // 举个例子：
        // it->memberFunction() 等价于 (it.operator->())->memberFunction()
        // 其中第一个->: 调用迭代器的operator->()函数，返回指向data的指针，因为operator->()返回的是&(m_node->data)，是一个指向data的指针。
        // 第二个->: 通过指针访问data的成员函数
        // 所以it->memberFunction()最终调用的是data的成员函数。

        // 前置++（移动到下一个节点）
        iterator& operator++() {
            m_node = m_node->next;
            return *this; // *this表示返回当前迭代器对象的引用。那么是data成员吗？不是，*this表示当前迭代器对象本身。
            // 因为this是一个指向当前对象的指针，*this就是解引用这个指针，就得到当前对象的引用。
        }

        // 后置++（移动到下一个节点）
        iterator operator++(int) {
            iterator tmp = *this;
            m_node = m_node->next;
            return tmp;
        }

        // 前置--（移动到前一个节点）
        iterator& operator--() {
            m_node = m_node->prev;
            return *this;
        }

        // 后置--（移动到前一个节点）
        iterator operator--(int) {
            iterator tmp = *this;
            m_node = m_node->prev;
            return tmp;
        }

        // 迭代器比较操作符
        bool operator==(const iterator& other) const { return m_node == other.m_node; }
        bool operator!=(const iterator& other) const { return m_node != other.m_node; }

        // 供链表内部调用（获取当前节点指针）
        MyListNode<T>* node() const { return m_node; }

    private:
        MyListNode<T>* m_node; // 指向当前节点
    };

    // 反向迭代器（复用std::reverse_iterator）
    using reverse_iterator = std::reverse_iterator<iterator>;

    // 默认构造：初始化哨兵节点，形成空循环链表
    MyList() {
        m_head = new MyListNode<T>(); // 哨兵节点（无数据）
        m_head->prev = m_head;
        m_head->next = m_head;
        m_size = 0;
    }

    // 拷贝构造（深拷贝）（因为有动态内存分配，必须自定义拷贝构造函数）
    // 动态内存分配体现在哪里？体现在每个节点都是new出来的。
    MyList(const MyList& other) : MyList() { // 委托给默认构造函数初始化空链表，这里就看出为什么需要自定义拷贝构造函数了，因为是在堆上分配内存。
        for (const auto& val : other) {
            push_back(val);
        }
    }

    // 拷贝赋值运算符（深拷贝）
    MyList& operator=(const MyList& other) {
        if (this != &other) {
            clear(); // 先清空当前链表
            for (const auto& val : other) {
                push_back(val);
            }
        }
        return *this;
    }

    // 析构函数：释放所有节点内存
    ~MyList() {
        clear(); // 释放元素节点
        delete m_head; // 释放哨兵节点
        m_head = nullptr; // 避免悬空指针
    }

    // 迭代器接口
    iterator begin() { return iterator(m_head->next); } // 指向第一个元素节点
    iterator end() { return iterator(m_head); } // 哨兵节点（尾后）
    reverse_iterator rbegin() { return reverse_iterator(end()); } // 反向开始
    reverse_iterator rend() { return reverse_iterator(begin()); } // 反向结束

    // 容量接口
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    // 元素访问
    T& front() {
        return *begin(); // 这里的*表示解引用迭代器，返回第一个元素的引用，*是iterator类中重载的operator*()
    }
    const T& front() const {
        return *begin();
    }
    T& back() {
        return *(--end()); // 先将end()迭代器前移一个位置，再解引用，得到最后一个元素
    }
    const T& back() const {
        return *(--end()); // 同上
    }

    // 尾部插入
    void push_back(const T& value) {
        insert(end(), value);
    }

    // 头部插入
    void push_front(const T& value) {
        insert(begin(), value);
    }

    // 尾部删除
    void pop_back() {
        if (!empty()) {
            erase(--end());
        }
    }

    // 头部删除
    void pop_front() {
        if (!empty()) {
            erase(begin());
        }
    }

    // 在pos位置插入新元素(更准确地说是在pos之前插入)
    iterator insert(iterator pos, const T& val) {
        MyListNode<T>* new_node = new MyListNode<T>(val); // 调用节点构造函数创建新节点
        MyListNode<T>* pos_node = pos.node(); // 获取pos位置的节点指针，这里的.node()是iterator类中的成员函数，返回当前节点指针。
        
        // 调整指针，插入新节点
        new_node->prev = pos_node->prev;
        new_node->next = pos_node;
        pos_node->prev->next = new_node;
        pos_node->prev = new_node;

        ++m_size; // 更新元素数量
        return iterator(new_node); // 返回新节点的迭代器
    }

    // 删除pos位置的元素，返回下一个元素的迭代器
    iterator erase(iterator pos) {
        if (pos == end()) { // 不能删除哨兵节点
            return end();
        }
        MyListNode<T>* del_node = pos.node(); // 获取要删除的节点指针
        iterator next_it(del_node->next); // 保存下一个节点的迭代器，这里调用了iterator的构造函数

        // 调整指针，跳过要删除的节点
        del_node->prev->next = del_node->next;
        del_node->next->prev = del_node->prev;

        delete del_node; // 释放删除节点的内存
        m_size--; // 更新元素数量
        return next_it; // 返回下一个节点的迭代器
    }

    // 清空所有元素（保留哨兵节点，为什么？因为哨兵节点是链表结构的基础，删除它会破坏链表结构）
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

private:
    MyListNode<T>* m_head; // 哨兵节点指针
    size_t m_size; // 元素数量
};