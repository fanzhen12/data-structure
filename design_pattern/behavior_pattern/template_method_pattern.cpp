#include <iostream>
#include <string>

// ====================== 1. 抽象类（Abstract Class）：饮品制作模板 ======================
class Beverage {
protected:
    // ===== 通用方法：所有饮品共享，抽象类实现 =====
    // 1. 烧开水（通用步骤，固定实现）
    void boilWater() const {
        std::cout << "[通用步骤] 烧开水（100℃）" << std::endl;
    }

    // 2. 倒杯子（通用步骤，固定实现）
    void pourInCup() const {
        std::cout << "[通用步骤] 将饮品倒入杯子" << std::endl;
    }

    // ===== 抽象方法：子类必须实现（可变步骤） =====
    // 3. 冲饮品（咖啡/茶不同）
    virtual void brew() const = 0;

    // 4. 加调料（咖啡/茶不同）
    virtual void addCondiments() const = 0;

    // ===== 钩子方法（Hook）：可选覆盖（默认加调料） =====
    // 钩子：客户是否需要加调料（子类可覆盖，比如客户要无糖咖啡）
    virtual bool customerWantsCondiments() const {
        return true; // 默认需要加调料
    }

public:
    // ===== 模板方法（Template Method）：固定算法骨架 =====
    // C++中无final，用public+不可重写（不声明virtual）保证流程不被修改
    void prepareRecipe() const {
        // 步骤1：烧开水（通用）
        boilWater();
        // 步骤2：冲饮品（子类实现）
        brew();
        // 步骤3：倒杯子（通用）
        pourInCup();
        // 步骤4：加调料（钩子控制是否执行）
        if (customerWantsCondiments()) {
            addCondiments();
        }
        std::cout << "===== 饮品制作完成！=====\n" << std::endl;
    }

    // 虚析构函数（确保子类析构正常）
    virtual ~Beverage() = default;
};

// ====================== 2. 具体子类（Concrete Class） ======================
// 具体子类1：咖啡
class Coffee : public Beverage {
protected:
    // 实现抽象方法：冲咖啡
    void brew() const override {
        std::cout << "[咖啡步骤] 用沸水冲泡咖啡粉" << std::endl;
    }

    // 实现抽象方法：加奶糖
    void addCondiments() const override {
        std::cout << "[咖啡步骤] 加入牛奶和方糖" << std::endl;
    }

    // 可选覆盖钩子：模拟“无糖咖啡”需求（返回false则不加调料）
    // 注释掉则使用父类默认值（true）
    // bool customerWantsCondiments() const override {
    //     return false; // 无糖咖啡，不加调料
    // }

public:
    // 咖啡名称（便于展示）
    std::string getName() const {
        return "咖啡";
    }
};

// 具体子类2：茶
class Tea : public Beverage {
protected:
    // 实现抽象方法：冲茶
    void brew() const override {
        std::cout << "[茶步骤] 用80℃热水冲泡茶叶" << std::endl;
    }

    // 实现抽象方法：加柠檬
    void addCondiments() const override {
        std::cout << "[茶步骤] 加入柠檬片" << std::endl;
    }

public:
    // 茶名称（便于展示）
    std::string getName() const {
        return "茶";
    }
};

// 扩展子类3：无糖咖啡（覆盖钩子方法）
class SugarFreeCoffee : public Coffee {
protected:
    // 覆盖钩子：不加调料
    bool customerWantsCondiments() const override {
        return false;
    }

public:
    std::string getName() const {
        return "无糖咖啡";
    }
};

// ====================== 3. 客户端代码 ======================
// 通用制作饮品的函数（依赖抽象类，符合里氏替换）
void makeBeverage(const Beverage& beverage, const std::string& name) {
    std::cout << "\n------ 开始制作" << name << " ------" << std::endl;
    beverage.prepareRecipe();
}

int main() {
    // 场景1：制作普通咖啡
    Coffee coffee;
    makeBeverage(coffee, coffee.getName());

    // 场景2：制作茶
    Tea tea;
    makeBeverage(tea, tea.getName());

    // 场景3：制作无糖咖啡（覆盖钩子方法）
    SugarFreeCoffee sugarFreeCoffee;
    makeBeverage(sugarFreeCoffee, sugarFreeCoffee.getName());

    return 0;
}