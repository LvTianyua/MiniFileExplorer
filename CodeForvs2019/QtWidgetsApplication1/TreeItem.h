#pragma once
#include "QVariant"
#include "INTFSHelper.h"

class QTreeItem
{
public:
    explicit QTreeItem(QTreeItem *pParentItem = nullptr);
    ~QTreeItem();

    void appendChild(QTreeItem *pChild);      //�ڱ��ڵ��������ӽڵ�
    void removeChilds();                    //������нڵ�

    QTreeItem *child(int nRow) const;               //��ȡ��row���ӽڵ�ָ��
    QTreeItem *parentItem();                 //��ȡ���ڵ�ָ��
    int childCount() const;                 //�ӽڵ����
    int row() const;                        //��ȡ�ýڵ��Ǹ��ڵ�ĵڼ����ӽڵ�

    QVariant data() const;                  //���ĺ�������ȡ�ڵ�����
    QVariant getIcon() const;               //��ȡͼ��

    //���á���ȡ�ڵ�������ָ��
    void setPtr(const pFileAttrInfo pAttrInfo) { m_pFileAttrInfo = pAttrInfo; }
    pFileAttrInfo ptr() { return m_pFileAttrInfo; }

    //����ýڵ����丸�ڵ�ĵڼ����ӽڵ㣬��ѯ�Ż�����
    void setRow(int nRow) {
        m_nRow = nRow;
    }

	QTreeItem* FindIndexFromItemChild(const quint64& ui64FileNum, const CString& strDriverName) const;

private:
    QVector<QTreeItem*> m_vecChildItems;                        //�ӽڵ�
    QTreeItem *m_parentItem = nullptr;                                    //���ڵ�
    pFileAttrInfo m_pFileAttrInfo = nullptr;     //�洢���ݵ�ָ��
    int m_nRow = 0;                                             //��¼��item�ǵڼ��������Ż���ѯЧ��
};

