#include <iostream>
#include <string>

// 1. 产品接口：汽车
class Car {
public:
    virtual void drive() const = 0; // 纯虚函数，定义了产品的共同行为
    virtual ~Car() {} // 虚析构函数，确保子类能被正确销毁
};

// 2. 具体产品A：轿车
class Sedan : public Car {
public:
    void drive() const override {
        std::cout << "驾驶着舒适的轿车在城市里穿行。" << std::endl;
    }
};

// 3. 具体产品B：SUV
class SUV : public Car {
public:
    void drive() const override {
        std::cout << "驾驶着高大的SUV在山路上越野。" << std::endl;
    }
};

// 4. 简单工厂类
class CarFactory {
public:
    // 根据字符串参数创建不同的Car对象
    static Car* createCar(const std::string& type) {
        if (type == "Sedan") {
            return new Sedan();
        } else if (type == "SUV") {
            return new SUV();
        } else {
            std::cout << "不支持的汽车类型: " << type << std::endl;
            return nullptr;
        }
    }
};

// 客户端代码
int main() {
    // 客户端只需要与工厂交互，不需要知道具体的Car子类
    Car* myCar1 = CarFactory::createCar("Sedan");
    if (myCar1) {
        myCar1->drive(); // 输出：驾驶着舒适的轿车在城市里穿行。
        delete myCar1;   // 记得释放内存
    }

    Car* myCar2 = CarFactory::createCar("SUV");
    if (myCar2) {
        myCar2->drive(); // 输出：驾驶着高大的SUV在山路上越野。
        delete myCar2;
    }
    
    // 如果我们想增加一种新的汽车类型，比如"SportsCar"
    // 我们需要：
    // 1. 创建一个 SportsCar 类，继承自 Car。
    // 2. 修改 CarFactory::createCar 方法，增加一个 else if (type == "SportsCar") 的分支。
    // 这违反了开放/封闭原则。

    return 0;
}