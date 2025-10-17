#include <deque>
#include <vector>
#include <list>

template <typename T, typename Container = std::deque<T>>
class MyStack {
public:
    using value_type = T;
    using container_type = Container;
    using size_type = typename Container::size_type;

private:
    Container c: // 底层容器，用于存储数据

public:
    MyStack() : c() {}

    // 构造函数：用底层容器初始化
    explicit MyStack(const Container& cont) : c(cont) {}

    // 入栈: 将元素添加到栈顶（底层容器尾部）
    void push(const value_type& val) {
        c.push_back(val);
    }

    // 出栈：移除栈顶元素（底层容器尾部）
    void pop() {
        if (!c.empty()) {
            c.pop_back();
        }
    }

    // 访问栈顶元素（返回底层容器尾部元素的引用）
    // 为什么返回的是引用，因为我们不仅希望通过这个函数返回栈顶元素，
    // 还希望能够修改它
    /*
        MyStack<int> stask;
        stack.push(10);
        stack.top() = 20; // 此时栈顶元素变为20
    */
    value_type& top() {
        return c.back();
    }

    const value_type& top() const {
        return c.back();
    }

    // 判空
    bool empty() const {
        return c.empty();
    }

    size_type size() const {
        return c.size();
    }

    // 交换两个栈的内容
    void swap(MyStack& other) noexcept {
        c.swap(other.c);
    }
};