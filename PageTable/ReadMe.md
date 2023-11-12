# ReadMe

这是一个分页表格组件，可以通过如下方式构造并使用：

* 构造方法 

  ```CPP
  PageTable* page = new PageTable(); // 构造组件
  setCentralWidget(page);// 在QMainWindow下添加
  // 或
  otherLayout->addWidget(page);// 添加到其它布局
  ```

* 布局封装；添加到另一个QWidget需要布局封装，使用方便些
  ```cpp
  QVBoxLayout* pageLayout = PageTable::createObjWithLayout();// 获取组件的封装
  otherWidget->setLayout(pageLayout);
  // 获取其中的组件
  // PageTable* page = static_cast<PageTable*>(pageLayout->itemAt(0)->widget());
  ```



调用时只关心构造组件的方法和更新数据的方法，数据将会以线程安全的形式单元格级更新表格；以下是更新数据的方法签名：

```cpp
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
*        如果索引越界, 则追加数据。
* - 删除: 删除总数据中与传入数据匹配的所有数据。
*
* 注意：修改操作是基于index索引位置进行的。
*      该方法在更新数据后会重新初始化分页信息和显示分页控件。
*/
void updateData(QList<QStringList> &data, Operation operation=Operation::Append, int index=-1);
```