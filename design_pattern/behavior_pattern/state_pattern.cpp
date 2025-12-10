#include <iostream>
#include <string>

// 前置声明：上下文类（状态类需要引用上下文以切换状态）
class ElevatorContext;

// ====================== 1. 抽象状态（State）：电梯状态接口 ======================
class ElevatorState {
protected:
    ElevatorContext* context_; // 引用上下文，用于切换状态
public:
    // 构造函数：注入上下文
    explicit ElevatorState(ElevatorContext* context) : context_(context) {}
    virtual ~ElevatorState() = default;

    // 电梯核心行为接口（所有状态必须实现）
    virtual void open() const = 0;    // 开门
    virtual void close() const = 0;   // 关门
    virtual void run() const = 0;     // 运行
    virtual void stop() const = 0;    // 停止
    // 获取状态名称（便于日志）
    virtual std::string getStateName() const = 0;
};

// ====================== 2. 上下文（Context）：电梯控制上下文 ======================
class ElevatorContext {
private:
    ElevatorState* currentState_; // 持有当前状态对象

public:
    // 构造函数：初始状态为“停止状态”
    ElevatorContext();
    ~ElevatorContext() {
        delete currentState_; // 释放状态对象
    }

    // 切换状态（核心：替换当前状态对象）
    void setState(ElevatorState* newState) {
        std::cout << "\n[电梯] 状态切换：" << currentState_->getStateName() 
                  << " → " << newState->getStateName() << std::endl;
        delete currentState_; // 释放旧状态
        currentState_ = newState;
    }

    // 获取当前状态（供状态类访问）
    ElevatorState* getCurrentState() const {
        return currentState_;
    }

    // 对外接口：委托给当前状态处理
    void open() const {
        std::cout << "\n===== 触发操作：开门 =====" << std::endl;
        currentState_->open();
    }

    void close() const {
        std::cout << "\n===== 触发操作：关门 =====" << std::endl;
        currentState_->close();
    }

    void run() const {
        std::cout << "\n===== 触发操作：运行 =====" << std::endl;
        currentState_->run();
    }

    void stop() const {
        std::cout << "\n===== 触发操作：停止 =====" << std::endl;
        currentState_->stop();
    }
};

// ====================== 3. 具体状态（Concrete State） ======================
// 具体状态1：开门状态（OpenState）
class OpenState : public ElevatorState {
public:
    explicit OpenState(ElevatorContext* context) : ElevatorState(context) {}

    void open() const override {
        std::cout << "[" << getStateName() << "] 电梯已开门，无法再次开门！" << std::endl;
    }

    void close() const override {
        std::cout << "[" << getStateName() << "] 执行关门操作..." << std::endl;
        // 关门后切换到“关门状态”
        context_->setState(new CloseState(context_));
    }

    void run() const override {
        std::cout << "[" << getStateName() << "] 电梯开门状态，无法运行！" << std::endl;
    }

    void stop() const override {
        std::cout << "[" << getStateName() << "] 电梯已停止（开门状态），无需停止！" << std::endl;
    }

    std::string getStateName() const override {
        return "开门状态";
    }
};

// 具体状态2：关门状态（CloseState）
class CloseState : public ElevatorState {
public:
    explicit CloseState(ElevatorContext* context) : ElevatorState(context) {}

    void open() const override {
        std::cout << "[" << getStateName() << "] 执行开门操作..." << std::endl;
        // 开门后切换到“开门状态”
        context_->setState(new OpenState(context_));
    }

    void close() const override {
        std::cout << "[" << getStateName() << "] 电梯已关门，无法再次关门！" << std::endl;
    }

    void run() const override {
        std::cout << "[" << getStateName() << "] 执行运行操作..." << std::endl;
        // 运行后切换到“运行状态”
        context_->setState(new RunState(context_));
    }

    void stop() const override {
        std::cout << "[" << getStateName() << "] 执行停止操作..." << std::endl;
        // 停止后切换到“停止状态”
        context_->setState(new StopState(context_));
    }

    std::string getStateName() const override {
        return "关门状态";
    }
};

// 具体状态3：运行状态（RunState）
class RunState : public ElevatorState {
public:
    explicit RunState(ElevatorContext* context) : ElevatorState(context) {}

    void open() const override {
        std::cout << "[" << getStateName() << "] 电梯运行中，禁止开门！" << std::endl;
    }

    void close() const override {
        std::cout << "[" << getStateName() << "] 电梯已关门（运行状态），无需关门！" << std::endl;
    }

    void run() const override {
        std::cout << "[" << getStateName() << "] 电梯已运行，无法再次运行！" << std::endl;
    }

    void stop() const override {
        std::cout << "[" << getStateName() << "] 执行停止操作..." << std::endl;
        // 停止后切换到“停止状态”
        context_->setState(new StopState(context_));
    }

    std::string getStateName() const override {
        return "运行状态";
    }
};

// 具体状态4：停止状态（StopState）
class StopState : public ElevatorState {
public:
    explicit StopState(ElevatorContext* context) : ElevatorState(context) {}

    void open() const override {
        std::cout << "[" << getStateName() << "] 执行开门操作..." << std::endl;
        // 开门后切换到“开门状态”
        context_->setState(new OpenState(context_));
    }

    void close() const override {
        std::cout << "[" << getStateName() << "] 电梯已关门（停止状态），无需关门！" << std::endl;
    }

    void run() const override {
        std::cout << "[" << getStateName() << "] 执行运行操作..." << std::endl;
        // 运行后切换到“运行状态”
        context_->setState(new RunState(context_));
    }

    void stop() const override {
        std::cout << "[" << getStateName() << "] 电梯已停止，无法再次停止！" << std::endl;
    }

    std::string getStateName() const override {
        return "停止状态";
    }
};

// 上下文构造函数：初始化状态为停止状态
ElevatorContext::ElevatorContext() {
    currentState_ = new StopState(this);
    std::cout << "[电梯] 初始化完成，初始状态：" << currentState_->getStateName() << std::endl;
}

// ====================== 4. 客户端代码 ======================
int main() {
    // 创建电梯上下文（客户端无需关心状态类）
    ElevatorContext elevator;

    // 场景1：停止状态 → 开门
    elevator.open();

    // 场景2：开门状态 → 关门
    elevator.close();

    // 场景3：关门状态 → 运行
    elevator.run();

    // 场景4：运行状态 → 尝试开门（禁止）
    elevator.open();

    // 场景5：运行状态 → 停止
    elevator.stop();

    // 场景6：停止状态 → 运行
    elevator.run();

    return 0;
}