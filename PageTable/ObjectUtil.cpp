#include "ObjectUtil.h"

/**
* @brief 清空布局中的元素, 但不销毁它们
* @param layout 将要清空的布局指针
*/
void ObjectUtil::clearLayout(QLayout *layout) {
    QLayoutItem* child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }
    }
}
