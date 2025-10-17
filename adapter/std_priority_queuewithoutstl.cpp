#include <cstddef> // 用于size_t
#include <stdexcept> // 用于抛出异常
#include <iostream>

template <typename T>
void swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
class MyPriorityQueue {
private:
    T* data;          // 动态数组：存储堆的元素
    size_t size_;     // 当前元素数量
    size_t capacity_; // 数组容量

private:
    // 扩容操作
    void resize() {
        size_t new_capacity = (capacity_ == 0) ? 4 : capacity_ * 2;
        T* new_data = new T[new_capacity];
        
        // 复制旧元素到新数组（手动循环，不依赖库，造轮子，提升水品）
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = data[i];
        }

        delete[] data;
        data = new_data;
        capacity_ = new_capacity;
    }

    // 上浮操作，维护大顶堆    
    void siftUp(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (data[index] <= data[parent]) {
                break;
            }
            swap(data[index], data[parent]);
            index = parent;
        }
    }

    // 下沉操作
    void siftDown(size_t index) {
        while (true) {
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;
            size_t largest = index;

            if (left < size_ && data[left] > data[largest]) {
                left = largest;
            }
            if (right < size_ && data[right] > data[largest]) {
                right = largest;
            }
            if (largest == index) {
                break;
            }
            swap(data[index], data[largest]);
            index = largest;
        }
    }

public:
    MyPriorityQueue() : data(nullptr), size_(0), capacity_(0) {}

    ~MyPriorityQueue() {
        delete[] data;
    }

    // 禁止拷贝构造和拷贝复制
    MyPriorityQueue(const MyPriorityQueue&) = delete;
    MyPriorityQueue& operator=(const MyPriorityQueue&) = delete;

    bool empty() const {
        return size_ == 0;
    }

    size_t size() const {
        return size_;
    }

    const T& top() const {
        if (empty()) {
            throw::out_of_range("Priority queue is empty");
        }
        return data[0];
    }

    void push(const T& value) {
        if (size_ >= capacity_) {
            resize();
        }
        data[size_] = value;
        siftUp(size_);
        size_++;
    }

    void pop() {
        if (empty()) {
            throw::out_of_range("Priority queue is empty");
        }
        swap(data[0], data[size_ - 1]);
        size_--;
        if (!empty()) {
            siftDown(0);
        }
    }
};

// 测试代码
int main() {
    MyPriorityQueue<int> pq;

    pq.push(30);
    pq.push(10);
    pq.push(50);
    pq.push(20);

    std::cout << "元素数量: " << pq.size() << std::endl; // 4
    std::cout << "当前最大元素: " << pq.top() << std::endl; // 50

    pq.pop();
    std::cout << "弹出后最大元素: " << pq.top() << std::endl; // 30

    pq.push(40);
    std::cout << "插入40后最大元素: " << pq.top() << std::endl; // 40

    std::cout << "弹出所有元素: ";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    // 输出：40 30 20 10 

    return 0;
}
