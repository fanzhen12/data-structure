#include <cassert> // 用于断言
#include <iostream> // 用于输入输出流
#include <utility> // 用于 std::move

// 通过单链表这一基本数据结构实现std::queue
template <class T>
class MyQueue {
private:
    // 链表节点结构
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
        Node(T&& value) : data(std::move(value)), next(nullptr) {}
        // 直接构造数据（用于emplace）
        template <class... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
    };

    Node* head; // 队头指针
    Node* tail; // 队尾指针
    size_t size_; // 队列中元素数量

public:
    // 类型别名（与STL风格一致）
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

public:
    // 默认构造
    MyQueue() : head(nullptr), tail(nullptr), size_(0) {}

    // 析构函数，释放所有节点
    ~MyQueue() {
        clear();
    }

    // 拷贝构造（深拷贝）
    MyQueue(const MyQueue& other) : head(nullptr), tail(nullptr), size_(0) {
        Node* cur = other.head;
        while (cur) {
            push(cur->data);
            cur = cur->next;
        }
    }

    // 移动构造
    MyQueue(MyQueue&& other) noexcept : head(other.head), tail(other.tail), size_(other.size_) {
        // 接管资源后，置空原对象
        other.head = nullptr;
        other.tail = nullptr;
        other.size_ = 0;
    }

    // 拷贝赋值
    MyQueue& operator=(const MyQueue& other) {
        if (this != &other) {
            clear(); // 释放当前资源
            Node* cur = other.head;
            while (cur) {
                push(cur->data);
                cur = cur->next;
            }
        }
        return *this;
    }

    // 移动赋值
    MyQueue& operator=(MyQueue&& other) noexcept {
        if (this != &other) {
            clear(); // 释放当前资源
            // 接管资源
            head = other.head;
            tail = other.tail;
            size_ = other.size_;
            // 置空原对象
            other.head = nullptr;
            other.tail = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // 判空
    // 这里注意，什么时候用 noexcept 标记成员函数
    // 当函数不会抛出异常时，使用 noexcept 可以帮助编译器进行优化
    bool empty() const noexcept {
        return size_ == 0;
    }

    // 获取元素数量
    size_type size() const noexcept {
        return size_;
    }

    // 访问队头元素（不可修改）
    const_reference front() const {
        assert(!empty());
        return head->data;
    }

    // 访问队头元素（可修改）
    reference front() {
        assert(!empty());
        return head->data;
    }

    // 访问队尾元素（不可修改）
    const_reference back() const {
        assert(!empty());
        return tail->data;
    }

    // 访问队尾元素（可修改）
    reference back() {
        assert(!empty());
        return tail->data;
    }

    // 入队(拷贝语义)
    void push(const value_type& value) {
        Node* newNode = new Node(value);
        if (empty()) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++size_;
    }

    // 入队(移动语义)
    void push(value_type&& value) {
        Node* newNode = new Node(std::move(value));
        if (empty()) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++size_;
    }

    // 入队(原地构造)
    template <class... Args>
    void emplace(Args&&... args) {
        Node* newNode = new Node(std::forward<Args>(args)...);
        if (empty()) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++size_;
    }

    // 出队(移除队头元素)
    void pop() {
        assert(!empty());
        Node* old_head = head;
        head = head->next;
        delete old_head;
        --size_;
        if (empty()) {
            tail = nullptr;
        }
    }

    // 交换两个队列的内容
    void swap(MyQueue& other) noexcept {
        using std::swap;
        swap(head, other.head);
        swap(tail, other.tail);
        swap(size_, other.size_);
    }

    // 交换两个队列的内容（不依赖std的版本）
    void swap(MyQueue& other) noexcept {
        Node* tempHead = head;
        Node* tempTail = tail;
        size_t tempSize = size_;

        head = other.head;
        tail = other.tail;
        size_ = other.size_;

        other.head = tempHead;
        other.tail = tempTail;
        other.size_ = tempSize;
    }

    // 清空队列，释放所有节点
    void clear() noexcept {
        while (!empty()) {
            pop();
        }
    }
};

// @@@@@@@@@@@@ 测试代码 @@@@@@@@@@@@
int main() {
    // 测试基本操作
    MyQueue<int> q;
    assert(q.empty() == true);
    assert(q.size() == 0);

    // 入队操作
    q.push(10);
    q.push(20);
    q.emplace(30);  // 直接构造
    assert(q.size() == 3);
    assert(q.front() == 10);
    assert(q.back() == 30);

    // 出队操作
    q.pop();
    assert(q.size() == 2);
    assert(q.front() == 20);

    // 测试移动语义
    MyQueue<int> q2 = std::move(q);
    assert(q.empty() == true);  // q已被移动，为空
    assert(q2.size() == 2);
    assert(q2.front() == 20);

    // 测试拷贝构造
    MyQueue<int> q3(q2);
    assert(q3.size() == 2);
    assert(q3.back() == 30);

    // 测试清空
    q3.clear();
    assert(q3.empty() == true);

    std::cout << "所有测试通过！" << std::endl;
    return 0;
}
