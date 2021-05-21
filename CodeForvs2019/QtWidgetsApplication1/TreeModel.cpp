#include "TreeModel.h"
#include "NTFSHelper.h"


QTreeModel::QTreeModel(QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
{
    m_pRootItem = new QTreeItem;
}

QTreeModel::~QTreeModel()
{
    if (m_pRootItem)
    {
        delete m_pRootItem;
        m_pRootItem = nullptr;
    }
}

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    QTreeItem *pItem = static_cast<QTreeItem*>(index.internalPointer());
    if (pItem)
    {
        if (role == Qt::DisplayRole)
        {
            CString strPath = pItem->data().value<FileAttrInfo>().strFilePath;
            strPath = PathFindFileName(strPath);
            return CNTFSHelper::CStringToQString(strPath);
        }
        else if (role == Qt::DecorationRole)
        {
            return pItem->getIcon();
        }
    }
    return QVariant();
}

Qt::ItemFlags QTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemFlags();
    }
    return QAbstractItemModel::flags(index);
}

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    QTreeItem *parentItem;

    if (!parent.isValid())
    {
        parentItem = m_pRootItem;
    }
    else
    {
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());
    }

    QTreeItem *childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex QTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    QTreeItem *childItem = static_cast<QTreeItem*>(index.internalPointer());
    QTreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_pRootItem)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int QTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    QTreeItem *parentItem;
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        parentItem = m_pRootItem;
    }
    else
    {
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int QTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 1;
}

QTreeItem* QTreeModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }

    QTreeItem *item = static_cast<QTreeItem*>(index.internalPointer());
    return item;
}

QTreeItem * QTreeModel::root() const
{
    return m_pRootItem;
}

QModelIndex QTreeModel::GetItemIndexByInfo(const quint64& ui64FileNum, const CString& strDriverName) const
{
    if (m_pRootItem)
    {
        QTreeItem* pFindItem = m_pRootItem->FindIndexFromItemChild(ui64FileNum, strDriverName);
        if (pFindItem)
        {
            return createIndex(pFindItem->row(), 0, static_cast<void*>(pFindItem));
        }
    }
    return QModelIndex();
}