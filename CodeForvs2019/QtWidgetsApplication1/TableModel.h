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

    //以下为自定义model需要实现的一些虚函数，将会被Qt在查询model数据时调用
    //data: 核心函数，获取某个索引index的元素的各种数据
    //      role决定获取哪种数据，常用有下面几种：
    //      DisplayRole（默认）：就是界面显示的文本数据
    //      TextAlignmentRole：就是元素的文本对齐属性
    //      TextColorRole、BackgroundRole：分别指文本颜色、单元格背景色
    //parent：获取指定元素的父元素
    //rowCount: 获取指定元素的子节点个数（下一级行数）
    //columnCount: 获取指定元素的列数
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	using QObject::parent;

public:
    void SetVecAttrInfos(const std::vector<FileAttrInfo>& vecInfos) { m_vecAttrInfos = vecInfos; }
    bool GetAttrInfoByIndex(const QModelIndex& index, FileAttrInfo& attrInfo) const;

protected:
    // SYSTIME转CString
    QString TimeToString(const SYSTEMTIME& systemTime) const;
    // Size转CString
    QString SizeToString(const UINT64& ui64FileSize) const;
    // 获取文件图标
    QIcon GetIconByFileAttr(const FileAttrInfo& attrInfo) const;
    // 获取文件类型名
    QString GetFileTypeNameByFilePath(const CString& strFilePath) const;

private:
    std::vector<FileAttrInfo> m_vecAttrInfos;
};

