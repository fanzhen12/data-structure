#include <iostream>
#include <string>

// 1. 产品接口：汽车 (与简单工厂相同)
class Car {
public:
    virtual void drive() const = 0;
    virtual ~Car() {}
};

// 2. 具体产品A：轿车 (与简单工厂相同)
class Sedan : public Car {
public:
    void drive() const override {
        std::cout << "驾驶着舒适的轿车在城市里穿行。" << std::endl;
    }
};

// 3. 具体产品B：SUV (与简单工厂相同)
class SUV : public Car {
public:
    void drive() const override {
        std::cout << "驾驶着高大的SUV在山路上越野。" << std::endl;
    }
};

// 4. 工厂接口
class CarFactory {
public:
    virtual Car* createCar() const = 0; // 工厂方法，返回一个Car对象
    virtual ~CarFactory() {}
};

// 5. 具体工厂A：生产轿车的工厂
class SedanFactory : public CarFactory {
public:
    Car* createCar() const override {
        return new Sedan();
    }
};

// 6. 具体工厂B：生产SUV的工厂
class SUVFactory : public CarFactory {
public:
    Car* createCar() const override {
        return new SUV();
    }
};

// 客户端代码
int main() {
    // 客户端需要知道具体的工厂
    CarFactory* sedanFactory = new SedanFactory();
    Car* myCar1 = sedanFactory->createCar();
    myCar1->drive(); // 输出：驾驶着舒适的轿车在城市里穿行。
    delete myCar1;
    delete sedanFactory;

    CarFactory* suvFactory = new SUVFactory();
    Car* myCar2 = suvFactory->createCar();
    myCar2->drive(); // 输出：驾驶着高大的SUV在山路上越野。
    delete myCar2;
    delete suvFactory;

    // 如果我们想增加一种新的汽车类型，比如"SportsCar"
    // 我们只需要：
    // 1. 创建一个 SportsCar 类，继承自 Car。
    // 2. 创建一个 SportsCarFactory 类，继承自 CarFactory，并实现 createCar() 方法。
    // 客户端代码可以直接使用 new SportsCarFactory() 来创建新的产品，无需修改任何已有代码。
    // 这完全符合开放/封闭原则。

    return 0;
}