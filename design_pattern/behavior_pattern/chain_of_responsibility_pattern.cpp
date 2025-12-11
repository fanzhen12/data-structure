#include <iostream>
#include <string>
#include <memory>

// 权限请求结构体（统一请求格式）
struct AuthRequest {
    std::string userId;    // 用户ID
    bool isLogin = false;  // 是否登录
    std::string role;      // 角色：admin/user
    std::string operation; // 操作：add/delete/query
};

// ====================== 1. 抽象处理者（Handler）：权限校验处理器 ======================
class AuthHandler {
protected:
    std::shared_ptr<AuthHandler> nextHandler_; // 下一个处理者（智能指针避免内存泄漏）
public:
    virtual ~AuthHandler() = default;

    // 设置下一个处理者（链式调用，便于构建链）
    std::shared_ptr<AuthHandler> setNext(std::shared_ptr<AuthHandler> next) {
        nextHandler_ = next;
        return next; // 返回下一个处理者，支持链式设置
    }

    // 处理请求（核心接口，子类必须实现）
    virtual bool handleRequest(const AuthRequest& req) = 0;

    // 获取处理器名称（便于日志）
    virtual std::string getName() const = 0;
};

// ====================== 2. 具体处理者（Concrete Handler） ======================
// 具体处理者1：登录状态校验
class LoginAuthHandler : public AuthHandler {
public:
    bool handleRequest(const AuthRequest& req) override {
        std::cout << "\n===== 执行【" << getName() << "】=====" << std::endl;
        // 校验登录状态：未登录则直接拒绝，不传递
        if (!req.isLogin) {
            std::cout << "❌ 用户【" << req.userId << "】未登录，拒绝操作！" << std::endl;
            return false;
        }
        std::cout << "✅ 登录状态校验通过！" << std::endl;
        // 登录通过，传递给下一个处理者
        if (nextHandler_) {
            return nextHandler_->handleRequest(req);
        }
        // 无下一个处理者，默认通过
        return true;
    }

    std::string getName() const override {
        return "登录状态校验器";
    }
};

// 具体处理者2：角色权限校验（仅管理员可删除）
class RoleAuthHandler : public AuthHandler {
public:
    bool handleRequest(const AuthRequest& req) override {
        std::cout << "\n===== 执行【" << getName() << "】=====" << std::endl;
        // 校验角色：删除操作仅管理员可执行
        if (req.operation == "delete" && req.role != "admin") {
            std::cout << "❌ 用户【" << req.userId << "】（角色：" << req.role << "）无删除权限！" << std::endl;
            return false;
        }
        std::cout << "✅ 角色权限校验通过！" << std::endl;
        // 角色通过，传递给下一个处理者
        if (nextHandler_) {
            return nextHandler_->handleRequest(req);
        }
        return true;
    }

    std::string getName() const override {
        return "角色权限校验器";
    }
};

// 具体处理者3：操作权限校验（普通用户仅可查询）
class OperationAuthHandler : public AuthHandler {
public:
    bool handleRequest(const AuthRequest& req) override {
        std::cout << "\n===== 执行【" << getName() << "】=====" << std::endl;
        // 校验操作：普通用户仅可查询
        if (req.role == "user" && req.operation != "query") {
            std::cout << "❌ 用户【" << req.userId << "】（普通用户）仅可执行查询操作！" << std::endl;
            return false;
        }
        std::cout << "✅ 操作权限校验通过！" << std::endl;
        // 操作通过，传递给下一个处理者（无则通过）
        if (nextHandler_) {
            return nextHandler_->handleRequest(req);
        }
        return true;
    }

    std::string getName() const override {
        return "操作权限校验器";
    }
};

// ====================== 3. 客户端代码 ======================
int main() {
    // 步骤1：创建具体处理者
    auto loginHandler = std::make_shared<LoginAuthHandler>();
    auto roleHandler = std::make_shared<RoleAuthHandler>();
    auto operationHandler = std::make_shared<OperationAuthHandler>();

    // 步骤2：构建责任链（登录→角色→操作）
    loginHandler->setNext(roleHandler)->setNext(operationHandler);

    // 场景1：未登录用户执行删除操作（登录校验失败）
    std::cout << "---------- 测试场景1：未登录删除 ----------" << std::endl;
    AuthRequest req1{"user1", false, "admin", "delete"};
    loginHandler->handleRequest(req1);

    // 场景2：普通用户执行删除操作（角色校验失败）
    std::cout << "\n---------- 测试场景2：普通用户删除 ----------" << std::endl;
    AuthRequest req2{"user2", true, "user", "delete"};
    loginHandler->handleRequest(req2);

    // 场景3：普通用户执行查询操作（全部通过）
    std::cout << "\n---------- 测试场景3：普通用户查询 ----------" << std::endl;
    AuthRequest req3{"user2", true, "user", "query"};
    loginHandler->handleRequest(req3);

    // 场景4：管理员执行删除操作（全部通过）
    std::cout << "\n---------- 测试场景4：管理员删除 ----------" << std::endl;
    AuthRequest req4{"admin1", true, "admin", "delete"};
    loginHandler->handleRequest(req4);

    // 扩展：动态调整链（跳过角色校验，仅登录+操作校验）
    std::cout << "\n---------- 测试场景5：调整链（跳过角色校验） ----------" << std::endl;
    loginHandler->setNext(operationHandler); // 重新设置链：登录→操作
    AuthRequest req5{"user3", true, "user", "add"}; // 普通用户添加（操作校验失败）
    loginHandler->handleRequest(req5);

    return 0;
}