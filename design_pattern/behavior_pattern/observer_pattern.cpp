#include <iostream>
#include <vector>
#include <algorithm> // 用于删除观察者

// ====================== 1. 抽象观察者（Observer）：显示面板接口 ======================
class DisplayObserver {
public:
    virtual ~DisplayObserver() {}
    // 核心更新方法：接收气象站的温度、湿度数据
    virtual void update(float temperature, float humidity) = 0;
    // 返回观察者名称（便于日志）
    virtual std::string getName() const = 0;
};

// ====================== 2. 抽象主题（Subject）：气象站接口 ======================
class WeatherSubject {
public:
    virtual ~WeatherSubject() {}
    // 注册观察者（订阅）
    virtual void registerObserver(DisplayObserver* observer) = 0;
    // 移除观察者（取消订阅）
    virtual void removeObserver(DisplayObserver* observer) = 0;
    // 通知所有观察者（状态变化时调用）
    virtual void notifyObservers() = 0;
};

// ====================== 3. 具体主题（Concrete Subject）：气象站实现 ======================
class WeatherStation : public WeatherSubject {
private:
    std::vector<DisplayObserver*> observers_; // 观察者列表
    float temperature_; // 温度（主题的核心状态）
    float humidity_;    // 湿度（主题的核心状态）

    // 私有方法：检查状态是否真的变化（避免无意义通知）
    bool isStateChanged(float newTemp, float newHumidity) const {
        return (newTemp != temperature_) || (newHumidity != humidity_);
    }

public:
    WeatherStation() : temperature_(0.0f), humidity_(0.0f) {}

    // 注册观察者：添加到列表
    void registerObserver(DisplayObserver* observer) override {
        observers_.push_back(observer);
        std::cout << "[气象站] 注册观察者：" << observer->getName() << std::endl;
    }

    // 移除观察者：从列表删除
    void removeObserver(DisplayObserver* observer) override {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            std::cout << "[气象站] 移除观察者：" << observer->getName() << std::endl;
            observers_.erase(it);
        }
    }

    // 通知所有观察者：遍历列表，调用update()
    void notifyObservers() override {
        std::cout << "\n[气象站] 状态更新，通知所有观察者..." << std::endl;
        for (auto observer : observers_) {
            observer->update(temperature_, humidity_); // 推送模式：主动传状态
        }
    }

    // 气象站更新天气数据（核心：状态变化后触发通知）
    void setWeatherData(float newTemp, float newHumidity) {
        std::cout << "\n===== 气象站监测到新数据 =====" << std::endl;
        std::cout << "原温度：" << temperature_ << "℃，新温度：" << newTemp << "℃" << std::endl;
        std::cout << "原湿度：" << humidity_ << "%，新湿度：" << newHumidity << "%" << std::endl;

        // 只有状态变化时才通知
        if (isStateChanged(newTemp, newHumidity)) {
            temperature_ = newTemp;
            humidity_ = newHumidity;
            notifyObservers(); // 触发通知
        } else {
            std::cout << "[气象站] 状态无变化，无需通知" << std::endl;
        }
    }

    // 拉取模式：提供获取状态的接口（观察者可主动获取）
    float getTemperature() const { return temperature_; }
    float getHumidity() const { return humidity_; }
};

// ====================== 4. 具体观察者（Concrete Observer） ======================
// 具体观察者1：温度显示面板
class TemperatureDisplay : public DisplayObserver {
public:
    void update(float temperature, float humidity) override {
        // 只关注温度，忽略湿度
        std::cout << "[" << getName() << "] 收到更新：当前温度 = " << temperature << "℃" << std::endl;
        // 业务逻辑：温度过高/过低提示
        if (temperature > 35) {
            std::cout << "[" << getName() << "] 预警：高温！当前温度超过35℃" << std::endl;
        } else if (temperature < 0) {
            std::cout << "[" << getName() << "] 预警：低温！当前温度低于0℃" << std::endl;
        }
    }

    std::string getName() const override {
        return "温度显示面板";
    }
};

// 具体观察者2：湿度显示面板
class HumidityDisplay : public DisplayObserver {
public:
    void update(float temperature, float humidity) override {
        // 只关注湿度，忽略温度
        std::cout << "[" << getName() << "] 收到更新：当前湿度 = " << humidity << "%" << std::endl;
        // 业务逻辑：湿度过高提示
        if (humidity > 80) {
            std::cout << "[" << getName() << "] 预警：高湿度！当前湿度超过80%" << std::endl;
        }
    }

    std::string getName() const override {
        return "湿度显示面板";
    }
};

// 具体观察者3：综合预警面板
class WarningDisplay : public DisplayObserver {
public:
    void update(float temperature, float humidity) override {
        // 关注温度+湿度，综合预警
        std::cout << "[" << getName() << "] 收到更新：温度=" << temperature << "℃，湿度=" << humidity << "%" << std::endl;
        if (temperature > 35 && humidity > 70) {
            std::cout << "[" << getName() << "] 紧急预警：高温高湿，易中暑！" << std::endl;
        }
    }

    std::string getName() const override {
        return "综合预警面板";
    }
};

// ====================== 5. 客户端代码 ======================
int main() {
    // 步骤1：创建主题（气象站）
    WeatherStation* weatherStation = new WeatherStation();

    // 步骤2：创建观察者（各个显示面板）
    DisplayObserver* tempDisplay = new TemperatureDisplay();
    DisplayObserver* humiDisplay = new HumidityDisplay();
    DisplayObserver* warnDisplay = new WarningDisplay();

    // 步骤3：注册观察者（订阅）
    weatherStation->registerObserver(tempDisplay);
    weatherStation->registerObserver(humiDisplay);
    weatherStation->registerObserver(warnDisplay);

    // 场景1：更新天气数据（25℃，60%）→ 状态变化，通知所有观察者
    weatherStation->setWeatherData(25.0f, 60.0f);

    // 场景2：更新天气数据（36℃，75%）→ 高温，触发温度面板预警
    weatherStation->setWeatherData(36.0f, 75.0f);

    // 场景3：移除湿度面板，再更新数据（36℃，85%）→ 湿度面板不再收到通知
    weatherStation->removeObserver(humiDisplay);
    weatherStation->setWeatherData(36.0f, 85.0f);

    // 场景4：更新相同数据 → 无状态变化，不通知
    weatherStation->setWeatherData(36.0f, 85.0f);

    // 清理内存
    delete weatherStation;
    delete tempDisplay;
    delete humiDisplay;
    delete warnDisplay;

    return 0;
}