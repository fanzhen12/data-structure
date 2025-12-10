#include <iostream>
#include <string>
#include <stdexcept>

// 1. 抽象迭代器
class Iterator {
public:
    virtual ~Iterator() = default;
    virtual bool hasNext() const = 0;
    virtual bool next() = 0;
    virtual int currentItem() const = 0;
    virtual void reset() = 0;
};

// 2. 抽象聚合
class Collection {
public:
    virtual ~Collection() = default;
    virtual void add(int num) = 0;
    virtual int size() const = 0;
    virtual Iterator* createIterator() const = 0;
    virtual std::string getName() const = 0;
};

// 3.1 具体迭代器：数组迭代器
class ArrayCollection;
class ArrayIterator : public Iterator {
private:
    ArrayCollection* collection_;
    int currentIndex_ = 0;

public:
    explicit ArrayIterator(ArrayCollection* collection) : collection_(collection) {}
    bool hasNext() const override {
        return currentIndex_ < collection_->size();
    }

    bool next() override {
        if (hasNext()) {
            currentIndex_++;
            return true;
        }
        return false;
    }

    int currentItem() const override {};

    void reset() override {
        currentIndex_ = 0;
    }
};

// 3.2 具体聚合：数组集合
class ArrayCollection : public Collection {
private:
    static const int MAX_SIZE = 100;
    int arr_[MAX_SIZE] = {0};
    int size_ = 0;

public:
    void add(int num) override {
        if (size_ < MAX_SIZE) {
            arr_[size_++] = num;
        } else {
            std::cout << "数组已满，无法添加更多元素！" << std::endl;
        }
    }

    int size() const override {
        return size_;
    }

    Iterator* createIterator() const override {
        return new ArrayIterator(const_cast<ArrayCollection*>(this));
    }

    std::string getName() const override {
        return "数组集合";
    }

    int getElement(int index) const {
        if (index > 0 && index < size_) {
            return arr_[index];
        }
        throw std::out_of_range("索引越界");
    }
};

// 4.1 具体迭代器：链表迭代器
struct ListNode {
    int val;
    ListNode* next;
    
    ListNode(int v) : val(v), next(nullptr) {}
    ~ListNode() { delete next; }
};

class ListCollection;
class ListIterator : public Iterator {
private:
    ListCollection* collection_;
    ListNode* currentNode_;

public:
    explicit ListIterator(ListCollection* collection) : collection_(collection), currentNode_(collection->getHead()) {}

    bool hasNext() const override {
        return currentNode_ != nullptr;
    }

    bool next() override {
        if (hasNext()) {
            currentNode_ = currentNode_->next;
            return true;
        }
        return false;
    }

    int currentItem() const override {
        if (hasNext()) {
            return currentNode_->val;
        }
        throw std::out_of_range("当前无有效元素");
    }

    void reset() override {
        currentNode_ = collection_->getHead();
    }
};

// 4.2 具体聚合：链表集合
class ListCollection : public Collection {
private:
    ListNode* head_ = nullptr;
    int size_ = 0;

public:
    ~ListCollection() {
        delete head_;
    }

    void add(int num) override {
        ListNode* newNode = new ListNode(num);
        if (!head_) {
            head_ = newNode;
        } else {
            ListNode* cur = head_;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = newNode;
        }
        size_++;
    }

    int size() const override {
        return size_;
    }

    Iterator* createIterator() const override {
        return new ListIterator(const_cast<ListCollection*>(this));
    }

    std::string getName() const override {
        return "链表集合";
    }

    ListNode* getHead() const {
        return head_;
    }
};

// 5. 客户端代码
void traverseCollection(const Collection& collection) {
    std::cout << "\n===== 遍历【" << collection.getName() << "】=====" << std::endl;
    Iterator* it = collection.createIterator();
    try {
        while (it->hasNext()) {
            std::cout << it->currentItem() << " ";
            it->next();
        }
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "遍历过程中发生异常: " << e.what() << std::endl;
    }
    delete it;
}

int main() {
    // 场景1：使用数组集合
    ArrayCollection arrayCol;
    arrayCol.add(10);
    arrayCol.add(20);
    arrayCol.add(30);
    traverseCollection(arrayCol);

    // 场景2：使用链表集合
    ListCollection listCol;
    listCol.add(100);
    listCol.add(200);
    listCol.add(300);
    traverseCollection(listCol);

    return 0;
}