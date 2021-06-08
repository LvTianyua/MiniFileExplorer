#include "QtWidgetsApplication1.h"
#include "stdafx.h"
#include "TreeModel.h"
#include "NTFSHelper.h"
#include "TableModel.h"
#include "MyTableView.h"
#include <windowsx.h>

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    InitBaseUI();
    InitTreeView();
    InitTableView();
    InitAction();
}

void QtWidgetsApplication1::InitBaseUI()
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui.widget->layout()->setMargin(0);
    ui.widget_2->layout()->setMargin(0);
    //ui.widget_3->layout()->setMargin(0);
    ui.widget_4->layout()->setMargin(0);

    ui.pushButton->setPicName(QString(":/QtWidgetsApplication1/res/min"));
    connect(ui.pushButton, &QPushButton::clicked, this, &QtWidgetsApplication1::slotMinClicked);
    ui.pushButton_2->setPicName(QString(":/QtWidgetsApplication1/res/close"));
    connect(ui.pushButton_2, &QPushButton::clicked, this, &QtWidgetsApplication1::slotCloseClicked);
}

void QtWidgetsApplication1::InitTreeView()
{
    QTreeView* pTree = ui.treeView;
    if (!pTree)
    {
        return;
    }

    //1��QTreeView����������
    pTree->setEditTriggers(QTreeView::NoEditTriggers);			//��Ԫ���ܱ༭
    pTree->setSelectionBehavior(QTreeView::SelectRows);			//һ��ѡ������
    pTree->setSelectionMode(QTreeView::SingleSelection);        //��ѡ�������������о���һ��ѡ����
    pTree->setFocusPolicy(Qt::NoFocus);                         //ȥ������Ƶ���Ԫ����ʱ�����߿�

    //2����ͷ�������
    pTree->setHeaderHidden(true);

    //3������Model
    //ע�⣺��ʱ��������Զ����TreeModel��
    m_pModelTree = new QTreeModel(pTree);
    if (!m_pModelTree)
    {
        return;
    }

    QTreeItem* root = m_pModelTree->root();
    QTreeItem* pComputer = new QTreeItem(root);
    pFileAttrInfo pRootInfo = new FileAttrInfo;
    pRootInfo->bIsDir = TRUE;
    pRootInfo->strFilePath = L"�ҵĵ���";
    pComputer->setPtr(pRootInfo);
    root->appendChild(pComputer);

    std::vector<CString> vecDriverNames = QNTFSHelper::GetInstance()->GetAllLogicDriversNames();
    for (const auto& driverName : vecDriverNames)
    {
        pFileAttrInfo pInfo = new FileAttrInfo;
        pInfo->bIsDir = TRUE;
        pInfo->strFilePath = driverName;
        pInfo->ui64FileUniNum = 5;
        AddOneTreeItem(pComputer, pInfo);
    }

    //4��Ӧ��model
    pTree->setModel(m_pModelTree);

    QModelIndex index = pTree->model()->index(0, 0);
    pTree->expand(index);

    //5���󶨵�ǰѡ��item�ı��¼�
    connect(pTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &QtWidgetsApplication1::slotTreeCurrentItemChanged);
    connect(pTree, &QTreeView::expanded, this, &QtWidgetsApplication1::slotExpanded);
}

void QtWidgetsApplication1::InitTableView()
{
    QMyTableView* pTable = ui.tableView;
    if (!pTable)
    {
        return;
    }
    //1��QListView����������
    pTable->setEditTriggers(QTreeView::NoEditTriggers);			//��Ԫ���ܱ༭
    pTable->setSelectionBehavior(QTreeView::SelectRows);			//һ��ѡ������
    pTable->setSelectionMode(QTreeView::SingleSelection);        //��ѡ�������������о���һ��ѡ����
    pTable->setMouseTracking(true);
    pTable->setFocusPolicy(Qt::NoFocus);                         //ȥ������Ƶ���Ԫ����ʱ�����߿�
    pTable->setStyleSheet("QTableView::item:selected{background:rgb(204,232,255);border-bottom:1px solid rgb(153,209,255);border-top:1px solid rgb(153,209,255);}");// ����ѡ������ɫ��QSS���ʵ��
    QHeaderView* pHHeader = pTable->horizontalHeader();
    if (pHHeader)
    {
        pHHeader->setSectionsClickable(false);  //���ñ�ͷ���ɵ�
        pHHeader->setDefaultAlignment(Qt::AlignLeft);       //���ñ�ͷ��������룬Ĭ���Ǿ������������
    }

    //3������Model
    //ע�⣺��ʱ��������Զ����ListModel��
    m_pModelTable = new QTableModel(pTable);
    if (!m_pModelTable)
    {
        return;
    }
    std::vector<FileAttrInfo> vecFileInfos;
    std::vector<CString> vecDriverNames = QNTFSHelper::GetInstance()->GetAllLogicDriversNames();
    for (const auto& driverName : vecDriverNames)
    {
        FileAttrInfo info;
        info.bIsDir = TRUE;
        info.strFilePath = driverName;
        info.ui64FileUniNum = 5;
        vecFileInfos.push_back(info);
    }
    if (m_pModelTable)
    {
        m_pModelTable->SetVecAttrInfos(vecFileInfos);
    }

    //4��Ӧ��model
    pTable->setModel(m_pModelTable);
    pTable->setColumnWidth(0, 300);
    pTable->setColumnWidth(1, 250);
    pTable->setColumnWidth(2, 250);
    pTable->setColumnWidth(3, 200);

    //5���󶨵�ǰѡ��item�ı��¼�
    connect(pTable, &QTableView::doubleClicked, this, &QtWidgetsApplication1::slotTableItemDBClicked);
    connect(pTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &QtWidgetsApplication1::slotTableCurrentItemChanged);
}

void QtWidgetsApplication1::InitAction()
{
    // ���������ӣ���ҪΪ����Ӧ��ݼ�
    addAction(ui.action_open);
    addAction(ui.action_del);
    addAction(ui.action_copy);
    addAction(ui.action_cut);
    addAction(ui.action_paste);

    // �Ҽ�����
    m_menuContext.addAction(ui.action_open);
    m_menuContext.addAction(ui.action_del);
    m_menuContext.addAction(ui.action_copy);
    m_menuContext.addAction(ui.action_cut);
    m_menuContext.addAction(ui.action_paste);

    // �󶨲ۺ���
    connect(ui.action_open, &QAction::triggered, this, &QtWidgetsApplication1::slotMenuOpen);
    connect(ui.action_del, &QAction::triggered, this, &QtWidgetsApplication1::slotMenuDel);
    connect(ui.action_copy, &QAction::triggered, this, &QtWidgetsApplication1::slotMenuCopy);
    connect(ui.action_cut, &QAction::triggered, this, &QtWidgetsApplication1::slotMenuCut);
    connect(ui.action_paste, &QAction::triggered, this, &QtWidgetsApplication1::slotMenuPaste);
}

void QtWidgetsApplication1::AddOneTreeItem(QTreeItem* pParentItem, const pFileAttrInfo pAttrInfo)
{
    if (pParentItem)
    {
        QTreeItem* pItem = new QTreeItem(pParentItem);
        if (pItem)
        {
            pItem->setPtr(pAttrInfo);
            pParentItem->appendChild(pItem);
            QTreeItem* pVChildItem = new QTreeItem(pItem);
            if (pVChildItem)
            {
                pFileAttrInfo pVAttrInfo = new FileAttrInfo;
                pVAttrInfo->ui64FileUniNum = -1;
                pVChildItem->setPtr(pVAttrInfo);
                pItem->appendChild(pVChildItem);
            }
        }
    }
}

CString QtWidgetsApplication1::GetDriverNameByPath(const CString& strPath)
{
    return CString(strPath[0]);
}

void QtWidgetsApplication1::ShowOneItemChildren(const QModelIndex& current)
{
    if (m_pModelTree)
    {
        QTreeItem* pItem = m_pModelTree->itemFromIndex(current);
        if (pItem)
        {
            if (pItem->ptr() && pItem->ptr()->ui64FileUniNum == 0)
            {
                return;
            }
            if (pItem->childCount() > 0)
            {
                m_pModelTree->beginRemoveRows(current, 0, pItem->childCount() - 1);
                pItem->removeChilds();
                m_pModelTree->endRemoveRows();
            }
            UINT uiDirNum = 0;
            std::vector<FileAttrInfo> childInfos;
            if (pItem->ptr() && QNTFSHelper::GetInstance() && QNTFSHelper::GetInstance()->SetCurDriverInfo(GetDriverNameByPath(pItem->ptr()->strFilePath))
                && QNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(pItem->ptr()->ui64FileUniNum, childInfos, uiDirNum) && uiDirNum > 0)
            {
                for (UINT ui = 0; ui < uiDirNum && ui < childInfos.size(); ++ui)
                {
                    pFileAttrInfo pInfo = new FileAttrInfo(childInfos[ui]);
                    AddOneTreeItem(pItem, pInfo);
                }
            }
            if (ui.treeView)
            {
                ui.treeView->doItemsLayout();
            }
        }
    }
}

void QtWidgetsApplication1::ShowFileList(const UINT64& ui64FileNum, const CString& strFilePath, bool bForceRefresh/* = false*/)
{
    m_ui64CurFileNum = ui64FileNum;
    m_strCurFilePath = strFilePath;
    if (m_pModelTable->rowCount() > 0)
    {
        m_pModelTable->beginRemoveRows(QModelIndex(), 0, m_pModelTable->rowCount() - 1);
        m_pModelTable->removeRows(0, m_pModelTable->rowCount());
        m_pModelTable->endRemoveRows();
    }
    std::vector<FileAttrInfo> vecFileInfos;
    if (ui64FileNum == 0)
    {
        std::vector<CString> vecDriverNames = QNTFSHelper::GetInstance()->GetAllLogicDriversNames();
        for (const auto& driverName : vecDriverNames)
        {
            FileAttrInfo info;
            info.bIsDir = TRUE;
            info.strFilePath = driverName;
            info.ui64FileUniNum = 5;
            vecFileInfos.push_back(info);
        }
    }
    else
    {
        UINT uiDirNum = 0;
        if (QNTFSHelper::GetInstance() && QNTFSHelper::GetInstance()->SetCurDriverInfo(GetDriverNameByPath(strFilePath))
            && QNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(ui64FileNum, vecFileInfos, uiDirNum, (BOOL)bForceRefresh))
        {
        }
    }
    if (vecFileInfos.size())
    {
        m_pModelTable->beginInsertRows(QModelIndex(), 0, vecFileInfos.size() - 1);
        m_pModelTable->SetVecAttrInfos(vecFileInfos);
        m_pModelTable->endInsertRows();
    }

    if (ui.label)
    {
        QString str = QNTFSHelper::CStringToQString(strFilePath);
        if (ui64FileNum == 5)
        {
            if (!str.contains(u8"\\"))
            {
                str += u8":\\";
            }
        }
        str = u8"·���� " + str;
        if (ui64FileNum == 0)
        {
            str.clear();
        }
        ui.label->setText(str);
    }
    if (ui.label_2)
    {
        QString str;
        str = u8"��" + str.setNum(vecFileInfos.size()) + u8"����Ŀ";
        ui.label_2->setText(str);
    }
}

void QtWidgetsApplication1::EnterOneItem(const QModelIndex& index)
{
    if (index.isValid() && m_pModelTable)
    {
        FileAttrInfo attrInfo;
        if (m_pModelTable->GetAttrInfoByIndex(index, attrInfo))
        {
            if (attrInfo.bIsDir)
            {
                ShowFileList(attrInfo.ui64FileUniNum, attrInfo.strFilePath);
            }
            else
            {
                ::ShellExecute(NULL, L"open", attrInfo.strFilePath, NULL, NULL, SW_SHOWNORMAL);
            }
        }
    }
}

void QtWidgetsApplication1::keyReleaseEvent(QKeyEvent* ev)
{
    if (ui.tableView && ui.tableView->rect().contains(ui.tableView->mapFromGlobal(QCursor::pos())))
    {
        if (ev->key() == Qt::Key_Backspace)
        {
            if (m_ui64CurFileNum != 0)
            {
                quint64 ui64ParentFileNum = 0;
                TCHAR szParentPath[MAX_PATH + 1] = { 0 };
                memcpy(szParentPath, m_strCurFilePath, m_strCurFilePath.GetLength() * 2);
                PathRemoveFileSpec(szParentPath);
                if (QNTFSHelper::GetInstance() && QNTFSHelper::GetInstance()->GetParentFileNumByFileNum(m_ui64CurFileNum, ui64ParentFileNum))
                {
                    ShowFileList(ui64ParentFileNum, szParentPath);
                }
            }
            return;
        }
        else if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
        {
            return EnterOneItem(ui.tableView->selectionModel()->currentIndex());
        }
        else if (ev->key() == Qt::Key_Escape)
        {
            return ui.tableView->selectionModel()->clearSelection();
        }
    }

    QWidget::keyReleaseEvent(ev);
}

void QtWidgetsApplication1::contextMenuEvent(QContextMenuEvent* event)
{
    if (ui.tableView && ui.tableView->rect().contains(ui.tableView->mapFromGlobal(QCursor::pos())))
    {
        m_menuContext.exec(QCursor::pos());
    }
}

bool QtWidgetsApplication1::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    MSG* pMsg = static_cast<MSG*>(message);
    if (pMsg)
    {
        if (pMsg->message == MSG_UPDATE_PROGRESS)
        {
            if (m_pProgressDlg)
            {
                if (m_pProgressDlg->wasCanceled())
                {
                    QNTFSHelper::GetInstance()->CancelCopyTask();
                    return true;
                }
                if ((int)(double)pMsg->wParam > 99)
                {
                    pMsg->wParam = 99;
                }
                m_pProgressDlg->setValue((int)(double)pMsg->wParam);
            }
            return true;
        }
        else if (pMsg->message == WM_NCHITTEST)
        {
            if (isMaximized())
            {
                return false;
            }
            int boundaryWidth = 4;
            int xPos = GET_X_LPARAM(pMsg->lParam) - this->frameGeometry().x();
            int yPos = GET_Y_LPARAM(pMsg->lParam) - this->frameGeometry().y();
            if (xPos < boundaryWidth && yPos < boundaryWidth)                    //���Ͻ�
                *result = HTTOPLEFT;
            else if (xPos >= width() - boundaryWidth && yPos < boundaryWidth)          //���Ͻ�
                *result = HTTOPRIGHT;
            else if (xPos < boundaryWidth && yPos >= height() - boundaryWidth)         //���½�
                *result = HTBOTTOMLEFT;
            else if (xPos >= width() - boundaryWidth && yPos >= height() - boundaryWidth)//���½�
                *result = HTBOTTOMRIGHT;
            else if (xPos < boundaryWidth)                                     //���
                *result = HTLEFT;
            else if (xPos >= width() - boundaryWidth)                              //�ұ�
                *result = HTRIGHT;
            else if (yPos < boundaryWidth)                                       //�ϱ�
                *result = HTTOP;
            else if (yPos >= height() - boundaryWidth)                             //�±�
                *result = HTBOTTOM;
            else              //�������ֲ�������������false�����������¼�����������
                return false;
            return true;
        }
    }
    return false;
}

void QtWidgetsApplication1::paintEvent(QPaintEvent* event)
{
    if (!isMaximized())
    {
        // ����Բ��
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(10, 10, width() - 20, height() - 20, 5, 5);

        // ���ñ�����ɫ�������
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, QBrush(QColor(65, 65, 65)));

        // ���ÿ�����ص㣬������Ӱ��Բ��Ч��
        QColor color(45, 45, 45, 50);
        for (int i = 0; i < 5; i++)
        {
            QPainterPath path1;
            path1.setFillRule(Qt::WindingFill);
            path1.addRoundedRect(5 - i, 5 - i, width() - (5 - i) * 2, height() - (5 - i) * 2, 5, 5);
            color.setAlpha(100 - qSqrt(i) * 50);
            painter.setPen(color);
            painter.drawPath(path1);
        }

        painter.setBrush(QBrush(QColor(245, 245, 245)));
        painter.setPen(Qt::transparent);
        QRect rc = rect();
        rc.setX(5);
        rc.setY(5);
        rc.setWidth(rc.width() - 5);
        rc.setHeight(rc.height() - 5);
        // rect: ��������  15��Բ�ǻ���
        painter.drawRoundedRect(rc, 5, 5);

        // ���ƶ���widget
        QPainterPath path1;
        path1.setFillRule(Qt::WindingFill);
        path1.addRoundedRect(5, 5, width() - 10, ui.widget_4->height() + 10, 5, 5);
        path1.addRect(5, 5 + (ui.widget_4->height() + 10) / 2, width() - 10, (ui.widget_4->height() + 10) / 2 + 1);

        // ���ñ�����ɫ�������
        QPainter painter1(this);
        painter1.setRenderHint(QPainter::Antialiasing, true);
        painter1.fillPath(path1, QBrush(QColor(153, 209, 255)));
    }
    else
    {
        // ����Բ��
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(0, 0, width(), height());

        // ���ñ�����ɫ�������
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, QBrush(QColor(245, 245, 245)));

        // ���ƶ���widget
        QPainterPath path1;
        path1.setFillRule(Qt::WindingFill);
        path1.addRect(0, 0, width(), ui.widget_4->height() + 15);

        // ���ñ�����ɫ�������
        QPainter painter1(this);
        painter1.setRenderHint(QPainter::Antialiasing, true);
        painter1.fillPath(path1, QBrush(QColor(153, 209, 255)));
    }
}

void QtWidgetsApplication1::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_movePt = event->pos();
        m_bPress = true;
    }
}

void QtWidgetsApplication1::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton && m_bPress)
    {
        if (!isMaximized())
        {
            move(event->globalPos() - m_movePt);
        }
    }
}

void QtWidgetsApplication1::mouseReleaseEvent(QMouseEvent* event)
{
    m_bPress = false;
}

void QtWidgetsApplication1::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (isMaximized())
        {
            showNormal();
        }
        else
        {
            showMaximized();
        }
    }
}

void QtWidgetsApplication1::slotTreeCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid() || !m_pModelTable)
    {
        return;
    }

    QTreeItem* pItem = m_pModelTree->itemFromIndex(current);
    if (pItem && pItem->ptr())
    {
        ShowFileList(pItem->ptr()->ui64FileUniNum, pItem->ptr()->strFilePath);
    }
}

void QtWidgetsApplication1::slotExpanded(const QModelIndex &current)
{
    if (current.isValid())
    {
        ShowOneItemChildren(current);
    }
}

void QtWidgetsApplication1::slotTableItemDBClicked(const QModelIndex& index)
{
    if (qApp && qApp->mouseButtons() == Qt::LeftButton)
    {
        EnterOneItem(index);
    }
}

void QtWidgetsApplication1::slotTableCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous)
{
    ui.action_open->setEnabled(false);
    ui.action_del->setEnabled(false);
    ui.action_copy->setEnabled(false);
    ui.action_cut->setEnabled(false);
    ui.action_paste->setEnabled(m_bCopying);

    FileAttrInfo fileAttrInfo;
    if (m_pModelTable)
    {
        if (m_pModelTable->GetAttrInfoByIndex(current, fileAttrInfo))
        {
            if (ui.label)
            {
                QString str = QNTFSHelper::CStringToQString(fileAttrInfo.strFilePath);
                if (fileAttrInfo.ui64FileUniNum == 5)
                {
                    str += u8":\\";
                }
                str = u8"·���� " + str;
                ui.label->setText(str);
            }

            if (fileAttrInfo.ui64FileUniNum == 0 || fileAttrInfo.ui64FileUniNum == 5 || fileAttrInfo.bIsDir)
            {
                ui.action_open->setEnabled(true);
                ui.action_del->setEnabled(false);
                ui.action_copy->setEnabled(false);
                ui.action_cut->setEnabled(false);
                ui.action_paste->setEnabled(false);
            }
            else
            {
                ui.action_open->setEnabled(true);
                ui.action_del->setEnabled(true);
                ui.action_copy->setEnabled(true);
                ui.action_cut->setEnabled(true);
                ui.action_paste->setEnabled(m_bCopying);
            }
        }
    }
}

void QtWidgetsApplication1::slotMinClicked()
{
    showMinimized();
}

void QtWidgetsApplication1::slotCloseClicked()
{
    close();
}

void QtWidgetsApplication1::slotMenuOpen()
{
    if (ui.tableView && ui.tableView->selectionModel())
    {
        EnterOneItem(ui.tableView->selectionModel()->currentIndex());
    }
}

void QtWidgetsApplication1::slotMenuDel()
{
    if (QMessageBox::question(this, u8"ɾ���ļ�", u8"ȷ��ɾ�����ļ�?", u8"ȷ��", u8"ȡ��") == 0)
    {
        if (ui.tableView && ui.tableView->selectionModel() && m_pModelTable)
        {
            FileAttrInfo attrInfo;
            if (m_pModelTable->GetAttrInfoByIndex(ui.tableView->selectionModel()->currentIndex(), attrInfo))
            {
                if (QFile(QNTFSHelper::CStringToQString(attrInfo.strFilePath)).remove())
                {
                    ShowFileList(m_ui64CurFileNum, m_strCurFilePath, true);
                    QMessageBox::information(this, u8"��ʾ", u8"�ļ�ɾ����ɣ�");
                }
                else
                {
                    QMessageBox::critical(this, u8"����", u8"�ļ�ɾ��ʧ�ܣ������ļ�δ�رջ�Ȩ�޲��㣡");
                }
            }
        }
    }
}

void QtWidgetsApplication1::slotMenuCopy()
{
    m_bCut = false;
    m_bCopying = true;
    if (ui.action_paste)
    {
        ui.action_paste->setEnabled(true);
    }

    if (ui.tableView && ui.tableView->selectionModel() && m_pModelTable)
    {
        FileAttrInfo attrInfo;
        if (m_pModelTable->GetAttrInfoByIndex(ui.tableView->selectionModel()->currentIndex(), attrInfo))
        {
            m_ui64SrcFileNum = attrInfo.ui64FileUniNum;
            m_ui64SrcFileSize = attrInfo.ui64FileSize;
            m_strSrcFilePath = attrInfo.strFilePath;
        }
    }
}

void QtWidgetsApplication1::slotMenuCut()
{
    m_bCut = true;
    m_bCopying = true;

    if (ui.tableView && ui.tableView->selectionModel() && m_pModelTable)
    {
        FileAttrInfo attrInfo;
        if (m_pModelTable->GetAttrInfoByIndex(ui.tableView->selectionModel()->currentIndex(), attrInfo))
        {
            m_ui64SrcFileNum = attrInfo.ui64FileUniNum;
            m_ui64SrcFileSize = attrInfo.ui64FileSize;
            m_strSrcFilePath = attrInfo.strFilePath;
        }
    }
}

void QtWidgetsApplication1::slotMenuPaste()
{
    if (!m_bCopying)
    {
        return;
    }

    // ƴװĿ��·��
    TCHAR szPath[MAX_PATH + 1] = { 0 };
    if (m_ui64CurFileNum == 5)
    {
        CString strTmp = QNTFSHelper::GetInstance()->GetCurDriverName() + L":";
        memcpy(szPath, strTmp, strTmp.GetLength() * 2);
    }
    else
    {
        memcpy(szPath, m_strCurFilePath, m_strCurFilePath.GetLength() * 2);
    }
    PathAppend(szPath, PathFindFileName(m_strSrcFilePath));

    // ���Դ·����Ŀ��·�� ��ȫһ�� ֱ�����ɸ���
    if (m_strSrcFilePath.Compare(szPath) == 0)
    {
        CString strExt = PathFindExtension(szPath);
        PathRemoveExtension(szPath);
        CString strTmp(szPath);
        strTmp += L" - ����";
        strTmp += strExt;
        ZeroMemory(szPath, MAX_PATH + 1);
        memcpy(szPath, strTmp, strTmp.GetLength() * 2);
        UINT uiNum = 2;
        while (PathFileExists(szPath))
        {
            if (strTmp.Compare(szPath) != 0)
            {
                ZeroMemory(szPath, MAX_PATH + 1);
                memcpy(szPath, strTmp, strTmp.GetLength() * 2);
            }
            CString strExt = PathFindExtension(szPath);
            PathRemoveExtension(szPath);
            CString strTmp1(szPath);
            strTmp1.Format(L"%s (%u)", strTmp1, uiNum);
            strTmp1 += strExt;
            ZeroMemory(szPath, MAX_PATH + 1);
            memcpy(szPath, strTmp1, strTmp1.GetLength() * 2);
            ++uiNum;
        }
    }

    // ���Ŀ��λ�ô���ͬ���ļ����ȵ�����ʾ�����ǲ��Ǹ���
    if (PathFileExists(szPath))
    {
        if (QMessageBox::question(this, u8"������ʾ", u8"Ŀ��λ�ô���ͬ���ļ���ȡ������ֹ���β������Ƿ񸲸ǣ�", u8"ȷ��", u8"ȡ��") != 0)
        {
            m_bCut = false;
            m_bCopying = false;
            m_ui64SrcFileNum = 0;
            m_ui64SrcFileSize = 0;
            m_strSrcFilePath.Empty();
            return;
        }
        DeleteFile(szPath);
    }

    bool bSuc = false;
    // �ֿ�� ֱ�ӷŵ����ȴ�������ȥ���������Լ�ά��������ͨ������ֵ֪���Ǳ�ִ�е����
    if (m_ui64SrcFileSize > 4 * 1024 * 1024)
    {
        if (m_pProgressDlg == nullptr)
        {
            m_pProgressDlg = new QProgressDialog(u8"������...", QString(), 0, 100, this, Qt::WindowCloseButtonHint);
        }
        m_pProgressDlg->setWindowModality(Qt::WindowModal);
        m_pProgressDlg->setMinimumDuration(1000);

        if (QNTFSHelper::GetInstance())
        {
            QNTFSHelper::GetInstance()->SetProgressWndHandle((HWND)winId());
            QNTFSHelper::GetInstance()->SetCurDriverInfo(m_strSrcFilePath[0]);
            bSuc = QNTFSHelper::GetInstance()->MyCopyFile(m_ui64SrcFileNum, m_ui64SrcFileSize, szPath);
            QNTFSHelper::GetInstance()->SetCurDriverInfo(CString(szPath)[0]);
        }
    }
    else
    {
        QNTFSHelper::GetInstance()->SetCurDriverInfo(m_strSrcFilePath[0]);
        bSuc = QNTFSHelper::GetInstance() && QNTFSHelper::GetInstance()->MyCopyFile(m_ui64SrcFileNum, m_ui64SrcFileSize, szPath);
        QNTFSHelper::GetInstance()->SetCurDriverInfo(CString(szPath[0]));
    }

    if (bSuc)
    {
        if (m_bCut)
        {
            DeleteFile(m_strSrcFilePath);
        }
        ShowFileList(m_ui64CurFileNum, m_strCurFilePath, true);
        if (m_pProgressDlg)
        {
            m_pProgressDlg->setValue(100);
        }
        QMessageBox::information(this, u8"��ʾ", u8"�ļ�������ɣ�");
    }
    else
    {
        if (QNTFSHelper::GetInstance())
        {
            if (!QNTFSHelper::GetInstance()->IsCopyTaskByCancel())
            {
                QMessageBox::critical(this, u8"����", u8"�ļ����ƻ���й��̳��ִ���");
            }
            else
            {
                if (m_pProgressDlg)
                {
                    m_pProgressDlg->reset();
                }
                QNTFSHelper::GetInstance()->ResetCopyTaskFlag();
                ShowFileList(m_ui64CurFileNum, m_strCurFilePath, true);
            }
        }
    }

    m_bCut = false;
    m_bCopying = false;
    m_ui64SrcFileNum = 0;
    m_ui64SrcFileSize = 0;
    m_strSrcFilePath.Empty();
}