#include <initializer_list> // for std::initializer_list
#include <stdexcept> // for std::out_of_range
#include <utility> // for std::move

// ******** 节点结构与前向迭代器实现 ******** //
// 节点结构
template <typename T>
struct my_forward_list_node {
    T data;
    my_forward_list_node* next; // 指向下一个节点的指针

    // 拷贝构造函数
    my_forward_list_node(const T& value, my_forward_list_node* n = nullptr) : data(value), next(n) {}
    // 移动构造函数
    my_forward_list_node(T&& value, my_forward_list_node* n = nullptr) : data(std::move(value)), next(n) {}; 
};

// 前向迭代器(非const版本)
template <typename T>
class my_forward_list_iterator {
public:
    // 迭代器特性定义（满足forward_iterator要求）
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using difference_type = std::ptrdiff_t; // 用于指针算术运算
    using iterator_category = std::forward_iterator_tag; // 前向迭代器标签

    // 构造函数(从节点指针构造)
    explicit my_forward_list_iterator(my_forward_list_node<T>* ptr) : node_(ptr) {}
    // 解引用操作符
    reference operator*() const { return node_->data }
    pointer operator->() const { return &(node_->data); }

    // 前向移动(前置++)
    // 即返回递增之后的迭代器引用
    my_forward_list_iterator& operator++() {
        node_ = node_->next;
        return *this;
    }

    // 前向移动(后置++)
    // 即返回递增之前的迭代器副本
    my_forward_list_iterator operator++(int) {
        my_forward_list_iterator temp = *this;
        node_ = node_->next;
        return temp;
    }

    // 相等比较
    bool operator==(const my_forward_list_iterator& other) const {
        return node_ == other.node_;
    }
    // 不等比较
    bool operator!=(const my_forward_list_iterator& other) const {
        return node_ != other.node_;
    }

    // 友元类声明，允许访问私有成员
    friend class my_forward_list<T>;

private:
    my_forward_list_node<T>* node_; // 当前节点指针，指向当前节点
};

// 前向迭代器(const版本)
template <typename T>
class my_forward_list_const_iterator {
public:
    using value_type = const T;
    using reference = const T&;
    using pointer = const T*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    // 构造函数（支持从非const迭代器转换）
    explicit my_forward_list_const_iterator(const my_forward_list_node<T>* node) : node_(node) {}
    my_forward_list_const_iterator(const my_forward_list_iterator<T>& other) : node_(other.node_) {}

    // 解引用操作（仅读取）
    reference operator*() const { return node_->data; }
    pointer operator->() const { return &(node_->data); }

    // 前向移动(前置++)
    my_forward_list_const_iterator& operator++() {
        node_ = node_->next;
        return *this;
    }

    // 前向移动(后置++)
    my_forward_list_const_iterator operator++(int) {
        my_forward_list_const_iterator tmp = *this;
        node_ = node_->next;
        return tmp;
    }

    // 相等比较
    bool operator==(const my_forward_list_const_iterator& other) const {
        return node_ == other.node_;
    }
    // 不等比较
    bool operator!=(const my_forward_list_const_iterator& other) const {
        return node_ != other.node_;
    }

    // 友元类声明，允许访问私有成员
    friend class my_forward_list<T>;

private:
    const my_forward_list_node<T>* node_; // 当前节点指针，指向当前节点(const版本)
};

// ******** 前向链表容器实现 ******** //
template <typename T>
class my_forward_list {
public:
    // 类型别名（兼容STL风格）
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = my_forward_list_iterator<T>;
    using const_iterator = my_forward_list_const_iterator<T>;
    using size_type = std::size_t;

    // 构造函数（空链表）
    my_forward_list() : head_(nullptr) {}

    // 初始化列表构造（从std::initializer_list初始化）
    my_forward_list(std::initializer_list<T> init) : head_(nullptr) {
        // 从初始化列表的尾部开始插入节点
        for (auto it = init.end(); it != init.begin();) {
            --it;
            push_front(*it);
        }
    }

    // 拷贝构造（深拷贝）
    my_forward_list(const my_forward_list& other) : head_(nullptr) {
        my_forward_list_node<T>** curr = &head_; // 跟踪当前节点的next指针，即插入位置
        // &的作用是取地址
        // 为什么这里是指向指针的指针？
        // 因为我们需要修改当前节点的next指针以插入新节点
        // 两个*可以干什么，回答如下：
        // 第一个*解引用curr，得到当前节点的指针
        // 第二个*解引用当前节点的指针，得到当前节点本身，从而访问next指针
        for (const auto& val : other) {
            *curr = new my_forward_list_node<T>(val); // 通过拷贝构造新节点
            curr = &((*curr)->next); // 移动到下一个插入位置
            // 上一句代码解释如下：
            // curr: 指向当前节点指针的指针
            // *curr：*为解引用，此时*curr代表当前节点的指针
            // (*curr)->next：访问当前节点的next指针
            // &((*curr)->next)：取当前节点的next指针的地址，作为下一个插入位置
        }
    }

    // 移动构造: (移动语义，接管资源)
    my_forward_list(my_forward_list&& other) noexcept : head_(other.head_) {
        other.head_ = nullptr; // 将源对象置为空链表，防止析构时释放资源，源对象放弃资源所有权
    }

    // 析构函数（释放所有节点）
    ~my_forward_list() {
        clear();
    }

    // 拷贝赋值运算符（使用拷贝-交换习惯用法）
    my_forward_list& operator=(const my_forward_list& other) {
        if (this != &other) {
            my_forward_list temp(other); // 利用拷贝构造函数创建临时副本
            swap(temp);  // 交换当前对象与临时对象的资源
        }
        return *this;
    }

    // 移动赋值运算符, 接管资源
    my_forward_list& operator=(my_forward_list&& other) noexcept {
        if (this != &other) {
            clear(); // 释放当前资源
            head_ = other.head_; // 接管资源
            other.head_ = nullptr; // 将源对象置为空链表
        }
        return *this;
    }

    // 交换两个链表的内容
    void swap(my_forward_list& other) noexcept {
        std::swap(head_, other.head_);
    }

    // 迭代器接口
    iterator before_begin() noexcept {
        return iterator(reinterpret_cast<my_forward_list_node<T>*>(&head_));
        // 对上面这一句代码的解释如下：
        // head_: 指向链表第一个节点的指针
        // &head_: 取head_指针的地址，即指向“指向链表第一个节点的指针”的指针，也即是before_begin位置
        // reinterpret_cast<my_forward_list_node<T>*>: 将指向指针的指针转换为my_forward_list_node<T>*类型的指针，在转换之前是void**类型
        // iterator(...): 使用转换后的指针构造迭代器对象，这是调用了迭代器的构造函数
        // 返回的迭代器指向before_begin位置，是指向head_指针的指针，通过指向head_指针的指针可以访问和修改head_指针
        // 如何实现首部插入？通过修改head_指针来实现，因为head_指针指向链表的第一个节点，当head_指针改变时，链表的第一个节点也随之改变
    }
    const_iterator before_begin() const {
        return const_iterator(reinterpret_cast<const my_forward_list_node<T>*>(&head_));
    }

    iterator begin() { return iterator(head_); }
    const_iterator begin() const { return const_iterator(head_); }
    const_iterator cbegin() const { return const_iterator(head_); }

    iterator end() { return iterator(nullptr); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cend() const { return const_iterator(nullptr); }

    bool empty() const noexcept {
        return head_ == nullptr;
    }

    // 头部插入新节点（拷贝版本）
    void push_front(const T& value) {
        head_ = new my_forward_list_node<T>(value, head_);
    }

    // 头部插入新节点（移动版本）
    void push_front(T&& value) {
        head_ = new my_forward_list_node<T>(std::move(value), head_);
    }

    // 头部删除节点
    void pop_front() {
        if (empty()) {
            throw std::out_of_range("List is empty");
        }
        my_forward_list_node<T>* temp = head_;
        head_ = head_->next;
        delete temp;
    }

    // 在pos之后插入新节点（拷贝版本）,返回新节点的迭代器
    iterator insert_after(const_iterator pos, const T& value) {
        if (pos.node_ == reinterpret_cast<my_forward_list_node<T>*>(&head_)) {
            // pos是before_begin位置，等价于头部插入
            return insert_after_before_begin(val);
        }
        if (!pos.node_) {
            throw std::out_of_range("insert_after on end iterator");
        }
        // 在pos节点之后插入新节点
        // 为什么要保存pos.node_->next？
        // 因为插入新节点后，原来的next指针会被修改
        // new_node的next指针需要指向原来的下一个节点
        my_forward_list_node<T>* new_node = new my_forward_list_node<T>(value, pos.node_->next);
        pos.node_->next = new_node;
        return iterator(new_node);
    }

    // 在pos之后插入新节点（移动版本）
    iterator insert_after(const_iterator pos, T&& value) {
        if (pos.node_ == reinterpret_cast<my_forward_list_node<T>*>(&head_)) {
            return insert_after_before_begin(std::move(val));
        }
        if (!pos.node_) {
            throw std::invalid_argument("insert_after on end iterator");
        }
        my_forward_list_node<T>* new_node = new my_forward_list_node<T>(std::move(value), pos.node_->next); // new_node的next指向原来的下一个节点
        pos.node_->next = new_node;
        return iterator(new_node);
    }

    // 删除pos之后的元素
    iterator erase_after(iterator pos) {
        if (pos.node_ == reinterpret_cast<my_forward_list_node<T>*>(&head_)) {
            // pos是before_begin位置，等价于头部删除
            return erase_after_before_begin();
        }
        if (!pos.node_ || !pos.node_->next) {
            throw std::out_of_range("erase_after on invalid position");
        }
        // 删除pos节点之后的节点
        my_forward_list_node<T>* to_delete = pos.node_->next;
        pos.node_->next = to_delete->next;
        delete to_delete;
        return iterator(pos.node_->next); // 返回被删除节点之后的节点的迭代器
    }

    // 清空链表
    void clear() {
        while (head_) {
            my_forward_list_node<T>* tmp = head_;
            head_ = head_->next;
            delete tmp;
        }
    }

private:
    // 在before_begin位置后插入新节点(拷贝版本)
    iterator insert_after_before_begin(const T& value) {
        head_ = new my_forward_list_node<T>(value, head_);
        return iterator(head_);
    }

    // 在before_begin位置后插入新节点(移动版本)
    iterator insert_after_before_begin(T&& value) {
        head_ = new my_forward_list_node<T>(std::move(value), head_);
        return iterator(head_);
    }

    // 辅助函数：删除before_begin后的元素（头部删除）
    iterator erase_after_before_begin() {
        if (empty()) {
            throw std::out_of_range("erase_after on empty list");
        }
        my_forward_list_node<T>* to_delete = head_;
        head_ = head_->next;
        delete to_delete;
        return iterator(head_);
    }

private:
    my_forward_list_node<T>* head_; // 指向链表第一个节点的指针

    // 辅助函数：在before_begin节点之后插入新节点(头部插入)
};