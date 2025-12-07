#include <iostream>
#include <string>
using namespace std;

// ------- 产品类 ------- //
class Computer {
public:
    void setCPU(const string& cpu) { this->cpu = cpu; }
    void setMemory(const string& memory) { this->memory = memory; }
    void setHardDisk(const string& hardDisk) { this->hardDisk = hardDisk; }
    void setGraphics(const string& graphics) { this->graphics = graphics; }

    // 显示电脑配置
    void showConfig() const {
        cout << "电脑配置：" << endl;
        cout << "CPU：" << cpu << endl;
        cout << "内存：" << memory << endl;
        cout << "硬盘：" << hardDisk << endl;
        cout << "显卡：" << graphics << endl;
        cout << "-------------------------" << endl;
    }

private:
    string cpu;       // 处理器
    string memory;    // 内存
    string hardDisk;  // 硬盘
    string graphics;  // 显卡
};

// ------- 抽象建造者类 ------- //
class ComputerBuilder {
public:
    virtual ~ComputerBuilder() = default;

    // 构建各部件的纯虚方法
    virtual void buildCPU() = 0;
    virtual void buildMemory() = 0;
    virtual void buildHardDisk() = 0;
    virtual void buildGraphics() = 0;

    // 返回构建完成的电脑
    virtual Computer* getComputer() = 0;
};

// ------- 具体建造者类 ------- //
// 具体建造者1：游戏本建造者（高性能配置）
class GameComputerBuilder : public ComputerBuilder {
public:
    GameComputerBuilder() {
        // 初始化空电脑对象
        computer = new Computer();
    }

    ~GameComputerBuilder() override {
        delete computer;
    }

    void buildCPU() override {
        computer->setCPU("Intel i9-14900K");
    }

    void buildMemory() override {
        computer->setMemory("32GB DDR5 6400MHz");
    }

    void buildHardDisk() override {
        computer->setHardDisk("2TB NVMe SSD");
    }

    void buildGraphics() override {
        computer->setGraphics("NVIDIA RTX 4090");
    }

    Computer* getComputer() override {
        return computer;
    }

private:
    Computer* computer; // 持有产品实例
};

// 具体建造者2：办公本建造者（轻量化配置）
class OfficeComputerBuilder : public ComputerBuilder {
public:
    OfficeComputerBuilder() {
        computer = new Computer();
    }

    ~OfficeComputerBuilder() override {
        delete computer;
    }

    void buildCPU() override {
        computer->setCPU("Intel i5-13400U");
    }

    void buildMemory() override {
        computer->setMemory("16GB DDR4 3200MHz");
    }

    void buildHardDisk() override {
        computer->setHardDisk("1TB SATA SSD");
    }

    void buildGraphics() override {
        computer->setGraphics("Intel UHD Graphics");
    }

    Computer* getComputer() override {
        return computer;
    }

private:
    Computer* computer;
};

// ------- 指挥者类 ------- //
// 指挥者：电脑组装师
class ComputerDirector {
public:
    // 构造函数传入建造者（依赖抽象，而非具体）
    ComputerDirector(ComputerBuilder* builder) : builder(builder) {}

    // 组装电脑的固定流程
    void constructComputer() {
        builder->buildCPU();
        builder->buildMemory();
        builder->buildHardDisk();
        builder->buildGraphics();
    }

private:
    ComputerBuilder* builder;
};

// ------- 客户端代码 ------- //
int main() {
    // 1. 构建游戏本
    ComputerBuilder* gameBuilder = new GameComputerBuilder();
    ComputerDirector* gameDirector = new ComputerDirector(gameBuilder);
    gameDirector->constructComputer(); // 按流程组装
    Computer* gameComputer = gameBuilder->getComputer();
    gameComputer->showConfig();

    // 2. 构建办公本
    ComputerBuilder* officeBuilder = new OfficeComputerBuilder();
    ComputerDirector* officeDirector = new ComputerDirector(officeBuilder);
    officeDirector->constructComputer();
    Computer* officeComputer = officeBuilder->getComputer();
    officeComputer->showConfig();

    // 释放资源
    delete gameDirector;
    delete gameBuilder;
    delete officeDirector;
    delete officeBuilder;

    return 0;
}