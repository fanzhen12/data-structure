#include <iostream>
#include <vector>
#include <memory>
#include <string>

// 1. 接收者（Receiver）类：执行具体操作的类
class TextEditor {
private:
    std::string content_; // 编辑器文本内容

public:
    // 核心业务逻辑1：插入文本
    void insert(int pos, const std::string& text) {
        content_.insert(pos, text);
        std::cout << "[编辑器] 插入文本 → 内容：" << content_ << std::endl;
    }

    // 核心业务逻辑2：删除文本
    void remove(int pos, int len) {
        if (pos + len > content_.size()) {
            len = content_.size() - pos; // 调整长度，防止越界
        }
        content_.erase(pos, len);
        std::cout << "[编辑器] 删除文本 → 内容：" << content_ << std::endl;
    }

    // 获取当前内容（供命令/客户端查看）
    const std::string& getContent() const { return content_; }

    // 清空文本（辅助）
    void clear() { content_.clear(); }
};

// 2. 抽象命令（Command）接口
class EditorCommand {
public:
    virtual ~EditorCommand() = default;
    virtual void execute() = 0; // 执行命令
    virtual void undo() = 0;    // 撤销命令
    virtual std::string getName() const = 0; // 获取命令名称(辅助调试)
};

// 3. 具体命令类
// 3.1 插入文本命令
class InsertCommand : public EditorCommand {
private:
    std::shared_ptr<TextEditor> editor_; // 持有接收者的引用
    int pos_;                     // 插入位置
    std::string text_;            // 插入的文本

public:
    // 构造函数：绑定接收者 + 记录命令上下文
    InsertCommand(std::shared_ptr<TextEditor> editor, int pos, const std::string& text)
        : editor_(editor), pos_(pos), text_(text) {}
        
    // 执行：调用接收者的insert方法
    void execute() override {
        editor_->insert(pos_, text_);
    }

    // 撤销：删除插入的文本（反向操作）
    void undo() override {
        editor_->remove(pos_, text_.size());
        std::cout << "[撤销] 插入命令 → 回滚插入的文本：" << text_ << std::endl;
    }

    std::string getName() const override {
        return "插入命令（文本：" + text_ + "，位置：" + std::to_string(pos_) + "）";
    }
};

// 3.2 删除文本命令
class DeleteCommand : public EditorCommand {
private:
    std::shared_ptr<TextEditor> editor_; // 接收者的引用
    int pos_;                     // 删除位置
    int len_;                     // 删除长度
    std::string deletedText_;     // 记录删除的文本（用于撤销）

public:
    // 构造函数：绑定接收者 + 记录上下文（删除前先保存文本）
    DeleteCommand(std::shared_ptr<TextEditor> editor, int pos, int len)
        : editor_(editor), pos_(pos), len_(len) {
        // 提前保存要删除的文本，用于撤销
        if (pos + len > editor_->getContent().size()) {
            len_ = editor_->getContent().size() - pos; // 调整长度，防止越界
        }    
        deletedText_ = editor_->getContent().substr(pos, len_);
    }

    // 执行：调用接收者的remove方法
    void execute() override {
        editor_->remove(pos_, len_);
    }

    // 撤销：重新插入被删除的文本
    void undo() override {
        editor_->insert(pos_, deletedText_);
        std::cout << "[撤销] 删除命令 → 恢复文本：" << deletedText_ << std::endl;
    }

    std::string getName() const override {
        return "删除命令（位置：" + std::to_string(pos_) + "，长度：" + std::to_string(len_) + "）";
    }
};

// 4. 调用者（Invoker）类：管理和执行命令
class CommandInvoker {
private:
    std::vector<std::shared_ptr<EditorCommand>> commandHistory_; // 命令历史记录
    int currentCommandIdx_ = -1; // 当前执行到的命令索引

public:
    // 执行命令（添加到历史，触发execute）
    void executeCommand(std::shared_ptr<EditorCommand> cmd) {
        if (!cmd) return;
        std::cout << "\n===== 执行【" << cmd->getName() << "】=====" << std::endl;
        cmd->execute();
        // 清理撤销后新增的命令（比如撤销后执行新命令，旧的后续命令失效）
        if (currentCommandIdx_ < (int)commandHistory_.size() - 1) {
            commandHistory_.erase(commandHistory_.begin() + currentCommandIdx_ + 1, commandHistory_.end());
        }
        commandHistory_.push_back(cmd);
        currentCommandIdx_++;
        std::cout << "==========================" << std::endl;
    }

    // 撤销上一个命令
    bool undo() {
        if (currentCommandIdx_ < 0) {
            std::cout << "\n⚠️  无可撤销的命令！" << std::endl;
            return false;
        }

        auto cmd = commandHistory_[currentCommandIdx_];
        std::cout << "\n===== 撤销【" << cmd->getName() << "】=====" << std::endl;
        cmd->undo();
        currentCommandIdx_--;
        std::cout << "==========================" << std::endl;
        return true;
    }

    // 获取历史命令长度（辅助）
    int getCommandCount() const {
        return static_cast<int>(commandHistory_.size());
    }
};

int main() {
    // 步骤1：创建接收者（文本编辑器）
    auto editor = std::make_shared<TextEditor>();
    editor->clear();

    // 步骤2：创建调用者（命令管理器）
    CommandInvoker invoker;

    // 场景1：执行插入命令
    auto insertCmd1 = std::make_shared<InsertCommand>(editor, 0, "Hello");
    invoker.executeCommand(insertCmd1);

    // 场景2：执行删除命令（删除位置1，长度2 → "He"被删，剩下"llo"）
    auto deleteCmd1 = std::make_shared<DeleteCommand>(editor, 1, 2);
    invoker.executeCommand(deleteCmd1);

    // 场景3：执行第二个插入命令（位置1插入"i" → "li lo" → "llo"→插入后"lilo"）
    auto insertCmd2 = std::make_shared<InsertCommand>(editor, 1, "i");
    invoker.executeCommand(insertCmd2);

    // 场景4：撤销上一个命令（撤销插入"i"，回到"llo"）
    invoker.undo();

    // 场景5：再次撤销（撤销删除命令，恢复"Hello"）
    invoker.undo();

    // 场景6：第三次撤销（撤销插入命令，回到空）
    invoker.undo();

    // 场景7：无命令可撤销
    invoker.undo();

    return 0;
}