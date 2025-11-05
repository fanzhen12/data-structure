// reference: https://www.doubao.com/chat/27669708128781826
#include <cstddef> // 用于size_t、ptrdiff_t等类型定义
#include <cstdlib> // 用于malloc、free等内存管理函数
#include <utility>    // std::move、std::forward
#include <type_traits> // std::is_nothrow_move_constructible（可选）

// 前置声明：哈希函数默认实现（脱离std::hash）
template <typename T>
struct DefaultHash;

// 前置声明：相等比较器默认实现（脱离std::equal_to）
template <typename T>
struct DefaultEqual;

// ------- 节点结构：双向链表节点 ------- //
template <typename T>
struct HashNode {
    T data; // 存储的数据
    HashNode* next; // 后继节点
    HashNode* prev; // 前驱节点
    size_t bucket_idx; // 所在桶的索引

    // 复制构造函数（左值）
    explicit HashNode(const T& val, size_t idx) 
        : data(val), prev(nullptr), next(nullptr), bucket_idx(idx) {}

    // 移动构造函数（右值）
    explicit HashNode(T&& val, size_t idx)
        : data(std::move(val)), prev(nullptr), next(nullptr), bucket_idx(idx) {}

    // 原地构造数据（完美转发参数）
    template <typename... Args>
    explicit HashNode(Args&&... args, size_t idx)
        : data(std::forward<Args>(args)...), prev(nullptr), next(nullptr), bucket_idx(idx) {}

};

// ------- 自定义unordered_multiset ------- //
template <typename T,
          typename Hash = DefaultHash<T>,
          typename KeyEqual = DefaultEqual<T>>
class MyUnorderedMultiSet {
public:
    // 类型定义
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    // 迭代器（前向迭代器）
    class iterator {
    private:
        HashNode<T>* m_node; // 指向当前节点的指针
        MyUnorderedMultiSet* m_container; // 指向所属容器的指针
        friend class MyUnorderedMultiSet;

    public:
        using iterator_category = std::forward_iterator_tag; // 这里用到了std中的标签，
        // 什么是std中的标签呢？标签是一种空类型，用于标识迭代器的类别，比如前向迭代器、双向迭代器等。
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t;

        iterator() : m_node(nullptr), m_container(nullptr) {}
        iterator(HashNode<T>* node, MyUnorderedMultiSet* container)
            : m_node(node), m_container(container) {}

        // 移动迭代器
        iterator(iterator&& other) noexcept
            : m_node(other.m_node), m_container(other.m_container) {
            other.m_node = nullptr;
            other.m_container = nullptr;
        }

        reference operator*() const {
            return m_node->data;
        }

        pointer operator->() const {
            return &(m_node->data);
        }
        // 地址就地址吧

        // 前置递增
        iterator& operator++() {
            if (m_node->next) {
                m_node = m_node->next;
            } else {
                size_t current_bucket = m_node->bucket_idx;
                for (size_t i = current_bucket + 1; i < m_container->m_buckets.size(); ++i) {
                    if (m_container->m_buckets[i]) {
                        m_node = m_container->m_buckets[i]; // 这是桶的头节点
                        return *this;
                    }
                }
                m_node = nullptr; // 到达容器末尾
            }
            return *this;
        }

        // 后置递增
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this); // 这里的++调用了上面的前置递增
            return tmp;
        }

        // 比较操作符
        bool operator==(const iterator& other) const {
            return m_node == other.m_node && m_container == other.m_container;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other); // 利用上面的==操作符
        }
    };
};