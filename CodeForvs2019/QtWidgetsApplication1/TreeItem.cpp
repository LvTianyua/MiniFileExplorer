#include "TreeItem.h"

QTreeItem::QTreeItem(QTreeItem *pParentItem /*= nullptr*/)
    : m_parentItem(pParentItem)
{
}

QTreeItem::~QTreeItem()
{
    removeChilds();
    if (m_pFileAttrInfo)
    {
        delete m_pFileAttrInfo;
        m_pFileAttrInfo = nullptr;
    }
}

void QTreeItem::appendChild(QTreeItem *pChild)
{
    pChild->setRow(m_vecChildItems.size());   //item存自己是第几个，可以优化效率
    m_vecChildItems.push_back(pChild);
}

void QTreeItem::removeChilds()
{
    qDeleteAll(m_vecChildItems);
    m_vecChildItems.clear();
}

QTreeItem * QTreeItem::child(int nRow) const
{
    return m_vecChildItems.value(nRow);
}

QTreeItem * QTreeItem::parentItem()
{
    return m_parentItem;
}

int QTreeItem::childCount() const
{
    return m_vecChildItems.count();
}

int QTreeItem::row() const
{
    return m_nRow;
}

QVariant QTreeItem::data() const
{
    if (m_pFileAttrInfo)
    {
        return QVariant::fromValue(*m_pFileAttrInfo);
    }

    return QVariant();
}

QVariant QTreeItem::getIcon() const
{
    if (m_pFileAttrInfo)
    {
        if (m_pFileAttrInfo->ui64FileUniNum == 0)
        {
            return QIcon(":/QtWidgetsApplication1/res/computer.png");
        }
        else if (m_pFileAttrInfo->ui64FileUniNum == 5)
        {
            TCHAR sysDir[128];
            ::GetSystemDirectory(sysDir, 128 * sizeof(TCHAR));
            CString sysDisk(sysDir[0]);
            if (sysDisk.CompareNoCase(m_pFileAttrInfo->strFilePath) == 0)
            {
                return QIcon(":/QtWidgetsApplication1/res/driversys.png");
            }
            else
            {
                return QIcon(":/QtWidgetsApplication1/res/driver.png");
            }
        }
        else
        {
            if (m_pFileAttrInfo->bIsDir)
            {
                return QIcon(":/QtWidgetsApplication1/res/dir.png");
            }
        }
    }

    return QVariant();
}

QTreeItem* QTreeItem::FindIndexFromItemChild(const quint64& ui64FileNum, const CString& strDriverName) const
{
    if (m_pFileAttrInfo && m_pFileAttrInfo->ui64FileUniNum == ui64FileNum && m_pFileAttrInfo->strFilePath[0] == strDriverName)
    {
        return const_cast<QTreeItem*>(this);
    }
    else
    {
        for (quint32 ui = 0; ui < childCount(); ++ui)
        {
            if (child(ui))
            {
                return child(ui)->FindIndexFromItemChild(ui64FileNum, strDriverName);
            }
        }
        return nullptr;
    }
}
