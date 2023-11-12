#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDateTime>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle("分页表格组件");
    showMaximized();
    m_randomGenerator.seed(QDateTime::currentMSecsSinceEpoch());

    /******************************* 分页组件测试用例 *********************************/
    // 初始化并挂载组件
    // 构造初始化
    page = new PageTable();
    setCentralWidget(page);

    // 初始化为布局包装
//    QVBoxLayout* pageLayout = PageTable::createObjWithLayout();
//    centralWidget()->setLayout(pageLayout);
//    // 这里重新获取page是为了不影响下方定时器相关的代码
//    page = static_cast<PageTable*>(pageLayout->itemAt(0)->widget());

    /********************* 添加数据测试 ***********************/
    // 定时器动态追加数据, 频率不要太快, 整组件级刷新而不是仅更新单元格文本, 将会影响按钮点击
    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    int timerCount = 0;// 定时器执行次数, 追加数据次数
    // 随机生成静态数据
    int size = m_randomGenerator.bounded(65, 101);
    connect(&m_timer, &QTimer::timeout, this, [=]() mutable {
        for (int i = 0; i < size; i++) {
            QStringList d;
            for(int v = 1; v <= 10; v++){
                d << QString("测试列%1").arg(m_randomGenerator.generateDouble() * (2350.00 - 100.00) + 100.00);
            }
            m_data.append(d);
        }
        // NOTE: 追加数据
        page->updateData(m_data);
        m_data.clear();

        // 追加10次
        timerCount++;
        if (timerCount >= 10) {
            m_timer.stop();
            // 执行完追加后, 才允许进行修改操作
            changeTest();
        }
    });
    m_timer.start();

}

/**
* @brief 修改数据的测试
*/
void MainWindow::changeTest() {
    /********************* 修改&删除数据测试 ***********************/
    initOpBtn();
    // 更新数据操作
    connect(m_modifyButton, &QPushButton::clicked, this, [=]() mutable {
        // 获取当前页数据并修改随机行
        QList<QStringList> currentPageData = page->getCurrentPageData();
        int rowCount = currentPageData.size();
        if (rowCount > 0) {
            // 随机选择要修改的行
            int randomRowIndex = m_randomGenerator.bounded(rowCount);
            // 修改该行的每个单元格，并在每个单元格后面加上一个小于pageSize的随机数
            for (auto& cell : currentPageData[randomRowIndex]) {
                cell = "修改后的数据" + QString::number(m_randomGenerator.generateDouble() * (2350.00 - 100.00) + 100.00);
            }

            // NOTE: 更新当前页数据
            // 计算以分页偏移量需要更新数据的初始位置
            int index = page->PageSize() * (page->CurrentPage() - 1);
            page->updateData(currentPageData, PageTable::Modify, index);
        }
    });

    // 删除数据操作
    connect(m_deleteButton, &QPushButton::clicked, this, [=]() {
        // 获取当前页数据
        QList<QStringList> currentPageData = page->getCurrentPageData();
        // NOTE: 删除数据
        page->updateData(currentPageData, PageTable::Delete);
    });
}

/**
* @brief 初始化操作按钮
*/
void MainWindow::initOpBtn() {
    // 设置操作按钮
    m_modifyButton = new QPushButton("修改当前页数据");
    m_deleteButton = new QPushButton("删除当前页数据");
    QString buttonQSS = "QPushButton{border: none; background: #409EFF; color: #FFF; border-radius: 4px; min-height: 35px; padding: 0 5px; font: 10pt '阿里巴巴普惠体 2.0 55 Regular';}";
    m_modifyButton->setStyleSheet(buttonQSS);
    m_deleteButton->setStyleSheet(buttonQSS);
    m_modifyButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    // 创建水平布局并设置按钮水平居中
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();  // 左侧弹簧
    buttonLayout->addWidget(m_modifyButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();  // 右侧弹簧
    // 将按钮添加到主界面
    QVBoxLayout* centralLayout = dynamic_cast<QVBoxLayout*>(centralWidget()->layout());
    if (centralLayout) {
        centralLayout->addLayout(buttonLayout);
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
