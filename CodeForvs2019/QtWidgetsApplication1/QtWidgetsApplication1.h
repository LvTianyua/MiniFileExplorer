#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include "TreeItem.h"

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtWidgetsApplication1(QWidget *parent = Q_NULLPTR);

protected:
    void InitBaseUI();
    void InitTreeView();
    void InitTableView();
    void InitAction();
    void AddOneTreeItem(QTreeItem* pParentItem, const pFileAttrInfo pAttrInfo);
    CString GetDriverNameByPath(const CString& strPath);
    void ShowOneItemChildren(const QModelIndex& current);
    void ShowFileList(const UINT64& ui64FileNum, const CString& strFilePath, bool bForceRefresh = false);
    void EnterOneItem(const QModelIndex& index);

protected:
    virtual void keyReleaseEvent(QKeyEvent* ev) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void slotTreeCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotExpanded(const QModelIndex &current);
    void slotTableItemDBClicked(const QModelIndex& index);
    void slotTableCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotMinClicked();
    void slotCloseClicked();

    // 右键菜单相关槽函数
    void slotMenuOpen();
    void slotMenuDel();
    void slotMenuCopy();
    void slotMenuCut();
    void slotMenuPaste();

private:
    Ui::QtWidgetsApplication1Class ui;
    QTreeModel* m_pModelTree = nullptr;
    QTableModel* m_pModelTable = nullptr;
    QMenu m_menuContext;

    quint64                 m_ui64CurFileNum = 0;
    CString                 m_strCurFilePath;

    // 文件复制剪切相关标记
    bool m_bCut = false;
    bool m_bCopying = false;
    quint64 m_ui64SrcFileNum = 0;
    quint64 m_ui64SrcFileSize = 0;
    CString m_strSrcFilePath;

    QProgressDialog* m_pProgressDlg = nullptr;

    QPoint                  m_movePt;
    bool                    m_bPress = false;
};
