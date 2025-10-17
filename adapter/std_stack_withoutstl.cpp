#include <iostream>
#include <cassert> // 用于断言空栈操作的合法性

// 栈中的一个节点
template <typename T>
struct StackNode {
    T data;          // 节点存储的数据
    StackNode* next; // 指向栈的下一个(指前一个入栈)节点

    StackNode(const T& val) : data(val), next(nullptr) {}
};

// 栈类（基于单链表实现）
template <typename T>
class MyStack {
private:
    StackNode<T>* top_node; // 此指针指向栈顶节点，即最新入栈的元素
    size_t size_;           // 栈中元素的数量

private:
    void copyFrom(const MyStack& other) {
        StackNode<T>* cur = other.top_node; // 从原栈的栈顶开始遍历
        MyStack<T> tmp;                     // 临时栈，用于反转顺序

        // 将原栈的元素按“栈顶->栈底”顺序压入临时栈，此时顺序是反的
        while (cur) {
            tmp.push(cur->data);
            cur = cur->next;
        }

        // 将临时栈的元素弹出并压入当前栈，恢复原栈顺序
        while (!tmp.empty()) {
            push(tmp.top());
            tmp.pop();
        }
    }

public:
    // 构造函数：初始化空栈
    MyStack() : top_node(nullptr), size_t(0) {}

    // 深拷贝构造函数
    MyStack(const MyStack& other) : top_node(nullptr), size_(0) {
        copyFrom(other);
    }

    // 深拷贝赋值运算符
    MyStack& operator=(const MyStack& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // 析构函数，释放所有节点内存，防止泄漏
    ~MyStack() {
        clear(); // 清空栈中所有的元素
    }

    // 入栈：在栈顶添加元素
    void push(const T& val) {
        // 创建新节点，新节点的next指针指向当前栈顶
        StackNode<T>* new_node = new StackNode<T>(val);
        new_node->next = top_node;
        // 更新栈顶为新节点
        top_node = new_node;
        size_++;
    }

    // 出栈：移除栈顶元素
    void pop() {
        // 断言：禁止对空栈进行操作
        assert(!empty() && "Error: pop() on empty stack");

        // 保存当前栈顶节点的指针
        StackNode<T>* tmp = top_node;
        top_node = top_node->next;
        delete tmp;
        size_--;
    }

    // 访问栈顶元素(返回引用，支持修改)
    T& top() {
        // 断言：禁止对空栈访问top
        assert(!empty() && "Error: top() on empty stack");
        return top_node->data;
    }

    const T& top() const {
        assert(!empty() && "Error: top() on empty stack");
        return top_node->data;
    }

    // 判断栈是否为空
    bool empty() const {
        return size_ == 0;
    }

    size_t size() const {
        return size_;
    }

    // 清空栈中所有元素（释放内存）
    void clear() {
        while (!q.empty()) {
            pop();
        }
    }

    // 拷贝构造（深拷贝）
};