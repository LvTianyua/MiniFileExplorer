#pragma once
#include "QVariant"
#include "INTFSHelper.h"

class QTreeItem
{
public:
    explicit QTreeItem(QTreeItem *pParentItem = nullptr);
    ~QTreeItem();

    void appendChild(QTreeItem *pChild);      //在本节点下增加子节点
    void removeChilds();                    //清空所有节点

    QTreeItem *child(int nRow) const;               //获取第row个子节点指针
    QTreeItem *parentItem();                 //获取父节点指针
    int childCount() const;                 //子节点计数
    int row() const;                        //获取该节点是父节点的第几个子节点

    QVariant data() const;                  //核心函数：获取节点数据
    QVariant getIcon() const;               //获取图标

    //设置、获取节点存的数据指针
    void setPtr(const pFileAttrInfo pAttrInfo) { m_pFileAttrInfo = pAttrInfo; }
    pFileAttrInfo ptr() { return m_pFileAttrInfo; }

    //保存该节点是其父节点的第几个子节点，查询优化所用
    void setRow(int nRow) {
        m_nRow = nRow;
    }

	QTreeItem* FindIndexFromItemChild(const quint64& ui64FileNum, const CString& strDriverName) const;

private:
    QVector<QTreeItem*> m_vecChildItems;                        //子节点
    QTreeItem *m_parentItem = nullptr;                                    //父节点
    pFileAttrInfo m_pFileAttrInfo = nullptr;     //存储数据的指针
    int m_nRow = 0;                                             //记录该item是第几个，可优化查询效率
};

