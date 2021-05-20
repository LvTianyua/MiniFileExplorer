#pragma once
#include "stdafx.h"
#include "common.h"
#include "qabstractitemmodel.h"

class QTableModel : public QAbstractTableModel
{
    Q_OBJECT

friend class QtWidgetsApplication1;

public:
    explicit QTableModel(QObject *parent = nullptr);
    ~QTableModel() = default;

    //����Ϊ�Զ���model��Ҫʵ�ֵ�һЩ�麯�������ᱻQt�ڲ�ѯmodel����ʱ����
    //data: ���ĺ�������ȡĳ������index��Ԫ�صĸ�������
    //      role������ȡ�������ݣ����������漸�֣�
    //      DisplayRole��Ĭ�ϣ������ǽ�����ʾ���ı�����
    //      TextAlignmentRole������Ԫ�ص��ı���������
    //      TextColorRole��BackgroundRole���ֱ�ָ�ı���ɫ����Ԫ�񱳾�ɫ
    //parent����ȡָ��Ԫ�صĸ�Ԫ��
    //rowCount: ��ȡָ��Ԫ�ص��ӽڵ��������һ��������
    //columnCount: ��ȡָ��Ԫ�ص�����
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	using QObject::parent;

public:
    void SetVecAttrInfos(const std::vector<FileAttrInfo>& vecInfos) { m_vecAttrInfos = vecInfos; }
    bool GetAttrInfoByIndex(const QModelIndex& index, FileAttrInfo& attrInfo) const;

protected:
    // SYSTIMEתCString
    QString TimeToString(const SYSTEMTIME& systemTime) const;
    // SizeתCString
    QString SizeToString(const UINT64& ui64FileSize) const;
    // ��ȡ�ļ�ͼ��
    QIcon GetIconByFileAttr(const FileAttrInfo& attrInfo) const;
    // ��ȡ�ļ�������
    QString GetFileTypeNameByFilePath(const CString& strFilePath) const;

private:
    std::vector<FileAttrInfo> m_vecAttrInfos;
};

