#include <vector>
#include <stdexcept>
#include <algorithm>

// 自定义优先级队列（最大堆实现）
template <typename T, typename Container = std::vector<T>>
class MyPriorityQueue {
private:
    Container c; // 底层容器

private:
    // 上浮操作，调整插入的元素到正确位置（维护最大堆）
    void siftUp(size_t index) {
        while (index > 0) {
            // 若当前节点大于父节点，则交换并继续向上调整
            size_t parent = (index - 1) / 2; // 父节点索引
            if (c[index] <= c[parent]) {
                break;
            }
            std::swap(c[index], c[parent]);
            index = parent;
        }
    }

    void siftDown(size_t index) {
        size_t size = c.size();
        while (true) {
            size_t left = 2 * index + 1; // 左孩子索引
            size_t right = 2 * index + 2; // 右孩子索引
            size_t largest = index; // 假设当前节点最大

            // 找到当前节点、左孩子、右孩子中最大的元素
            if (left < size && c[left] > c[largest]) {
                largest = left;
            }
            if (right < size && c[right] > c[largest]) {
                largest = right;
            }

            // 若最大元素就是当前元素，则停止调整
            if (largest == index) {
                break;
            }

            // 交换当前节点与最大的那个孩子，然后继续向下调整
            std::swap(c[index], c[largest]);
            index = largest;
        }
    }

public:
    // 构造函数，使用初始元素构建堆
    MyPriorityQueue(const Container& cont = Container()) : c(cont) {
        // 从最后一个非叶子节点开始下沉，构建堆
        if (!c.empty()) {
            for (int i = (c.size() - 2) / 2; i >= 0; --i) {
                siftDown(i);
            }
        }
    }

    bool empty() {
        return c.empty();
    }

    size_t size() const {
        return c.size();
    }

    // 访问优先级最高的元素（堆顶）
    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Priority queue is empty");
        }
        return c.front();
    }

    // 插入元素到队列
    void push(const T& value) {
        c.push_back(value); // 添加到容器末尾
        siftUp(c.size() - 1); // 上浮调整
    }

    // 移除优先级最高的元素（堆顶）
    void pop() {
        if (empty()) {
            throw std::out_of_range("Priority queue is empty");
        }
        std::swap(c.front(), c.back()); // 交换堆顶与最后一个元素
        c.pop_back(); // 由于堆顶与最后一个元素交换了，所以此处弹出的最后一个元素，实际就是此前的堆顶
        if (!empty()) {
            siftDown(0); // 下沉调整新的堆顶
        }
    }
}