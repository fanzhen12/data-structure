#include <iostream>
#include <utility> // 用于std::pair

// 颜色枚举
enum class Color { RED, BLACK };

template <typename Key, typename Value>
struct Node {
    Key key;     // 键
    Value value; // 值
    Color color; // 节点颜色
    Node* left;  // 左孩子
    Node* right; // 右孩子
    Node* parent; // 父节点

    Node(const Key& k, const Value& v) : 
        key(k), value(v), color(Color::RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

template <typename Key, typename Value>
class MyMap {
private:
    using NodeType = Node<Key, Value>;
    NodeType* root; // 根节点
    NodeType* nil;  // 哨兵节点（代替nullptr，简化边界处理）

    // 左旋操作
    void leftRotate(NodeType* x) {
        NodeType* y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;

        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }
};
