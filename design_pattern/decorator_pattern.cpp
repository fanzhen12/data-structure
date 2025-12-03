#include <iostream>
#include <string>

// 1. 抽象组件（Component）：咖啡的统一接口
class Coffee {
public:
    virtual ~Coffee() {}
    // 获取咖啡描述
    virtual std::string getDescription() const = 0;
    // 获取咖啡价格
    virtual double getCost() const = 0;
};

// 1.1 具体组件（Concrete Component）：基础咖啡-浓缩咖啡
class Espresso : public Coffee {
public:
    std::string getDescription() const override {
        return "浓缩咖啡";
    }
    
    double getCost() const override {
        return 25.0; // 基础价格
    }
};

// 1.2 具体组件（Concrete Component）：基础咖啡-拿铁咖啡
class Latte : public Coffee {
public:
    std::string getDescription() const override {
        return "拿铁咖啡";
    }
    
    double getCost() const override {
        return 30.0; // 基础价格
    }
};

// 2. 抽象装饰器（Decorator）：咖啡配料的基类
class CoffeeDecorator : public Coffee {
protected:
    Coffee* coffee_; // 持有被装饰的咖啡对象（核心：组合关系）

public:
    // 构造函数，传入被装饰的咖啡对象
    CoffeeDecorator(Coffee* coffee) : coffee_(coffee) {}

    // 析构函数，释放被装饰的咖啡对象
    ~CoffeeDecorator() {
        delete coffee_;
    }
};

// 2.1 具体装饰器（Concrete Decorator）：加牛奶
class MilkDecorator : public CoffeeDecorator {
public:
    MilkDecorator(Coffee* coffee) : CoffeeDecorator(coffee) {}

    std::string getDescription() const override {
        return coffee_->getDescription() + " + 牛奶";
    }

    double getCost() const override {
        return coffee_->getCost() + 5.0; // 牛奶价格
    }
};

// 2.2 具体装饰器（Concrete Decorator）：加糖
class SugarDecorator : public CoffeeDecorator {
public:
    SugarDecorator(Coffee* coffee) : CoffeeDecorator(coffee) {}

    std::string getDescription() const override {
        return coffee_->getDescription() + " + 糖";
    }

    double getCost() const override {
        return coffee_->getCost() + 3.0; // 糖价格
    }
};

// 2.3 具体装饰器（Concrete Decorator）：加奶泡
class WhippedCreamDecorator : public CoffeeDecorator {
public:
    WhippedCreamDecorator(Coffee* coffee) : CoffeeDecorator(coffee) {}

    std::string getDescription() const override {
        return coffee_->getDescription() + " + 奶泡";
    }

    double getCost() const override {
        return coffee_->getCost() + 8.0; // 奶泡价格
    }
};

int main() {
    // 场景1：纯浓缩咖啡（无装饰）
    Coffee* espresso = new Espresso();
    std::cout << espresso->getDescription() << " 价格: " << espresso->getCost() << " 元" << std::endl;
    delete espresso;

    // 场景2：拿铁咖啡 + 牛奶 + 糖(动态添加2种装饰)
    Coffee* latteWithMildAndSugar = new SugarDecorator(new MilkDecorator(new Latte()));
    std::cout << latteWithMildAndSugar->getDescription() << " 价格: " << latteWithMildAndSugar->getCost() << " 元" << std::endl;
    delete latteWithMildAndSugar; // 递归释放所有装饰器和基础对象

    // 场景3：浓缩咖啡 + 牛奶 + 糖 + 奶泡(动态添加3种装饰)
    Coffee* superCoffee = new SugarDecorator(new MilkDecorator(new WhippedCreamDecorator(new Espresso())));
    std::cout << superCoffee->getDescription() << " 价格: " << superCoffee->getCost() << " 元" << std::endl;
    delete superCoffee; // 递归释放所有装饰器和基础对象
}