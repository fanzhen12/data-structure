#include <iostream>
#include <string>

// ====================== 1. 子系统类（Subsystem Classes） ======================
// 子系统1：灯光
class Light {
public:
    void turnOn() const {
        std::cout << "[Light] 灯光已打开（亮度100%）" << std::endl;
    }

    void turnOff() const {
        std::cout << "[Light] 灯光已关闭" << std::endl;
    }

    void dim(int brightness) const {
        std::cout << "[Light] 灯光调暗至 " << brightness << "%" << std::endl;
    }
};

// 子系统2：空调
class AirConditioner {
public:
    void turnOn() const {
        std::cout << "[AirConditioner] 空调已打开" << std::endl;
    }

    void turnOff() const {
        std::cout << "[AirConditioner] 空调已关闭" << std::endl;
    }

    void setTemperature(int temp) const {
        std::cout << "[AirConditioner] 空调温度调至 " << temp << "℃" << std::endl;
    }
};

// 子系统3：窗帘
class Curtain {
public:
    void open() const {
        std::cout << "[Curtain] 窗帘已拉开" << std::endl;
    }

    void close() const {
        std::cout << "[Curtain] 窗帘已关闭" << std::endl;
    }
};

// 子系统4：电视
class TV {
public:
    void turnOn() const {
        std::cout << "[TV] 电视已打开（频道：客厅影院）" << std::endl;
    }

    void turnOff() const {
        std::cout << "[TV] 电视已关闭" << std::endl;
    }
};

// ====================== 2. 外观类（Facade） ======================
class SmartHomeFacade {
private:
    // 持有所有子系统的引用（外观知道所有子系统）
    Light* light_;
    AirConditioner* ac_;
    Curtain* curtain_;
    TV* tv_;

public:
    // 构造函数：初始化所有子系统
    SmartHomeFacade() {
        light_ = new Light();
        ac_ = new AirConditioner();
        curtain_ = new Curtain();
        tv_ = new TV();
    }

    // 析构函数：释放子系统资源
    ~SmartHomeFacade() {
        delete light_;
        delete ac_;
        delete curtain_;
        delete tv_;
    }

    // 简化接口1：回家模式（一键执行多个子系统操作）
    void homeMode() const {
        std::cout << "\n===== 执行【回家模式】=====" << std::endl;
        light_->turnOn();          // 打开灯光
        ac_->turnOn();             // 打开空调
        ac_->setTemperature(26);   // 调至26℃
        curtain_->open();          // 拉开窗帘
        tv_->turnOn();             // 打开电视
    }

    // 简化接口2：离家模式
    void leaveHomeMode() const {
        std::cout << "\n===== 执行【离家模式】=====" << std::endl;
        light_->turnOff();         // 关闭灯光
        ac_->turnOff();            // 关闭空调
        curtain_->close();         // 关闭窗帘
        tv_->turnOff();            // 关闭电视
    }

    // 简化接口3：睡眠模式
    void sleepMode() const {
        std::cout << "\n===== 执行【睡眠模式】=====" << std::endl;
        light_->dim(10);           // 灯光调暗至10%
        ac_->setTemperature(28);   // 空调调至28℃
        curtain_->close();         // 关闭窗帘
        tv_->turnOff();            // 关闭电视
    }

    // 可选：保留直接操作子系统的入口（灵活性）
    Light* getLight() const { return light_; }
    AirConditioner* getAirConditioner() const { return ac_; }
};

// ====================== 3. 客户端代码（Client） ======================
int main() {
    // 客户端只需要创建外观对象，无需关心子系统
    SmartHomeFacade* smartHome = new SmartHomeFacade();

    // 场景1：下班回家，执行回家模式
    smartHome->homeMode();

    // 场景2：准备睡觉，执行睡眠模式
    smartHome->sleepMode();

    // 场景3：出门上班，执行离家模式
    smartHome->leaveHomeMode();

    // 可选：客户端仍可直接操作子系统（保留灵活性）
    std::cout << "\n===== 手动调整空调温度 =====" << std::endl;
    smartHome->getAirConditioner()->setTemperature(25);

    // 清理
    delete smartHome;

    return 0;
}