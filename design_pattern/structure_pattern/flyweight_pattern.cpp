#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

// 1. 抽象享元
class SpriteFlyweight {
public:
    virtual ~SpriteFlyweight() = default;
    virtual void render(int x, int y) const = 0; // 渲染方法，接受外部状态（位置）
    virtual std::string getType() const = 0; // 获取精灵类型（辅助调试）
};

// 2. 具体享元类
class ConcreteSprite : public SpriteFlyweight {
private:
    // 内部状态（可共享，仅存储一次）
    std::string type_;   // 精灵类型(草地/石头/树木等)
    std::string texture_; // 纹理数据（简化为字符串表示）

public:
    // 构造函数：初始化内部状态
    ConcreteSprite(std::string type, std::string texture)
        : type_(std::move(type)), texture_(std::move(texture)) {
        std::cout << "[享元创建] " << type_ << " 精灵（纹理：" << texture_ << "）" << std::endl;
    }

    // 渲染：结合外部状态（x/y）和内部状态
    void render(int x, int y) const override {
        std::cout << "[渲染] " << type_ << " @(" << x << "," << y << ")，纹理：" << texture_ << std::endl;
    }

    std::string getType() const override {
        return type_;
    }
};

// 3. 享元工厂类
class SpriteFactory {
private:
    // 缓存池：key=精灵类型，value=享元对象（智能指针避免内存泄漏）
    std::unordered_map<std::string, std::shared_ptr<SpriteFlyweight>> spritePool_;

public:
    // 获取享元对象
    std::shared_ptr<SpriteFlyweight> getSprite(const std::string& type) {
        // 检查缓存
        if (spritePool_.find(type) != spritePool_.end()) {
            return spritePool_[type];
        }
        // 无缓存则创建（模拟不同类型的纹理路径）
        std::string texture;
        if (type == "草地") {
            texture = "grass_texture.png";
        } else if (type == "石头") {
            texture = "rock_texture.png";
        } else if (type == "树木") {
            texture = "tree_texture.png";
        } else {
            texture = "default_texture.png";
        }
        // 加入缓存并返回
        auto newSprite = std::make_shared<ConcreteSprite>(type, texture);
        spritePool_[type] = newSprite;
        return newSprite;
    }

    // 获取缓存池大小（查看已创建的享元数量）
    int getPoolSize() const {
        return spritePool_.size();
    }
};

// 客户端代码示例
int main() {
    // 步骤1：创建享元工厂
    SpriteFactory factory;

    // 步骤2：渲染1000个草地精灵（共享同一享元）
    for (int i = 0; i < 1000; ++i) {
        auto grassSprite = factory.getSprite("草地");
        grassSprite->render(i % 100, i / 100); // 模拟不同位置
    }

    // 步骤3：渲染500个石头精灵（共享同一享元）
    for (int i = 0; i < 500; ++i) {
        auto rockSprite = factory.getSprite("石头");
        rockSprite->render(i % 50, i / 50); // 模拟不同位置
    }

    // 步骤4：查看享元对象数量
    std::cout << "已创建的享元对象数：" << factory.getPoolSize() << std::endl;

    // 步骤5：复用已创建的草地精灵（验证缓存）
    auto grassSprite = factory.getSprite("草地");
    grassSprite->render(10000, 100); // 新坐标，复用同一个享元

    return 0;
}