#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QMainWindow>
#include <QPushButton>
#include <QRandomGenerator>
#include "PageTable.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // 定时器
    QTimer m_timer;
    // 组件
    PageTable* page;
    // 临时数据
    QList<QStringList> m_data;
    // 随机生成器
    QRandomGenerator m_randomGenerator;

    QPushButton* m_modifyButton;
    QPushButton* m_deleteButton;
    /**
     * @brief 初始化操作按钮
     */
    void initOpBtn();

    /**
     * @brief 修改数据的测试
     */
    void changeTest();
};
#endif // MAINWINDOW_H
