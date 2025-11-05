// reference: https://www.doubao.com/chat/27669708128781826
#include <cstddef> // 用于size_t、ptrdiff_t等类型定义
#include <cstdlib> // 用于malloc、free等内存管理函数
#include <cmath>   // 提供简单哈希辅助
#include <utility> // 提供std::move(可选，优化性能)

// 前置声明：哈希函数默认实现（脱离std::hash）
template <typename T>
struct DefaultHash;

// 前置声明：相等比较器默认实现（脱离std::equal_to）
template <typename T>
struct DefaultEqual;

// ------- 节点结构：双向链表节点 ------- //
