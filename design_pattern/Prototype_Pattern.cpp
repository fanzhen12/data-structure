// 原型模式
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// 前向声明，因为Image在Document中被使用
class Image;

// ------- 原型接口 ------- //
class Prototype {
public:
    virtual ~Prototype() {}
    virtual Prototype* clone() const = 0; // 克隆方法
};

// ------- 图片类 ------- //
class Image {
public:
    std::string filename_;
    int width_;
    int height_;

public:
    Image(const std::string& filename, int width, int height)
        : filename_(filename), width_(width), height_(height) {
        std::cout << "Image constructor: " << filename_ << std::endl;
    }

    // 拷贝构造函数，用于深拷贝
    Image(const Image& other)
        : filename_(other.filename_), width_(other.width_), height_(other.height_) {
        std::cout << "Image copy constructor: " << filename_ << std::endl;
    }

    ~Image() {
        std::cout << "Image destructor: " << filename_ << std::endl;
    }
};

// ------- 具体原型类: 文档 ------- //
class Document : public Prototype {
private:
    std::string text_;
    std::vector<Image*> images_; // 文档中包含的图片，存储指向Image对象的指针

public:
    Document(const std::string& text) : text_(text) {
        std::cout << "Document constructor." << std::endl;
    }

    // 拷贝构造函数，实现深拷贝
    Document(const Document& other) {
        std::cout << "Document copy constructor." << other.text_ << std::endl;
        this->text_ = other.text_;
        // 深拷贝：为每个Image指针创建一个新的Image对象
        for (Image* img : other.images_) {
            this->images_.push_back(new Image(*img)); // 调用Image的拷贝构造函数，*img为指针解引用，传递Image对象
        }
    }

    ~Document() {
        std::cout << "Document destructor: " << text_ << std::endl;
        // 释放所有Image对象
        for (Image* img : images_) {
            delete img; // 这里是否需要img = nullptr;回答: 不需要，因为img是局部变量，函数结束后会被销毁
        }
        images_.clear();
    }

    // 实现克隆方法
    Prototype* clone() const override {
        std::cout << "Cloning Document: " << text_ << std::endl;
        return new Document(*this); // 使用拷贝构造函数进行深拷贝
    }

    // 添加图片
    void addImage(Image* img) {
        images_.push_back(img);
    }

    // 显示文档内容
    void showContent() const {
        std::cout << "Document Content: " << text_ << std::endl;
        std::cout << "Images in Document:" << std::endl;
        for (const Image* img : images_) {
            std::cout << " - " << img->filename_ << " (" << img->width_ << "x" << img->height_ << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    // 修改文本内容（用于测试克隆的独立性）
    void setText(const std::string& newText) {
        text_ = newText;
    }

    // 修改第一张图片的文件名（用于测试克隆的独立性）
    void modifyFirstImage(const std::string& newFilename) {
        if (!images_.empty()) {
            images_.front()->filename_ = newFilename;
        }
    }
};

// ------- 客户端代码 ------- //
int main() {
    // 1. 创建一个原型文档
    Document* originalDoc = new Document("Design Patterns - Prototype");
    
    // 2. 为原型文档添加一些图片
    originalDoc->addImage(new Image("pattern1.jpg", 800, 600));
    originalDoc->addImage(new Image("pattern2.png", 1024, 768));
    
    std::cout << "\n--- Original Document ---" << std::endl;
    originalDoc->showContent();

    // 3. 克隆原型文档
    Document* clonedDoc = dynamic_cast<Document*>(originalDoc->clone());
    clonedDoc->setText("Cloned Document - Prototype"); // 修改克隆文档的文本

    std::cout << "\n--- Cloned Document (before modification) ---" << std::endl;
    clonedDoc->showContent();

    // 4. 测试深拷贝：修改克隆文档中的图片
    clonedDoc->modifyFirstImage("cloned_pattern1.jpg");

    std::cout << "\n--- Original Document (after modifying clone) ---" << std::endl;
    originalDoc->showContent(); // 原始文档的图片应该保持不变

    std::cout << "\n--- Cloned Document (after modification) ---" << std::endl;
    clonedDoc->showContent();   // 克隆文档的图片应该被修改

    // 5. 清理内存
    delete originalDoc;
    delete clonedDoc;

    return 0;
}