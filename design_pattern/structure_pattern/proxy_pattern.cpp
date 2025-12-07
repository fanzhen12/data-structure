#include <iostream>
#include <string>
#include <thread>

// 1. 抽象主题（Subject）：图片接口
class Image {
public:
    virtual ~Image() {}
    virtual void display() const = 0; // 显示图片
};

// 2. 真实主题（Real Subject）：实际图片类（加载耗时）
class RealImage : public Image {
private:
    std::string filename_; // 图片文件名

private:
    // 加载图片（耗时操作，比如读取文件、解码）
    void loadImageFromDisk() const {
        std::cout << "[RealImage] 正在加载图片：" << filename_ << std::endl;
        // 模拟耗时
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

public:
    RealImage(const std::string& filename) : filename_(filename) {
        // 构造时不加载图片（懒加载核心）
        std::cout << "[RealImage] 图片对象创建（未加载）：" << filename_ << std::endl;
    }

    void display() const override {
        // 显示前确保图片已加载（通过等待加载完成）
        loadImageFromDisk();
        std::cout << "[RealImage] 显示图片：" << filename_ << std::endl;
    }
};

// 3. 代理主题（Proxy Subject）：图片代理（懒加载）
class ProxyImage : public Image {
private:
    std::string filename_; // 图片文件名
    mutable RealImage* realImage_; // 持有真实对象的引用（初始为nullptr）

private:
    void lazyload() const {
        if (realImage_ == nullptr) {
            realImage_ = new RealImage(filename_); // 此时还没有加载图片
        }
    }

public:
    ProxyImage(const std::string& filename) : filename_(filename), realImage_(nullptr) {
        std::cout << "[ProxyImage] 代理对象创建：" << filename_ << std::endl;
    }

    ~ProxyImage() {
        delete realImage_;
    }

    void display() const override {
        std::cout << "[ProxyImage] 准备显示图片..." << std::endl;
        lazyload(); // 需要时才加载真实图片
        realImage_->display(); // 委托给真实对象显示图片
    }
};

// 客户端代码
int main() {
    // 1. 创建代理对象（此时不会创建RealImage，也不加载图片）
    Image* image1 = new ProxyImage("photo1.jpg");
    Image* image2 = new ProxyImage("photo2.png");

    std::cout << "\n===== 第一次显示 photo1.jpg =====" << std::endl;
    image1->display(); // 首次调用：创建RealImage + 加载 + 显示

    std::cout << "\n===== 第二次显示 photo1.jpg =====" << std::endl;
    image1->display(); // 复用已创建的RealImage，直接显示（无需重新加载）

    std::cout << "\n===== 第一次显示 photo2.png =====" << std::endl;
    image2->display(); // 首次调用：创建RealImage + 加载 + 显示

    // 清理
    delete image1;
    delete image2;

    return 0;
}
