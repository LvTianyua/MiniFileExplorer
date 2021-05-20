#include "QtWidgetsApplication1.h"
#include "stdafx.h"
#include "TreeModel.h"
#include "NTFSHelper.h"
#include "TableModel.h"
#include "MyTableView.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    InitTreeView();
    InitTableView();
    InitContextMenu();
}

void QtWidgetsApplication1::InitTreeView()
{
    QTreeView* pTree = ui.treeView;
    if (!pTree)
    {
        return;
    }

    //1，QTreeView常用设置项
    pTree->setEditTriggers(QTreeView::NoEditTriggers);			//单元格不能编辑
    pTree->setSelectionBehavior(QTreeView::SelectRows);			//一次选中整行
    pTree->setSelectionMode(QTreeView::SingleSelection);        //单选，配合上面的整行就是一次选单行
    pTree->setFocusPolicy(Qt::NoFocus);                         //去掉鼠标移到单元格上时的虚线框

    //2，列头相关设置
    pTree->setHeaderHidden(true);

    //3，构造Model
    //注意：此时构造的是自定义的TreeModel！
    m_pModelTree = new QTreeModel(pTree);
    if (!m_pModelTree)
    {
        return;
    }

    QTreeItem* root = m_pModelTree->root();
    QTreeItem* pComputer = new QTreeItem(root);
    pFileAttrInfo pRootInfo = new FileAttrInfo;
    pRootInfo->bIsDir = TRUE;
    pRootInfo->strFilePath = L"我的电脑";
    pComputer->setPtr(pRootInfo);
    root->appendChild(pComputer);

    std::vector<CString> vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
    for (const auto& driverName : vecDriverNames)
    {
        pFileAttrInfo pInfo = new FileAttrInfo;
        pInfo->bIsDir = TRUE;
        pInfo->strFilePath = driverName;
        pInfo->ui64FileUniNum = 5;
        AddOneTreeItem(pComputer, pInfo);
    }

    //4，应用model
    pTree->setModel(m_pModelTree);

    QModelIndex index = pTree->model()->index(0, 0);
    pTree->expand(index);

    //5，绑定当前选中item改变事件
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
    //1，QListView常用设置项
    pTable->setEditTriggers(QTreeView::NoEditTriggers);			//单元格不能编辑
    pTable->setSelectionBehavior(QTreeView::SelectRows);			//一次选中整行
    pTable->setSelectionMode(QTreeView::SingleSelection);        //单选，配合上面的整行就是一次选单行
    pTable->setFocusPolicy(Qt::NoFocus);                         //去掉鼠标移到单元格上时的虚线框
    pTable->setStyleSheet("QTableView::item{selection-background-color:rgb(135,206,250);}");// 设置选中行颜色，QSS语句实现
    QHeaderView* pHHeader = pTable->horizontalHeader();
    if (pHHeader)
    {
        pHHeader->setSectionsClickable(false);  //设置表头不可点
        pHHeader->setDefaultAlignment(Qt::AlignLeft);       //设置表头文字左对齐，默认是居中如果不设置
    }

    //3，构造Model
    //注意：此时构造的是自定义的ListModel！
    m_pModelTable = new QTableModel(pTable);
    if (!m_pModelTable)
    {
        return;
    }
    std::vector<FileAttrInfo> vecFileInfos;
    std::vector<CString> vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
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

    //4，应用model
    pTable->setModel(m_pModelTable);
	pTable->setColumnWidth(0, 300);
	pTable->setColumnWidth(1, 250);
	pTable->setColumnWidth(2, 250);
	pTable->setColumnWidth(3, 200);

    //5，绑定当前选中item改变事件
    connect(pTable, &QTableView::doubleClicked, this, &QtWidgetsApplication1::slotTableItemDBClicked);
	connect(pTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &QtWidgetsApplication1::slotTableCurrentItemChanged);
}

void QtWidgetsApplication1::InitContextMenu()
{
    m_menuContext.addAction(ui.action_open);
	m_menuContext.addAction(ui.action_del);
	m_menuContext.addAction(ui.action_copy);
	m_menuContext.addAction(ui.action_cut);
	m_menuContext.addAction(ui.action_paste);

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
            if (pItem->ptr() && CNTFSHelper::GetInstance() && CNTFSHelper::GetInstance()->SetCurDriverInfo(GetDriverNameByPath(pItem->ptr()->strFilePath))
                && CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(pItem->ptr()->ui64FileUniNum, childInfos, uiDirNum) && uiDirNum > 0)
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
		std::vector<CString> vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
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
		if (CNTFSHelper::GetInstance() && CNTFSHelper::GetInstance()->SetCurDriverInfo(GetDriverNameByPath(strFilePath))
			&& CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(ui64FileNum, vecFileInfos, uiDirNum, (BOOL)bForceRefresh))
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
		QString str = CNTFSHelper::CStringToQString(strFilePath);
        if (ui64FileNum == 5)
        {
            if (!str.contains(u8"\\"))
            {
				str += u8":\\";
            }
        }
		str = u8"路径： " + str;
		if (ui64FileNum == 0)
		{
			str.clear();
		}
		ui.label->setText(str);
	}
    if (ui.label_2)
    {
        QString str;
        str = u8"共" + str.setNum(vecFileInfos.size()) + u8"个项目";
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
				if (CNTFSHelper::GetInstance() && CNTFSHelper::GetInstance()->GetParentFileNumByFileNum(m_ui64CurFileNum, ui64ParentFileNum))
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
        if (ui.tableView->selectionModel())
        {
			ui.action_open->setEnabled(false);
			ui.action_del->setEnabled(false);
			ui.action_copy->setEnabled(false);
			ui.action_cut->setEnabled(false);
			ui.action_paste->setEnabled(false);
			QModelIndex index = ui.tableView->selectionModel()->currentIndex();
            if (index.isValid())
            {
				FileAttrInfo attrInfo;
				if (m_pModelTable && m_pModelTable->GetAttrInfoByIndex(index, attrInfo))
				{
                    if (attrInfo.ui64FileUniNum == 0 || attrInfo.ui64FileUniNum == 5 || attrInfo.bIsDir)
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
						ui.action_paste->setEnabled(true);
                    }
				}
            }
			m_menuContext.exec(QCursor::pos());
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
    FileAttrInfo fileAttrInfo;
    if (m_pModelTable)
    {
        m_pModelTable->GetAttrInfoByIndex(current, fileAttrInfo);
		if (ui.label)
		{
			QString str = CNTFSHelper::CStringToQString(fileAttrInfo.strFilePath);
			if (fileAttrInfo.ui64FileUniNum == 5)
			{
				str += u8":\\";
			}
			str = u8"路径： " + str;
			ui.label->setText(str);
		}
    }
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
	int choose = QMessageBox::question(this, u8"删除文件", u8"确认删除该文件?", u8"确定", u8"取消");
	if (choose == 0) 
    {
		if (ui.tableView && ui.tableView->selectionModel() && m_pModelTable)
		{
			FileAttrInfo attrInfo;
			if (m_pModelTable->GetAttrInfoByIndex(ui.tableView->selectionModel()->currentIndex(), attrInfo))
			{
				if (QFile(CNTFSHelper::CStringToQString(attrInfo.strFilePath)).remove())
				{
					ShowFileList(m_ui64CurFileNum, m_strCurFilePath, true);
					QMessageBox::information(this, u8"提示", u8"文件删除完成！");
				}
				else
				{
					QMessageBox::critical(this, u8"错误", u8"文件删除失败，可能文件未关闭或权限不足！");
				}
			}
		}
	}
}

void QtWidgetsApplication1::slotMenuCopy()
{

}

void QtWidgetsApplication1::slotMenuCut()
{

}

void QtWidgetsApplication1::slotMenuPaste()
{

}
