#include <iostream>
#include <string>

// ====================== 1. 抽象策略（Strategy）：支付策略接口 ======================
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() {}
    // 统一的支付接口
    virtual void pay(double amount) const = 0;
    // 可选：返回策略名称（便于日志/展示）
    virtual std::string getName() const = 0;
};

// ====================== 2. 具体策略（Concrete Strategy） ======================
// 具体策略1：微信支付
class WechatPay : public PaymentStrategy {
public:
    void pay(double amount) const override {
        // 模拟微信支付的核心逻辑（签名、调接口等）
        std::cout << "[微信支付] 发起支付请求，金额：" << amount << "元" << std::endl;
        std::cout << "[微信支付] 支付成功！" << std::endl;
    }

    std::string getName() const override {
        return "微信支付";
    }
};

// 具体策略2：支付宝支付
class Alipay : public PaymentStrategy {
public:
    void pay(double amount) const override {
        std::cout << "[支付宝支付] 发起支付请求，金额：" << amount << "元" << std::endl;
        std::cout << "[支付宝支付] 支付成功！" << std::endl;
    }

    std::string getName() const override {
        return "支付宝支付";
    }
};

// 具体策略3：银行卡支付
class BankPay : public PaymentStrategy {
public:
    void pay(double amount) const override {
        std::cout << "[银行卡支付] 发起支付请求，金额：" << amount << "元" << std::endl;
        std::cout << "[银行卡支付] 支付成功！" << std::endl;
    }

    std::string getName() const override {
        return "银行卡支付";
    }
};

// 扩展：新增云闪付（无需修改任何原有代码，符合开闭原则）
class UnionPay : public PaymentStrategy {
public:
    void pay(double amount) const override {
        std::cout << "[云闪付] 发起支付请求，金额：" << amount << "元" << std::endl;
        std::cout << "[云闪付] 支付成功！" << std::endl;
    }

    std::string getName() const override {
        return "云闪付";
    }
};

// ====================== 3. 上下文（Context）：支付上下文 ======================
class PaymentContext {
private:
    PaymentStrategy* strategy_; // 持有抽象策略的引用
    std::string userName_;      // 额外上下文信息（比如用户名，非必须）

public:
    // 构造函数：注入策略和上下文信息
    PaymentContext(PaymentStrategy* strategy, const std::string& userName) 
        : strategy_(strategy), userName_(userName) {}

    // 提供统一的支付入口（客户端只需调用这个方法）
    void doPay(double amount) const {
        std::cout << "===== 【" << userName_ << "】的支付流程 =====" << std::endl;
        std::cout << "选择支付方式：" << strategy_->getName() << std::endl;
        strategy_->pay(amount); // 调用策略的支付方法
        std::cout << "==========================" << std::endl << std::endl;
    }

    // 支持运行时动态切换策略
    void setStrategy(PaymentStrategy* newStrategy) {
        strategy_ = newStrategy;
    }
};

// ====================== 4. 客户端代码 ======================
int main() {
    // 步骤1：创建具体策略对象
    PaymentStrategy* wechat = new WechatPay();
    PaymentStrategy* alipay = new Alipay();
    PaymentStrategy* unionPay = new UnionPay();

    // 步骤2：创建上下文，注入策略（用户：张三）
    PaymentContext* context = new PaymentContext(wechat, "张三");

    // 场景1：张三用微信支付100元
    context->doPay(100.0);

    // 场景2：张三临时切换为支付宝支付200元（动态切换策略）
    context->setStrategy(alipay);
    context->doPay(200.0);

    // 场景3：张三用云闪付支付150元
    context->setStrategy(unionPay);
    context->doPay(150.0);

    // 清理内存
    delete context;
    delete wechat;
    delete alipay;
    delete unionPay;

    return 0;
}