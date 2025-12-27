#include <iostream>
#include <string>

// ===================== 实现部分（Implementor）：操作系统接口 =====================
// 抽象的实现层，定义操作系统的核心行为
class OS {
public:
    virtual ~OS() = default; // 虚析构，确保子类析构被调用
    virtual void run() = 0;  // 纯虚函数，运行操作系统
    virtual std::string getName() = 0; // 获取系统名称
};

// 具体实现1：安卓系统
class AndroidOS : public OS {
public:
    void run() override {
        std::cout << "启动安卓系统，流畅运行~" << std::endl;
    }

    std::string getName() override {
        return "安卓";
    }
};

// 具体实现2：iOS系统
class IOS : public OS {
public:
    void run() override {
        std::cout << "启动iOS系统，丝滑体验~" << std::endl;
    }

    std::string getName() override {
        return "iOS";
    }
};

// ===================== 抽象部分（Abstraction）：手机抽象类 =====================
// 抽象层，持有实现层的引用（这就是“桥”）
class Phone {
protected:
    OS* os; // 组合关系：手机持有操作系统的指针（桥接的核心）
    std::string brand; // 手机品牌

public:
    // 构造函数：传入操作系统对象
    Phone(OS* os_, const std::string& brand_) : os(os_), brand(brand_) {}
    virtual ~Phone() { delete os; } // 析构时释放操作系统对象

    // 抽象方法：启动手机
    virtual void boot() = 0;
};

// 扩充抽象1：小米手机
class XiaomiPhone : public Phone {
public:
    // 继承构造函数，指定品牌为“小米”
    XiaomiPhone(OS* os_) : Phone(os_, "小米") {}

    void boot() override {
        std::cout << "===== " << brand << "手机 =====" << std::endl;
        std::cout << "搭载" << os->getName() << "系统：" << std::endl;
        os->run(); // 调用实现层的方法
        std::cout << "======================" << std::endl << std::endl;
    }
};

// 扩充抽象2：苹果手机
class IPhone : public Phone {
public:
    IPhone(OS* os_) : Phone(os_, "苹果") {}

    void boot() override {
        std::cout << "===== " << brand << "手机 =====" << std::endl;
        std::cout << "搭载" << os->getName() << "系统：" << std::endl;
        os->run();
        std::cout << "======================" << std::endl << std::endl;
    }
};

// ===================== 测试代码 =====================
int main() {
    // 场景1：小米手机 + 安卓系统
    Phone* xiaomi_with_android = new XiaomiPhone(new AndroidOS());
    xiaomi_with_android->boot();

    // 场景2：苹果手机 + iOS系统
    Phone* iphone_with_ios = new IPhone(new IOS());
    iphone_with_ios->boot();

    // 场景3（扩展）：小米手机 + iOS（仅演示扩展能力，实际不存在）
    Phone* xiaomi_with_ios = new XiaomiPhone(new IOS());
    xiaomi_with_ios->boot();

    // 释放资源
    delete xiaomi_with_android;
    delete iphone_with_ios;
    delete xiaomi_with_ios;

    return 0;
}