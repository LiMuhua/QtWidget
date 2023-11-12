#include "PageTable.h"

#include <QDebug>
#include <QMutex>
#include <QMessageBox>
#include <QPaintEvent>
#include <QHeaderView>
#include <QMutexLocker>
#include <QIntValidator>
#include <QtCore/qmath.h>
#include "ObjectUtil.h"

/************************** 公共方法 ****************************/
/**
* @brief 创建一个由布局对象包装的组件, 参数同构造, 提供默认值
* @param header 表头
* @param data 表数据
* @param pageSize 页面尺寸
* @param middleBtnCount 中间按钮数量
* @param parent 父级对象
* @return 布局封装, 注意管理指针内存
*/
QVBoxLayout* PageTable::createObjWithLayout(QStringList header, QList<QStringList> data, int pageSize, int middleBtnCount, QWidget *parent) {
    QVBoxLayout* layout = new QVBoxLayout(parent);
    PageTable* pageTable = new PageTable(header, data, pageSize, middleBtnCount, parent);
    pageTable->setObjectName("PageTable");
    layout->addWidget(pageTable);
    return layout;
}
/**
* @brief 更新数据
* @param data 数据集合
* @param operation 数据操作类型, 枚举定义, 包括追加、修改和删除
* @param index 起始位置, 用于修改操作; 这个起始位置应由外部计算分页偏移量
* @throw QMessage::critical 如果修改操作时传入的 index 小于 0。
*
* 根据传入的数据集合和操作类型，更新分页表格的数据。
* - 追加: 将新数据追加到总数据末尾。
* - 修改: 从指定索引位置开始，用传入的数据替换总数据中的相应位置数据。
*         如果索引越界，则追加数据。
* - 删除: 删除总数据中与传入数据匹配的所有数据。
*
* 注意：修改操作是基于index索引位置进行的。
*      该方法在更新数据后会重新初始化分页信息和显示分页控件。
*/
void PageTable::updateData(QList<QStringList> &data, Operation operation, int index) {
    QMutex mutex;
    QMutexLocker locker(&mutex);

    // 校验 index 参数
    if (operation == Modify && index < 0) {
        QMessageBox::critical(this, "错误", "修改操作必须传入显式有效的index。");
        return;
    }

    switch (operation) {
    case Append: m_Data.append(data); break; // 追加数据
    case Modify:
        // 修改数据
        for (int i = 0; i < data.size(); ++i) {
            int dataIndex = index + i;
            if (dataIndex < m_Data.size()) {
                m_Data[dataIndex] = data[i];
            } else {
                // 如果索引越界，则追加数据
                m_Data.append(data[i]);
            }
        }
        break;
    case Delete:
        // 删除数据
        for (int i = 0; i < data.size(); ++i) {
            m_Data.removeAll(data[i]);
        }
        break;
    }

    m_Total = m_Data.size();
    initialize();
}
/**
* @brief 获取当前页数据
* @return 当前页数据
*/
QList<QStringList> PageTable::getCurrentPageData() {
    int startIndex = (m_CurrentPage - 1) * m_PageSize;
    int endIndex = qMin(startIndex + m_PageSize, m_Data.size());
    return m_Data.mid(startIndex, endIndex - startIndex);
}

/************************** 限制方法 ****************************/
// 私有方法
/**
* @brief 初始化方法, 用于设置分页信息和显示分页控件
*/
void PageTable::initialize() {
    QMutex mutex;
    QMutexLocker locker(&mutex);

    /************************** 导航栏控件 ****************************/
    // 清除之前的按钮, 但跳过在构造中初始化的首末按钮
    QList<QAbstractButton*> buttons = m_BtnGroup->buttons();
    for (int i = 2; i < buttons.count(); i++) {
        QAbstractButton* button = buttons.at(i);
        m_BtnGroup->removeButton(button);
        button->deleteLater();
    }
    // 清空导航布局中的子控件, 但不删除指针 (不销毁控件);
    ObjectUtil::clearLayout(m_NavigationLayout);

    // 计算总页数, 余数自动向上取整
    m_PageCount = (m_Total + m_PageSize - 1) / m_PageSize;
    // 设置页码按钮显示数量
    m_PageBtnCount = (m_PageCount <= m_MiddleBtnCount) ? (m_PageCount + 2) : (m_MiddleBtnCount + 2);

    // 设置显示文本
    m_TotalText->setText(QString::fromUtf8("共%1条").arg(m_Data.size()));
    m_StartBtn->setText(QString("%1").arg(1));
    m_EndBtn->setText(QString("%1").arg(m_PageCount));

    m_QuickprevBtn->hide(); // 隐藏左边省略号按钮

    // 为导航栏布局添加各个控件
    //布局弹簧
    m_NavigationLayout->addItem(m_NaviLayoutSpacer);
    m_NavigationLayout->addItem(m_NaviLayoutSpacer);
    // 主要控件
    m_NavigationLayout->addWidget(m_TotalText);
    m_NavigationLayout->addWidget(m_PrevBtn);
    m_NavigationLayout->addWidget(m_StartBtn);
    m_NavigationLayout->addWidget(m_QuickprevBtn);

    // 为中间的页面按钮生成并添加到布局
    for (int i = 2; i < m_PageBtnCount; i++) {
        QPushButton* tempBtn = genBtnWithStyle();
        m_BtnGroup->addButton(tempBtn);
        // 处理显示按钮和总页数之间的关系
        tempBtn->setText(QString("%1").arg(m_PageCount <= m_MiddleBtnCount?i-1:i));
        m_NavigationLayout->addWidget(tempBtn);
    }

    // 为导航栏布局添加其余控件
    m_NavigationLayout->addWidget(m_QuicknextBtn);
    m_NavigationLayout->addWidget(m_EndBtn);
    m_NavigationLayout->addWidget(m_NextBtn);
    m_NavigationLayout->addWidget(m_GoToLabel);
    m_NavigationLayout->addWidget(m_PageLineEdit);
    m_PageLineEdit->setFocus();// 每次刷新后聚焦输入框
    m_NavigationLayout->addWidget(m_PageLabel);
    // 布局弹簧
    m_NavigationLayout->addItem(m_NaviLayoutSpacer);

    m_NavigationLayout->addStretch(); // 添加伸缩空间
    setCurrentPage(m_CurrentPage); // 设置当前页
}
/**
* @brief 更新分页按钮列表
* @return 更新后的按钮列表
*/
QList<int> PageTable::updatePages() {
    // 初始化计算参数 & 标志位
    int pageBtnCount = m_PageBtnCount;
    int halfPageBtnCount = (pageBtnCount - 1) / 2;
    int currentPage = m_CurrentPage;
    int pageCount = m_PageCount;
    bool showPrevMore = false;
    bool showNextMore = false;

    // 判断是否需要显示左边"更多"按钮
    if (pageCount > pageBtnCount) {
        if (currentPage > pageBtnCount - halfPageBtnCount) {
            showPrevMore = true;
        }
    }
    // 判断是否需要显示右边"更多"按钮, 页面总数大于配置的中间按钮数量、末尾页码和中间按钮最后一个(即除去首末按钮的按钮组的数量)不连号、当前页小于页码减去一半按钮数(即位于中间时)
    bool notLastToTheEnd = m_BtnGroup->buttons().size()-2!=pageCount-1;
    if ((pageCount > m_MiddleBtnCount) && notLastToTheEnd && (currentPage < pageCount - halfPageBtnCount)) {
        showNextMore = true;
    }

    QList<int> array; // 创建一个整数列表, 用于存储分页按钮的页码

    // 根据显示左边"更多"按钮和右边"更多"按钮的标志,
    // 确定要显示的分页按钮的页码; 同时处理显示按钮和总页数之间的关系
    if (showPrevMore && !showNextMore) {
        // 显示左边"更多"按钮, 但不显示右边"更多"按钮
        // 末尾的最大页按钮独立处理, 此时它被隐藏, 需要在这里补上, 故数量和数值都要+1
        int startPage = pageCount - (pageBtnCount - 2) + 1;
        for (int i = startPage; i <= pageCount; i++) {
            array.append(i);
        }
    } else if (!showPrevMore && showNextMore) {
        // 不显示左边"更多"按钮, 但显示右边"更多"按钮
        for (int i = 2; i < pageBtnCount; i++) {
            array.append(i-1);// 前方已将开始按钮隐藏，此处从1开始
        }
    } else if (showPrevMore && showNextMore) {
        // 同时显示左边"更多"按钮和右边"更多"按钮
        int offset = qFloor(pageBtnCount / 2) - 1;
        for (int i = currentPage - offset; i <= currentPage + offset; i++) {
            array.append(i);
        }
    } else {
        // 不显示任何"更多"按钮
        for (int i = 2; i < pageBtnCount; i++) {
            array.append(m_PageCount <= m_MiddleBtnCount?i-1:i);
        }
    }

    m_ShowPrevMore = showPrevMore; // 更新显示左边"更多"按钮的标志
    m_ShowNextMore = showNextMore; // 更新显示右边"更多"按钮的标志

    return array; // 返回分页按钮的页码列表
}
/**
* @brief 创建默认样式的按钮
* @return 按钮实例
*/
QPushButton* PageTable::genBtnWithStyle(){
    QPushButton *btn = new QPushButton(this);
    btn->setFont(m_Font);
    btn->setFixedSize(30,30);
    btn->setCursor(Qt::PointingHandCursor);
    btn->installEventFilter(this);
    return btn;
}
/**
* @brief 筛选除第一个和最后一个页码按钮之外的所有按钮
* @return 按钮集合
*/
QList<QAbstractButton *> PageTable::getButtons(){
    QList<QAbstractButton *> *buttons = new QList<QAbstractButton *>();
    for(int i=0; i<m_BtnGroup->buttons().count(); i++){
        //将变化的按钮筛选出来
        if(m_BtnGroup->buttons().at(i)!=m_StartBtn && m_BtnGroup->buttons().at(i)!=m_EndBtn){
            buttons->append(m_BtnGroup->buttons().at(i));
        }
    }
    return *buttons;
}
// 槽函数
/**
* @brief 加载表格, 单元格级刷新
* @param pageIndex 页面索引
*/
void PageTable::loadTable(int pageIndex) {
    QMutex mutex;
    QMutexLocker locker(&mutex);

    int pageSize = m_PageSize;
    int startIndex = (pageIndex - 1) * pageSize;
    int endIndex = startIndex + pageSize;

    // 无效的 pageIndex 或者不是当前页面, 不刷新
    if (startIndex < 0 || endIndex > m_Data.size() || pageIndex != m_CurrentPage) {
        return;
    }

    // 只更新变化的单元格
    for (int i = 0; i < pageSize; i++) {
        int row = i;
        int dataIdx = startIndex + i;
        QStringList items = m_Data[dataIdx];

        if (dataIdx >= m_Data.size()) {
            continue;  // 超出数据范围，跳过处理
        }

        for (int j = 0; j < items.size(); j++) {
            QTableWidgetItem* cell = m_TableWidget->item(row, j);
            if (!cell) {
                // 单元格不存在，创建并设置属性
                cell = new QTableWidgetItem(items[j]);
                cell->setFont(m_Font);
                m_TableWidget->setItem(row, j, cell);
                cell->setTextAlignment(Qt::AlignCenter);
            } else if (cell->text() != items[j]) {
                // 仅更新发生变化的单元格
                cell->setText(items[j].isEmpty() || items[j] == "nan" ? "--" : items[j]);
            }
        }
    }
}

// 保护方法
/**
* @brief 事件过滤器, 用于处理事件
* @param watched 监控对象
* @param e 事件对象
* @return 执行成功标志
*/
bool PageTable::eventFilter(QObject *watched, QEvent *e){

    if (e->type() == QEvent::MouseButtonRelease){
        int newPage =-1;
        int currentPage = m_CurrentPage;
        int pagerCountOffset = m_PageBtnCount - 2;

        //上一页按钮被点击
        if(watched==m_PrevBtn&&m_PrevBtn->isCheckable()){
            newPage = m_CurrentPage - 1;
            setCurrentPage(newPage);
        }
        //下一页按钮被点击
        if(watched==m_NextBtn&&m_NextBtn->isCheckable()){
            newPage = m_CurrentPage + 1;
            setCurrentPage(newPage);
        }
        //左边省略号按钮被点击
        if(watched==m_QuickprevBtn){
            newPage = currentPage - pagerCountOffset;
            setCurrentPage(newPage);
        }
        //右边省略号按钮被点击
        if(watched==m_QuicknextBtn){
            newPage = currentPage + pagerCountOffset;
            setCurrentPage(newPage);
        }

        for (int i = 0; i < m_BtnGroup->buttons().count(); ++i) {
            if (watched == m_BtnGroup->buttons().at(i)) {
                newPage = m_BtnGroup->buttons().at(i)->text().toInt();
                setCurrentPage(newPage);
                break;
            }
        }
    }

    //跳转页敲击回车事件
    if (watched == m_PageLineEdit && e->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
            setCurrentPage(m_PageLineEdit->text().toInt());
            return true;
        }
    }

    return QWidget::eventFilter(watched, e);
}

/************************** PO方法 ****************************/
// 构造
PageTable::PageTable(QStringList header, QList<QStringList> data, int pageSize, int middleBtnCount, QWidget *parent)
    : QWidget(parent), m_PageSize(pageSize), m_MiddleBtnCount(middleBtnCount), m_Data(data) {
    // 初始化基础信息
    m_CurrentPage = 1;
    m_PageBtnCount = m_MiddleBtnCount+2;
    m_Total = m_Data.isEmpty() ? m_PageSize : m_Data.size();
    // 全局字体
    m_Font = QFont("阿里巴巴普惠体 2.0 55 Regular", 10);

    m_RootLayout = new QVBoxLayout(this);
    /************************** 初始化表格控件 ****************************/
    if(header.isEmpty()){
        for (int i = 1; i <= 10; i++) {
            header<<QString("默认列%1").arg(i);
        }
    }
    m_TableWidget = new QTableWidget(m_PageSize, header.size());// 根据数据行和表头列初始化一个表格
    m_TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);// 设置表格为单行选择
    m_TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);// 设置只能选中行，不能单个选择单元格
    m_TableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);// 设置单元格不可编辑
    m_TableWidget->verticalHeader()->setHidden(true);// 隐藏行号 (不显示表格左边的行号)
    m_TableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 让表格挤满占个父容器
    m_TableWidget->setHorizontalHeaderLabels(header);// 设置表头文本及样式
    m_TableWidget->setStyleSheet("QHeaderView::section { color: black; font: bold 18px '阿里巴巴普惠体 2.0 55 Regular'; text-align: center; height: 25px; background-color: #d1dff0; border: 1px solid #8faac9; border-left: none; }");

    // 挂载部件
    m_RootLayout->addWidget(m_TableWidget);


    /************************** 初始化导航栏控件 ****************************/
    // 调整样式
    QString naviQSS = "QPushButton:hover{background: #e7e7e7;}"
                  "QPushButton[currentPage=\"true\"]{background: #409EFF;color:#FFF}"
                  "QPushButton{border: none; background: #f5f5f5;border-radius: 4px;}";
    this->setStyleSheet(naviQSS);
    // 初始化布局
    m_BtnGroup = new QButtonGroup(this);
    m_NaviLayoutSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_NavigationLayout = new QHBoxLayout();
    m_NavigationLayout->setSpacing(8);
    m_NavigationLayout->setMargin(0);

    // 创建一个QLabel并设置其文本为空
    QLabel* emptyLabel = new QLabel("");
    emptyLabel->setMinimumWidth(100); // 设置最小宽度
    emptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum); // 设置水平策略为Expanding
    emptyLabel->setStyleSheet("border: 1px solid black;"); // 设置边框样式
    m_NavigationLayout->addWidget(emptyLabel);

    // 初始化总条数标签
    m_TotalText = new QLabel(this);
    m_TotalText->setFont(m_Font);

    // 设置上一页按钮
    m_PrevBtn = genBtnWithStyle();
    m_PrevBtn->setIcon(QIcon(":/images/arrow-left"));
    m_StartBtn = genBtnWithStyle();
    m_BtnGroup->addButton(m_StartBtn);

    // 添加左边省略号按钮
    m_QuickprevBtn = genBtnWithStyle();
    m_QuickprevBtn->setText(QString("..."));

    // 设置页数列表
    m_VisibleBtnList = new QList<QPushButton *>();

    // 添加右边省略号按钮
    m_QuicknextBtn = genBtnWithStyle();
    m_QuicknextBtn->setText(QString("..."));
    m_EndBtn = genBtnWithStyle();
    m_BtnGroup->addButton(m_EndBtn);

    //设置上一页按钮
    m_NextBtn = genBtnWithStyle();
    m_NextBtn->setIcon(QIcon(":/images/arrow-right"));

    //设置 "前往多少页" 布局
    m_GoToLabel = new QLabel(this);
    m_GoToLabel->setFont(m_Font);
    m_GoToLabel->setText(QString::fromUtf8("前往"));
    m_PageLineEdit = new QLineEdit(this);
    m_PageLineEdit->setFont(m_Font);
    m_PageLineEdit->installEventFilter(this);
    m_PageLineEdit->setFixedSize(50,30);
    m_PageLineEdit->setAlignment(Qt::AlignHCenter);
    m_PageLineEdit->setValidator(new QIntValidator(1, 10000, this));
    m_PageLineEdit->setStyleSheet("QLineEdit{border-radius: 4px;border: 1px solid #dcdfe6;}");
    m_PageLabel=new QLabel(this);
    m_PageLabel->setFont(m_Font);
    m_PageLabel->setText(QString::fromUtf8("页"));

    // 挂载根布局
    m_RootLayout->addLayout(m_NavigationLayout);

    // 挂载信号
    connect(this, &PageTable::currentPageChanged, this, &PageTable::loadTable);

    // 构造完后执行初始化, 加载第一页
    initialize();
}

PageTable::~PageTable() {
    delete m_VisibleBtnList;
    delete m_TableWidget;

    // 销毁导航布局下的所有指针 以及自身
    QLayoutItem* child;
    while ((child = m_NavigationLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    delete m_NavigationLayout;
    delete m_RootLayout;
}

// Getters && Setters
void PageTable::setCurrentPage(int page){

    // 更新当前页 & 输入框数据
    // 如果页数小于1, 将其设置为1; 如果大于总页数, 将其设置为总页数；否则保持不变
    m_CurrentPage = (page < 1) ? 1 : (page > m_PageCount) ? m_PageCount : page;
    m_PageLineEdit->setText(QString("%1").arg(page));

    // 更新分页按钮的文本
    for(int k = 0; k < getButtons().count(); k++) {
        getButtons().at(k)->setText(QString("%1").arg(updatePages().at(k)));
    }

    // 设置左右省略号及首末按钮的可见性
    m_QuickprevBtn->setVisible(m_ShowPrevMore);
    m_QuicknextBtn->setVisible(m_ShowNextMore);
    m_EndBtn->setVisible(m_ShowNextMore);
    // 当末尾页码和中间按钮最后一个(即除去首末按钮的按钮组的数量)连号时, 保持首页按钮显示
    bool isLastToTheEnd = m_BtnGroup->buttons().size()-2==m_PageCount-1;
    m_StartBtn->setVisible(m_ShowPrevMore||isLastToTheEnd);

    // 标记当前页码按钮为选中状态, 其他按钮不选中
    for(int i = 0; i < m_BtnGroup->buttons().count(); i++) {
        if(m_BtnGroup->buttons().at(i)->text().toInt() == page) {
            m_BtnGroup->buttons().at(i)->setProperty("currentPage", "true");
        } else {
            m_BtnGroup->buttons().at(i)->setProperty("currentPage", "false");
        }
        m_BtnGroup->buttons().at(i)->setStyleSheet("/**/");
    }

    // 初始化"上一页" "下一页"按钮状态
    m_PrevBtn->setCursor(Qt::PointingHandCursor);
    m_NextBtn->setCursor(Qt::PointingHandCursor);
    m_PrevBtn->setCheckable(true);
    m_NextBtn->setCheckable(true);
    m_PrevBtn->setToolTip("上一页");
    m_NextBtn->setToolTip("下一页");

    // 如果当前页是第一页, 禁用上一页按钮
    if(page == 1) {
        m_PrevBtn->setCheckable(false);
        m_PrevBtn->setCursor(Qt::ForbiddenCursor);
        m_PrevBtn->setToolTip("已是第一页.");
    }
    // 如果当前页是最后一页, 禁用下一页按钮
    if(page == m_PageCount) {
        m_NextBtn->setCheckable(false);
        m_NextBtn->setCursor(Qt::ForbiddenCursor);
        m_NextBtn->setToolTip("已是最后一页.");
    }

    // 发送当前页数已更改的信号
    emit currentPageChanged(page);

}
void PageTable::setPageCount(int pageCount){
    m_PageCount = pageCount;
}


QList<QStringList> PageTable::Data() const {
    return m_Data;
}
int PageTable::Total() const {
    return m_Total;
}
int PageTable::PageCount() const {
    return m_PageCount;
}
int PageTable::CurrentPage() const {
    return m_CurrentPage;
}
int PageTable::PageSize() const {
    return m_PageSize;
}
