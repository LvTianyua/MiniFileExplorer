#pragma once
#include "stdafx.h"
#include "TreeItem.h"

class QTreeModel : public QAbstractItemModel
{
    Q_OBJECT

friend class QtWidgetsApplication1;

public:
    explicit QTreeModel(QObject *pParent = 0);
    ~QTreeModel();

    //以下为自定义model需要实现的一些虚函数，将会被Qt在查询model数据时调用
    //data: 核心函数，获取某个索引index的元素的各种数据
    //      role决定获取哪种数据，常用有下面几种：
    //      DisplayRole（默认）：就是界面显示的文本数据
    //      TextAlignmentRole：就是元素的文本对齐属性
    //      TextColorRole、BackgroundRole：分别指文本颜色、单元格背景色
    //flags: 获取index的一些标志，一般不怎么改
    //index: Qt向你的model请求一个索引为parent的节点下面的row行column列子节点的元素，在本函数里你需要返回该元素的正确索引
    //parent：获取指定元素的父元素
    //rowCount: 获取指定元素的子节点个数（下一级行数）
    //columnCount: 获取指定元素的列数
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

public:
    QTreeItem *itemFromIndex(const QModelIndex &index) const;
    QTreeItem *root() const;
    QModelIndex GetItemIndexByInfo(const quint64& ui64FileNum, const CString& strDriverName) const;

private:
    QTreeItem *m_pRootItem;    //根节点
};

