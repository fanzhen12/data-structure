// ------- 饿汉式单例模式 ------- //
/*
    * 优点：实现简单，天然线程安全（静态变量在程序启动时初始化，只执行一次）。
    * 缺点：
        浪费资源：如果程序全程没用到这个实例，它也会被创建（比如大型程序中的某个模块，可能用户没触发，但实例已存在）。
        无法延迟初始化：如果实例创建需要依赖其他模块的初始化（比如配置文件路径需要先读取），饿汉式可能会出问题（因为静态变量初始化顺序不确定）。
*/
#include <iostream>
#include <string>

class ConfigManager {
private:
    // 1. 私有构造函数，防止外部实例化
    ConfigManager() {
        std::cout << "ConfigManager 实例创建成功（饿汉式）" << std::endl;;
        loadConfig();
    }

    // 2. 私有拷贝构造函数和赋值运算符，防止拷贝
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // 3. 静态成员变量，持有唯一实例
    static ConfigManager instance_;

    // 成员变量：模拟配置数据
    std::string appName_;
    int appPort_;

    // 私有方法：加载配置（模拟）
    void loadConfig() {
        appName_ = "MyApp";
        appPort_ = 8080;
    }

public:
    // 4. 公共静态方法：全局访问点
    static ConfigManager& getInstance() {
        return instance_;
    }

    // 公共方法：获取配置
    std::string getAppName() const {
        return appName_;
    }
    int getAppPort() const {
        return appPort_;
    }
};

// 关键：在类外初始化静态成员（程序启动时执行，保证唯一实例）
ConfigManager ConfigManager::instance_;

// 测试饿汉式单例模式
int main() {
    // 第一次获取实例（已创建）
    ConfigManager& config1 = ConfigManager::getInstance();
    std::cout << "AppName: " << config1.getAppName() << ", Port: " << config1.getAppPort() << std::endl;

    // 第二次获取实例（同一实例）
    ConfigManager& config2 = ConfigManager::getInstance();
    std::cout << "是否为同一实例：" << (&config1 == &config2 ? "是" : "否") << std::endl;

    // 尝试拷贝（编译错误，已被删除）
    // ConfigMangager config3 = config1; // 编译错误

    return 0;
}

// ------- 懒汉式单例模式 ------- //
/**
 * 用到时才创建实例（延迟初始化），避免资源浪费。
 * 需要考虑线程安全问题（多线程可能同时创建实例）。
 * 双重检查锁定（DCL）:第一次检查避免频繁加锁，第二次检查防止多线程重复创建实例（核心面试考点）。
*/

#include <iostream>
#include <string>
#include <mutex>

class LogManager {
private:
    // 1. 私有构造函数
    LogManager() {
        std::cout << "LogManager 实例创建成功（懒汉式）" << std::endl;
        initLogFile();
    }

    // 2. 禁用拷贝构造和赋值运算符
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    // 3. 静态成员变量：存储唯一实例（指针，初始化为nullptr）
    static LogManager* instance_;

    // 4. 静态互斥锁，保证线程安全
    static std::mutex mutex_;

    // 成员变量：模拟日志配置
    std::string logFile_;

    // 私有方法：初始化日志系统（模拟）
    void initLogFile() {
        logFile_ = "app.log";
    }

public:
    // 5. 公共静态方法：全局访问点（双重检查锁定）
    static LogManager* getInstance() {
        // 第一次检查：避免每次调用都加锁（提升性能）第一次检查本身是「非线程安全」的 —— 多线程可能同时通过这层检查，进入后续的加锁逻辑,这也是为什么需要第二次检查
        if (instance_ == nullptr) {
            // 加锁，保证只有一个线程进入临界区
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：防止多个线程同时通过第一次检查
            if (instance_ == nullptr) {
                instance_ = new LogManager();
            }
        }
        return instance_;
    }

    // 公共方法：获取日志文件
    std::string getLogFile() const {
        return logFile_;
    }

    // 6. 静态方法：销毁实例（可选，避免内存泄漏）
    static void destroyInstance() {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};

// 类外初始化静态成员（必须初始化，否则链接错误）
LogManager* LogManager::instance_ = nullptr;
std::mutex LogManager::mutex_; // 初始化静态互斥锁

// 测试代码（多线程环境下使用懒汉式单例模式）
#include <thread>
void testLogManager() {
    LogManager* log = LogManager::getInstance();
    std::cout << "线程 " << std::this_thread::get_id() << " 获取日志文件：" << log->getLogFile() << std::endl;
}

int main() {
    std::thread t1(testLogManager);
    std::thread t2(testLogManager);
    std::thread t3(testLogManager);

    t1.join();
    t2.join();
    t3.join();

    // 销毁实例（程序退出前）
    LogManager::destroyInstance();
    return 0;
}

// ------- 懒汉式优化版单例模式 ------- //
#include <iostream>
#include <string>
#include <memory> // 引入智能指针

class ConfigManager {
private:
    ConfigManager() {
        std::cout << "ConfigManager 实例创建成功（C++11懒汉式）" << std::endl;
        loadConfig();
    }

    // 禁用拷贝构造和赋值运算符
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    std::string appName_;
    int appPort_;

    void loadConfig() {
        appName_ = "MyApp_C++11";
        appPort_ = 9090;
    }

public:
    // 核心：C++11 Magic Static(局部静态变量，线程安全)
    static ConfigManager& getInstance() {
        static ConfigManager instance; // 局部静态变量，首次调用时创建，且只初始化一次，线程安全
        return instance;
    }

    std::string getAppName() const {
        return appName_;
    }
    int getAppPort() const {
        return appPort_;
    }
};

// 测试C++11懒汉式单例模式
int main() {
    ConfigManager& config = ConfigManager::getInstance();
    std::cout << "AppName: " << config1.getAppName() << ", Port: " << config1.getAppPort() << std::endl;

    ConfigManager& config2 = ConfigManager::getInstance();
    std::cout << "是否为同一实例：" << (&config1 == &config2 ? "是" : "否") << std::endl;

    return 0;
}