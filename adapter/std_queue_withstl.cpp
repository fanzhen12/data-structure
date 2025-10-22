#include <queue> // 底层容器
#include <cassert> // 断言
#include <iostream> // 输入输出流

// 模板类: MyQueue，元素类型为 T，底层容器为std::deque<T>
template <class T, class Container = std::deque<T>>
class MyQueue {
public:
    // 类型别名
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = typename Container::size_type;
    using container_type = Container;

protected:
    Container c; // 底层容器(核心存储)

public:
    // 默认构造
    MyQueue() : c() {}

    // 用已有容器初始化
    explicit MyQueue(const Container& cont) : c(cont) {}

    // 移动构造（针对底层容器）
    explicit MyQueue(Container&& cont) noexcept : c(std::move(cont)) {}

    // 判空
    bool empty() const noexcept {
        return c.empty();
    }

    // 获取元素数量
    size_type size() const noexcept {
        return c.size();
    }

    // 访问队头元素（不可修改）
    const_reference front() const {
        assert(!empty());
        return c.front();
    }

    // 访问队头元素（可修改）
    reference front() {
        assert(!empty());
        return c.front();
    }

    // 访问队尾元素（不可修改）
    const_reference back() const {
        assert(!empty());
        return c.back();
    }

    // 访问队尾元素（可修改）
    reference back() {
        assert(!empty());
        return c.back();
    }

    // 入队
    void push(const value_type& value) {
        c.push_back(value);
    }

    // 入队（移动语义，减少拷贝）
    void push(value_type&& value) {
        c.push_back(std::move(value));
    }

    // 入队（原地构造）
    template <class... Args>
    decltype(auto) emplace(Args&&... args) {
        return c.emplace_back(std::forward<Args>(args)...);
    }

    // 出队
    void pop() {
        assert(!empty());
        c.pop_front();
    }

    // 交换两个队列的内容
    void swap(MyQueue& other) noexcept(std::is_nothrow_swappable_v<Container>) {
        using std::swap;
        swap(c, other.c);
    }

    // 比较两个队列是否相等
    friend bool operator==(const MyQueue& lhs, const MyQueue& rhs) {
        return lhs.c == rhs.c;
    }
};

// 全局swap函数（与STL风格一致）
template <class T, class Container>
void swap(MyQueue<T, Container>& lhs, MyQueue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}



//@@@@@@@@@@@@@@ 测试代码 @@@@@@@@@@@@@@
// 测试代码
int main() {
    // 测试默认构造和基本操作
    MyQueue<int> q;
    assert(q.empty() == true);
    assert(q.size() == 0);

    // 入队
    q.push(10);
    q.push(20);
    q.emplace(30);  // 直接构造元素
    assert(q.size() == 3);
    assert(q.front() == 10);  // 队头是第一个入队的元素
    assert(q.back() == 30);   // 队尾是最后一个入队的元素

    // 出队
    q.pop();
    assert(q.size() == 2);
    assert(q.front() == 20);  // 出队后队头更新为下一个元素

    // 测试底层容器为list
    MyQueue<int, std::list<int>> q_list;
    q_list.push(100);
    q_list.push(200);
    assert(q_list.front() == 100);
    q_list.pop();
    assert(q_list.front() == 200);

    std::cout << "所有测试通过！" << std::endl;
    return 0;
}
