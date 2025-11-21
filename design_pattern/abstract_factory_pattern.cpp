#include <iostream>
#include <string>

// ---- 产品族 A: 按钮 ----
class Button {
public:
    virtual void paint() const = 0;
    virtual ~Button() {}
};

class WindowsButton : public Button {
public:
    void paint() const override {
        std::cout << "绘制一个Windows风格的按钮。" << std::endl;
    }
};

class MacOSButton : public Button {
public:
    void paint() const override {
        std::cout << "绘制一个MacOS风格的按钮。" << std::endl;
    }
};

// ---- 产品族 B: 文本框 ----
class TextBox {
public:
    virtual void paint() const = 0;
    virtual ~TextBox() {}
};

class WindowsTextBox : public TextBox {
public:
    void paint() const override {
        std::cout << "绘制一个Windows风格的文本框。" << std::endl;
    }
};

class MacOSTextBox : public TextBox {
public:
    void paint() const override {
        std::cout << "绘制一个MacOS风格的文本框。" << std::endl;
    }
};

// ---- 抽象工厂 ----
class GUIFactory {
public:
    virtual Button* createButton() const = 0;
    virtual TextBox* createTextBox() const = 0;
    virtual ~GUIFactory() {}
};

// ---- 具体工厂 ----
class WindowsFactory : public GUIFactory {
public:
    Button* createButton() const override {
        return new WindowsButton();
    }
    TextBox* createTextBox() const override {
        return new WindowsTextBox();
    }
};

class MacOSFactory : public GUIFactory {
public:
    Button* createButton() const override {
        return new MacOSButton();
    }
    TextBox* createTextBox() const override {
        return new MacOSTextBox();
    }
};

// 客户端代码
void createAndDisplayUI(const GUIFactory& factory) {
    Button* button = factory.createButton();
    TextBox* textbox = factory.createTextBox();

    button->paint();
    textbox->paint();

    delete button;
    delete textbox;
}

int main() {
    std::cout << "在Windows系统上运行:" << std::endl;
    WindowsFactory windowsFactory;
    createAndDisplayUI(windowsFactory);

    std::cout << "\n在MacOS系统上运行:" << std::endl;
    MacOSFactory macOSFactory;
    createAndDisplayUI(macOSFactory);

    return 0;
}