#ifndef PageTable_H
#define PageTable_H

#include <QLabel>
#include <QEvent>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QButtonGroup>

/**
 * @author : LMH
 * @date   : 2023.11.01
 * @brief  : 分页组件
 */
class PageTable : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 数据操作类型枚举: 增加、修改、删除
     */
    enum Operation {
        Append = 0,
        Modify = 1,
        Delete = 2
    };
    Q_ENUM(Operation)

    /**
     * @brief 创建一个由布局对象包装的组件, 参数同构造, 提供默认值
     * @param header 表头
     * @param data 表数据
     * @param pageSize 页面尺寸
     * @param middleBtnCount 中间按钮数量
     * @param parent 父级对象
     * @return 布局封装, 注意管理指针内存
     */
    static QVBoxLayout* createObjWithLayout(QStringList header=QStringList(), QList<QStringList> data=QList<QStringList>(), int pageSize=25, int middleBtnCount=10, QWidget *parent = nullptr);

    /**
     * @brief 更新数据
     * @param data 数据集合
     * @param operation 数据操作类型, 枚举定义, 包括追加、修改和删除
     * @param index 起始位置, 用于修改操作; 这个起始位置应计算分页偏移量
     * @throw QMessage::critical 如果修改操作时传入的 index 小于 0。
     *
     * 根据传入的数据集合和操作类型，更新分页表格的数据。
     * - 追加: 将新数据追加到总数据末尾。
     * - 修改: 从指定索引位置开始, 用传入的数据替换总数据中的相应位置数据。
     *         如果索引越界, 则追加数据。
     * - 删除: 删除总数据中与传入数据匹配的所有数据。
     *
     * 注意：修改操作是基于index索引位置进行的。
     *      该方法在更新数据后会重新初始化分页信息和显示分页控件。
     */
    void updateData(QList<QStringList> &data, Operation operation=Operation::Append, int index=-1);

    /**
     * @brief 获取当前页数据
     * @return 当前页数据
     */
    QList<QStringList> getCurrentPageData();

    // 构造
    explicit PageTable(QStringList header=QStringList(), QList<QStringList> data=QList<QStringList>(), int pageSize=25, int middleBtnCount=10, QWidget *parent = nullptr);
    ~PageTable();
    // Getters
    int PageSize() const;
    int CurrentPage() const;
    int PageCount() const;
    int Total() const;
    QList<QStringList> Data() const;

signals:
    /**
     * @brief 当前页数改变时发射此信号
     * @param page 当前页码
     */
    void currentPageChanged(int page);

protected:
    /**
     * @brief 事件过滤器，用于处理事件
     * @param watched 监控对象
     * @param e 事件对象
     * @return 执行成功标志
     */
    virtual bool eventFilter(QObject *watched, QEvent *e);

private:
    /**************** 分页基础信息 ******************/
    /**
     * @brief 页面尺寸, 单页显示多少条数据
     */
    int m_PageSize;
    /**
     * @brief 当前页数
     */
    int m_CurrentPage;
    /**
     * @brief 总页数
     */
    int m_PageCount;
    /**
     * @brief 显示的页码按钮数量, 首尾各要一个, 剩下的是中间显示的按钮数量
     */
    int m_PageBtnCount;
    /**
     * @brief 中间显示的按钮数量, 动态配置, 以调整页码总数和显示按钮的逻辑
     */
    int m_MiddleBtnCount;
    /**
     * @brief 总数据条数
     */
    int m_Total;
    /**
     * @brief 数据集
     */
    QList<QStringList> m_Data;

    /**
     * @brief 根布局
     */
    QVBoxLayout* m_RootLayout;
    /**************** 表格元素 ******************/
    /**
     * @brief 字体配置
     */
    QFont m_Font;
    /**
     * @brief 表格控件
     */
    QTableWidget* m_TableWidget;
    /**
     * @brief 表头配置
     */
    QStringList m_TableHeader;


    /**************** 导航栏元素 ******************/
    /**
     * @brief 导航栏布局
     */
    QHBoxLayout* m_NavigationLayout;
    /**
     * @brief 导航栏布局弹簧
     */
    QSpacerItem* m_NaviLayoutSpacer;
    /**
     * @brief 显示总页数的标签
     */
    QLabel* m_TotalText;
    /**
     * @brief 按钮组，用于管理分页按钮
     */
    QButtonGroup *m_BtnGroup;
    /**
     * @brief 上一页按钮
     */
    QPushButton* m_PrevBtn;
    /**
     * @brief 下一页按钮
     */
    QPushButton* m_NextBtn;
    /**
     * @brief 第一页按钮
     */
    QPushButton* m_StartBtn;
    /**
     * @brief 最后一页按钮
     */
    QPushButton* m_EndBtn;
    /**
     * @brief 左边省略号按钮
     */
    QPushButton* m_QuickprevBtn;
    /**
     * @brief 右边省略号按钮
     */
    QPushButton* m_QuicknextBtn;
    /**
     * @brief 两个省略号中间的按钮列表
     */
    QList<QPushButton*>* m_VisibleBtnList;
    /**
     * @brief "前往"标签
     */
    QLabel* m_GoToLabel;
    /**
     * @brief "页" 标签
     */
    QLabel* m_PageLabel;
    /**
     * @brief 页码输入框
     */
    QLineEdit* m_PageLineEdit;
    /**
     * @brief 是否显示左边更多按钮
     */
    bool m_ShowPrevMore;
    /**
     * @brief 是否显示右边更多按钮
     */
    bool m_ShowNextMore;


    /**************** 私有方法 ******************/
    /**
     * @brief 初始化方法, 用于设置分页信息和显示分页控件
     */
    void initialize();
    /**
     * @brief 更新分页按钮列表
     * @return 更新后的按钮列表
     */
    QList<int> updatePages();
    /**
     * @brief 创建默认样式的按钮
     * @return 按钮实例
     */
    QPushButton* genBtnWithStyle();
    /**
     * @brief 筛选除第一个和最后一个页码按钮之外的所有按钮
     * @return 按钮集合
     */
    QList<QAbstractButton*> getButtons();

    // Private Setters
    void setCurrentPage(int page);
    void setPageCount(int pageCount);
private slots:
    /**
     * @brief 加载表格, 单元格级刷新
     * @param pageIndex 页面索引
     */
    void loadTable(int pageIndex);

};

#endif // PageTable_H
