#include <iostream>
#include <string>

// ====================== 1. 实现类接口（Implementor）：颜色维度 ======================
class Color {
public:
    virtual ~Color() {}
    // 颜色的核心方法：返回颜色名称（供形状使用）
    virtual std::string getColorName() const = 0;
};

// 具体实现类1：红色
class RedColor : public Color {
public:
    std::string getColorName() const override {
        return "红色";
    }
};

// 具体实现类2：蓝色
class BlueColor : public Color {
public:
    std::string getColorName() const override {
        return "蓝色";
    }
};

// 扩展实现类3：绿色（新增颜色，无需修改任何形状代码）
class GreenColor : public Color {
public:
    std::string getColorName() const override {
        return "绿色";
    }
};

// ====================== 2. 抽象类（Abstraction）：形状维度 ======================
class Shape {
protected:
    Color* color_; // 核心：持有实现类（颜色）的引用（组合）

public:
    // 构造函数：注入颜色对象
    Shape(Color* color) : color_(color) {}
    virtual ~Shape() {}

    // 形状的核心方法：绘制（抽象方法）
    virtual void draw() const = 0;
};

// ====================== 3. 扩展抽象类（Refined Abstraction） ======================
// 扩展抽象类1：圆形
class Circle : public Shape {
public:
    Circle(Color* color) : Shape(color) {}

    void draw() const override {
        std::cout << "绘制了一个 " << color_->getColorName() << " 的圆形" << std::endl;
    }
};

// 扩展抽象类2：方形
class Square : public Shape {
public:
    Square(Color* color) : Shape(color) {}

    void draw() const override {
        std::cout << "绘制了一个 " << color_->getColorName() << " 的方形" << std::endl;
    }
};

// 扩展抽象类3：五角星（新增形状，无需修改任何颜色代码）
class Star : public Shape {
public:
    Star(Color* color) : Shape(color) {}

    void draw() const override {
        std::cout << "绘制了一个 " << color_->getColorName() << " 的五角星" << std::endl;
    }
};

// ====================== 4. 客户端代码 ======================
int main() {
    // 步骤1：创建实现维度（颜色）的对象
    Color* red = new RedColor();
    Color* blue = new BlueColor();
    Color* green = new GreenColor();

    // 步骤2：创建抽象维度（形状）的对象，注入颜色（组合）
    Shape* redCircle = new Circle(red);
    Shape* blueSquare = new Square(blue);
    Shape* greenStar = new Star(green);
    Shape* redStar = new Star(red); // 灵活组合：红色+五角星

    // 步骤3：调用绘制方法（双维度解耦，任意搭配）
    redCircle->draw();   // 红色圆形
    blueSquare->draw();  // 蓝色方形
    greenStar->draw();   // 绿色五角星
    redStar->draw();     // 红色五角星

    // 清理内存（注意顺序：先清理形状，再清理颜色）
    delete redCircle;
    delete blueSquare;
    delete greenStar;
    delete redStar;
    delete red;
    delete blue;
    delete green;

    return 0;
}