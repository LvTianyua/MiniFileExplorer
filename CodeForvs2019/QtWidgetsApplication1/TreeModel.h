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

    //����Ϊ�Զ���model��Ҫʵ�ֵ�һЩ�麯�������ᱻQt�ڲ�ѯmodel����ʱ����
    //data: ���ĺ�������ȡĳ������index��Ԫ�صĸ�������
    //      role������ȡ�������ݣ����������漸�֣�
    //      DisplayRole��Ĭ�ϣ������ǽ�����ʾ���ı�����
    //      TextAlignmentRole������Ԫ�ص��ı���������
    //      TextColorRole��BackgroundRole���ֱ�ָ�ı���ɫ����Ԫ�񱳾�ɫ
    //flags: ��ȡindex��һЩ��־��һ�㲻��ô��
    //index: Qt�����model����һ������Ϊparent�Ľڵ������row��column���ӽڵ��Ԫ�أ��ڱ�����������Ҫ���ظ�Ԫ�ص���ȷ����
    //parent����ȡָ��Ԫ�صĸ�Ԫ��
    //rowCount: ��ȡָ��Ԫ�ص��ӽڵ��������һ��������
    //columnCount: ��ȡָ��Ԫ�ص�����
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
    QTreeItem *m_pRootItem;    //���ڵ�
};

