#ifndef OBJECTUTIL_H
#define OBJECTUTIL_H

#include <QWidget>
#include <QLayout>

/**
 * @author : LMH
 * @date   : 2023.11.09
 * @brief  : 工具类
 */
class ObjectUtil {

public:

    /**
     * @brief 清空布局中的元素, 但不销毁它们
     * @param layout 将要清空的布局指针
     */
    static void clearLayout(QLayout* layout);

};

#endif // OBJECTUTIL_H
