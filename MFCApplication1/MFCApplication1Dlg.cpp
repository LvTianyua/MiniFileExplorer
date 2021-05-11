
// MFCApplication1Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include <memory>
#include "Resource.h"
#include "CopyProgressWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1Dlg �Ի���

#define ONE_TIME_INSERT_LIST_SIZE 100
#define ONE_TIME_INSERT_TREE_SIZE 10000
#define MSG_ASYNC_ADD_TREE_ITEM     WM_USER + 20001 


CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE1, m_treeMain);
    DDX_Control(pDX, IDC_LIST2, m_listFiles);
    DDX_Control(pDX, IDC_STATIC1, m_staPath);
    DDX_Control(pDX, IDC_STATIC2, m_staListCount);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
 	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &CMFCApplication1Dlg::OnNMDblclkList2)
    ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication1Dlg::OnBnClickedButton2)
    ON_NOTIFY(NM_CLICK, IDC_TREE1, &CMFCApplication1Dlg::OnNMClickTree1)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CMFCApplication1Dlg::OnNMDblclkTree1)
    ON_NOTIFY(NM_CLICK, IDC_LIST2, &CMFCApplication1Dlg::OnNMClickList2)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CMFCApplication1Dlg::OnLvnItemchangedList2)
    ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CMFCApplication1Dlg::OnNMRClickList2)
    ON_COMMAND(ID_1_32777, &CMFCApplication1Dlg::OnOpen)
    ON_COMMAND(ID_1_32778, &CMFCApplication1Dlg::OnDelete)
    ON_COMMAND(ID_1_32779, &CMFCApplication1Dlg::OnCopy)
    ON_COMMAND(ID_1_32780, &CMFCApplication1Dlg::OnTie)
    ON_COMMAND(ID_1_32781, &CMFCApplication1Dlg::OnCut)
    ON_COMMAND(ID_2_32782, &CMFCApplication1Dlg::OnTie2)
    ON_UPDATE_COMMAND_UI(ID_1_32780, &CMFCApplication1Dlg::OnUpdateTie)
    ON_UPDATE_COMMAND_UI(ID_2_32782, &CMFCApplication1Dlg::OnUpdateTie2)
    ON_UPDATE_COMMAND_UI(ID_1_32779, &CMFCApplication1Dlg::OnUpdateCopy)
    ON_UPDATE_COMMAND_UI(ID_1_32781, &CMFCApplication1Dlg::OnUpdateCut)
    ON_NOTIFY(LVN_ENDSCROLL, IDC_LIST2, &CMFCApplication1Dlg::OnLvnEndScrollList2)
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE1, &CMFCApplication1Dlg::OnTvnItemexpandingTree1)
    ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, &CMFCApplication1Dlg::OnTvnDeleteitemTree1)
END_MESSAGE_MAP()


// CMFCApplication1Dlg ��Ϣ�������
BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    dwStyle &= ~WS_THICKFRAME;
    SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);

    SetMenu(NULL);
	ShowWindow(SW_SHOWNORMAL);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    InitTreeImageList();
    m_hTreeRootItem = AddOneTreeItem(TVI_ROOT, L"�ҵĵ���", 0, L"");
    AddSubTreeItem(m_hTreeRootItem);
    m_treeMain.Expand(m_hTreeRootItem, TVM_EXPAND);

    InitListFiles();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CMFCApplication1Dlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_RETURN)
        {
            if (GetFocus() == GetDlgItem(IDC_LIST2))
            {
                int nItem = m_listFiles.GetSelectionMark();
                if (nItem >= 0)
                {
                    ShowChildList(nItem);
                    ListSyncToTree(TRUE);
                }
            }
        }
        else if (pMsg->wParam == VK_BACK)
        {
            UINT64 ui64FileNum = m_ui64ParentFileNum;
            ShowChildList(ui64FileNum, CNTFSHelper::GetInstance()->GetCurDriverName());
            if (ui64FileNum == 0)
            {
                m_treeMain.SelectItem(m_hTreeRootItem);
                return S_OK;
            }

            for (const auto& it : m_mapTreeItemDatas)
            {
                if (it.second.ui64FileNum == ui64FileNum)
                {
                    if (ui64FileNum == 5 && it.second.strLocalDriverName != CNTFSHelper::GetInstance()->GetCurDriverName())
                    {
                        continue;
                    }
                    m_treeMain.SelectItem(it.first);
                    break;
                }
            }
        }
        else if (pMsg->wParam == L'C' && (::GetKeyState(VK_CONTROL) & 0x8000))
        {
            OnCopy();
        }
        else if (pMsg->wParam == L'X' && (::GetKeyState(VK_CONTROL) & 0x8000))
        {
            OnCut();
        }
        else if (pMsg->wParam == L'V' && (::GetKeyState(VK_CONTROL) & 0x8000))
        {
            OnTie();
        }
    }
    else if (pMsg->message == WM_MOUSEWHEEL)
    {
//         CRect rc;
//         m_treeMain.GetWindowRect(&rc);
//         if (rc.PtInRect(pMsg->pt))
//         {
//             m_treeMain.OnMouseWheel(0, 0, pMsg->pt);
//         }
    }

    return S_OK;
}

void CMFCApplication1Dlg::InitTreeImageList()
{
    m_treeImageList.Create(25, 25, ILC_COLOR32, 50, 50);
    m_treeMain.SetImageList(&m_treeImageList, LVSIL_NORMAL);

    HICON hIconComputer = AfxGetApp()->LoadIcon(IDI_COMPUTER);
    HICON hIconSysDriver = AfxGetApp()->LoadIcon(IDI_DRIVERSYS);
    HICON hIconDriver = AfxGetApp()->LoadIcon(IDI_DRIVER);
    HICON hIconDir = AfxGetApp()->LoadIcon(IDI_DIR);

    m_treeImageList.Add(hIconComputer);
    m_treeImageList.Add(hIconSysDriver);
    m_treeImageList.Add(hIconDriver);
    m_treeImageList.Add(hIconDir);
}

void CMFCApplication1Dlg::ExpandAnyTreeItem(const HTREEITEM& hTreeItem)
{
    // 1.��յ�ǰitemȫ������������һ��
    HTREEITEM hChildTreeItem = m_treeMain.GetChildItem(hTreeItem);
    if (m_mapTreeItemDatas.find(hChildTreeItem) != m_mapTreeItemDatas.end())
    {
        if (m_mapTreeItemDatas[hChildTreeItem].ui64FileNum == -1)
        {
            m_treeMain.DeleteItem(hChildTreeItem);
            AddSubTreeItem(hTreeItem);
            hChildTreeItem = m_treeMain.GetChildItem(hTreeItem);
            while (hChildTreeItem)
            {
                AddOneTreeItem(hChildTreeItem, L"", -1, L"");
                hChildTreeItem = m_treeMain.GetNextItem(hChildTreeItem, TVGN_NEXT);
            }
        }
        else if (m_mapTreeItemDatas[hChildTreeItem].ui64FileNum == 5)
        {
            while (hChildTreeItem)
            {
                AddOneTreeItem(hChildTreeItem, L"", -1, L"");
                hChildTreeItem = m_treeMain.GetNextItem(hChildTreeItem, TVGN_NEXT);
            }
        }
    }
}

void CMFCApplication1Dlg::AddSubTreeItem(const HTREEITEM& hParentItem)
{
    // ����������Ĳο�����0��˵���Ƕ��㣬���������̷��б�
    if (hParentItem == m_hTreeRootItem)
    {
        if (CNTFSHelper::GetInstance())
        {
            // ��ʼ��֮ǰ��������ʾ
            static bool bInit = true;
            std::vector<CString> vecDriverNames;
            if (bInit)
            {
                bInit = false;
                AsyncMesssageBox(L"������ʾ", L"��ʼ��������Ϣ����ȡ����NTFS�ṹ��...");
                vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
                CloseAsyncMessageBox(L"������ʾ");
            }
            else
            {
                vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
            }
            for (const auto& driverName : vecDriverNames)
            {
                AddOneTreeItem(hParentItem, driverName + L"", 5, driverName);
            }
        }
    }
    else
    {
        // �Ƕ����ҵĵ��ԣ�����������itemData���汣����ļ��ο��ţ����������б����
        ItemData itemData;
        if (m_mapTreeItemDatas.find(hParentItem) != m_mapTreeItemDatas.end())
        {
            itemData = m_mapTreeItemDatas[hParentItem];
        }
        if (itemData.ui64FileNum != 0 && CNTFSHelper::GetInstance())
        {
            CString strLastDriverName = CNTFSHelper::GetInstance()->GetCurDriverName();
            BOOL bDriverChange = (strLastDriverName.Compare(itemData.strLocalDriverName) != 0);
            if (CNTFSHelper::GetInstance()->SetCurDriverInfo(itemData.strLocalDriverName))
            {
                BOOL bSuc = itemData.ui64FileNum == m_ui64CurFileNum;
                if (!bSuc || bDriverChange)
                {
                    // ��ˢ�½��棬��Ϊ��������п���������գ���ˢһ�£����������ȴ�״̬������������
                    BeginWaitCursor();

                    m_vecCurChildAttrInfos.clear();
                    m_uiCurChildDirNum = 0;
                    bSuc = CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(itemData.ui64FileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum);

                    EndWaitCursor();
                }
                if (bSuc)
                {
                    itemData.uiChildDirNum = m_uiCurChildDirNum;
                    UpdateOneItemChildDirNum(itemData);

                    // ��ˢ�½��棬��Ϊ��������п���������գ���ˢһ�£����������ȴ�״̬������������
                    BeginWaitCursor();

                    // һ�����ֻ��10000�� ��ʡʱ�䣬ʣ�µĽ�����̨����߳�ȥ��
                    UINT uiInsertNum = min(m_uiCurChildDirNum, ONE_TIME_INSERT_TREE_SIZE);
                    for (UINT ui = 0; ui < uiInsertNum; ++ui)
                    {
                        // ����������ǰ�涼���ļ��У�����ֻȡǰm_uiCurChildDirNum���Ϳ�����
                        AddOneTreeItem(hParentItem, PathFindFileName(m_vecCurChildAttrInfos[ui].strFilePath), m_vecCurChildAttrInfos[ui].ui64FileUniNum, itemData.strLocalDriverName);
                    }

                    if (m_uiCurChildDirNum > ONE_TIME_INSERT_TREE_SIZE)
                    {
                        for (UINT ui = ONE_TIME_INSERT_TREE_SIZE; ui < m_uiCurChildDirNum; ++ui)
                        {
                            // ����������ǰ�涼���ļ��У�����ֻȡǰm_uiCurChildDirNum���Ϳ�����
                            AddOneTreeItem(hParentItem, PathFindFileName(m_vecCurChildAttrInfos[ui].strFilePath), m_vecCurChildAttrInfos[ui].ui64FileUniNum, itemData.strLocalDriverName);
                        }
                    }

                    EndWaitCursor();
                }
            }
        }
    }
}

HTREEITEM CMFCApplication1Dlg::AddOneTreeItem(const HTREEITEM& hParentItem, const CString& strFileName, const UINT64& ui64FileNum, const CString& strDriverName)
{
    int nImage = -1;
    if (hParentItem == TVI_ROOT)
    {
        nImage = 0;
    }
    else if (hParentItem == m_hTreeRootItem)
    {
        TCHAR sysDir[128]; 
        GetSystemDirectory(sysDir, 128 * sizeof(TCHAR)); 
        CString sysDisk(sysDir[0]); 
        if (sysDisk.CompareNoCase(strDriverName) == 0)
        {
            nImage = 1;
        }
        else
        {
            nImage = 2;
        }
    }
    else
    {
        nImage = 3;
    }
    HTREEITEM hItem = m_treeMain.InsertItem(strFileName, hParentItem);
    m_treeMain.SetItemImage(hItem, nImage, nImage);
    ItemData itemData;
    itemData.strLocalDriverName = strDriverName;
    itemData.ui64FileNum = ui64FileNum;
    if (m_mapTreeItemDatas.find(hItem) != m_mapTreeItemDatas.end())
    {
        m_mapTreeItemDatas[hItem] = itemData;
    }
    else
    {
        m_mapTreeItemDatas.insert(std::make_pair(hItem, itemData));
    }
    return hItem;
}

void CMFCApplication1Dlg::UpdateOneItemChildDirNum(const ItemData& itemData)
{
    for (auto& item : m_mapTreeItemDatas)
    {
        if (item.second.strLocalDriverName == itemData.strLocalDriverName && item.second.ui64FileNum == itemData.ui64FileNum)
        {
            item.second.uiChildDirNum = itemData.uiChildDirNum;
            break;
        }
    }
    for (auto& item : m_mapListItemDatas)
    {
        if (item.second.strLocalDriverName == itemData.strLocalDriverName && item.second.ui64FileNum == itemData.ui64FileNum)
        {
            item.second.uiChildDirNum = itemData.uiChildDirNum;
            break;
        }
    }
}

void CMFCApplication1Dlg::InitListFiles()
{
    DWORD dwStyle = m_listFiles.GetExtendedStyle();
    m_listFiles.SetExtendedStyle(dwStyle | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_DOUBLEBUFFER);

    m_listFiles.InsertColumn(0, L"����", LVCFMT_LEFT, 450);
    m_listFiles.InsertColumn(1, L"�޸�����", LVCFMT_LEFT, 250);
    m_listFiles.InsertColumn(2, L"����", LVCFMT_LEFT, 180);
    m_listFiles.InsertColumn(3, L"��С", LVCFMT_LEFT, 180);

    m_listImageList.Create(25, 25, ILC_COLOR32, 50, 50);
    m_listFiles.SetImageList(&m_listImageList, LVSIL_SMALL);
    HICON hIconSysDriver = AfxGetApp()->LoadIcon(IDI_DRIVERSYS);
    HICON hIconDriver = AfxGetApp()->LoadIcon(IDI_DRIVER);
    HICON hIconDir = AfxGetApp()->LoadIcon(IDI_DIR);
    m_listImageList.Add(hIconSysDriver);
    m_listImageList.Add(hIconDriver);
    m_listImageList.Add(hIconDir);

    if (CNTFSHelper::GetInstance())
    {
        std::vector<CString> vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
        for (const auto& driverName : vecDriverNames)
        {
            FileAttrInfo attrInfo;
            attrInfo.bIsDir = TRUE;
            attrInfo.strFilePath = driverName;
            attrInfo.ui64FileUniNum = 5;
            AddOneListItem(attrInfo, driverName);
        }
    }
}

void CMFCApplication1Dlg::AddOneListItem(const FileAttrInfo& fileAttrInfo, const CString& strDriverName)
{
    int iItemIndex = 0;
    // �����ļ�ͼ��
    int iIconIndex = -1;
    SHFILEINFO info = GetFileBaseInfo(fileAttrInfo.strFilePath);
    if (!fileAttrInfo.bIsDir)
    {
        if (m_mapExtIndex.find(fileAttrInfo.strFilePath) == m_mapExtIndex.end())
        {
            m_listImageList.Add(info.hIcon);
            m_mapExtIndex.insert(std::make_pair(fileAttrInfo.strFilePath, m_listImageList.GetImageCount() - 1));
        }
        iIconIndex = m_mapExtIndex[fileAttrInfo.strFilePath];
    }
    else
    {
        if (fileAttrInfo.ui64FileUniNum == 5)
        {
            TCHAR sysDir[128];
            GetSystemDirectory(sysDir, 128 * sizeof(TCHAR));
            CString sysDisk(sysDir[0]);
            if (sysDisk.CompareNoCase(strDriverName) == 0)
            {
                iIconIndex = 0;
            }
            else
            {
                iIconIndex = 1;
            }
        }
        else
        {
            iIconIndex = 2;
        }
    }
    // ��һ���ļ���
    iItemIndex = m_listFiles.InsertItem(m_listFiles.GetItemCount(), PathFindFileName(fileAttrInfo.strFilePath), iIconIndex);

    // �ڶ����޸�����
    m_listFiles.SetItemText(iItemIndex, 1, TimeToString(fileAttrInfo.stFileModifyTime));

    // ����������
    m_listFiles.SetItemText(iItemIndex, 2, fileAttrInfo.ui64FileUniNum == 5 ? L"NTFS��" : (fileAttrInfo.bIsDir ? L"�ļ���" : info.szTypeName));
    DestroyIcon(info.hIcon);

    // �����д�С
    m_listFiles.SetItemText(iItemIndex, 3, fileAttrInfo.bIsDir ? L"" : SizeToString(fileAttrInfo.ui64FileSize));

    ItemData itemData;
    itemData.strLocalDriverName = strDriverName;
    itemData.ui64FileNum = fileAttrInfo.ui64FileUniNum;
    itemData.strFilePath = fileAttrInfo.strFilePath;
    itemData.bIsDir = fileAttrInfo.bIsDir;
    itemData.ui64FileSize = fileAttrInfo.ui64FileSize;
    m_mapListItemDatas.insert(std::make_pair(iItemIndex, itemData));
}

void CMFCApplication1Dlg::ShowChildList(const int& nItemIndex)
{
    m_listFiles.DeleteAllItems();

    ItemData itemData;
    if (m_mapListItemDatas.find(nItemIndex) != m_mapListItemDatas.end())
    {
        itemData = m_mapListItemDatas[nItemIndex];
    }
    m_mapListItemDatas.clear();

    if (itemData.ui64FileNum != 0 && CNTFSHelper::GetInstance())
    {
        CString strLastDriverName = CNTFSHelper::GetInstance()->GetCurDriverName();
        BOOL bDriverChange = (strLastDriverName.Compare(itemData.strLocalDriverName) != 0);
        if (CNTFSHelper::GetInstance()->SetCurDriverInfo(itemData.strLocalDriverName))
        {
            BOOL bSuc = itemData.ui64FileNum == m_ui64CurFileNum;
            if (!bSuc || bDriverChange)
            {
                // ��ˢ�½��棬��Ϊ��������п���������գ���ˢһ�£����������ȴ�״̬������������
                BeginWaitCursor();

                m_vecCurChildAttrInfos.clear();
                m_uiCurChildDirNum = 0;
                bSuc = CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(itemData.ui64FileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum);

                EndWaitCursor();
            }
            if (bSuc)
            {
                itemData.uiChildDirNum = m_uiCurChildDirNum;
                UpdateOneItemChildDirNum(itemData);

                UINT uiNeedInsertNum = min(m_vecCurChildAttrInfos.size(), ONE_TIME_INSERT_LIST_SIZE);
                for (UINT ui = 0; ui < uiNeedInsertNum; ++ui)
                {
                    AddOneListItem(m_vecCurChildAttrInfos[ui], itemData.strLocalDriverName);
                }
                if (uiNeedInsertNum < m_vecCurChildAttrInfos.size())
                {
                    m_listFiles.RedrawWindow();
                }
            }
        }
    }
    m_ui64CurFileNum = itemData.ui64FileNum;
    CNTFSHelper::GetInstance()->GetParentFileNumByFileNum(itemData.ui64FileNum, m_ui64ParentFileNum);

    // ����·��
    CString strFilePath = itemData.strFilePath;
    if (itemData.ui64FileNum == 5)
    {
        strFilePath += L":\\";
    }
    else
    {
        strFilePath += L"\\";
    }
    m_staPath.SetWindowText(strFilePath);

    // ����Ŀ¼����Ŀ����
    CString strFileCount;
    strFileCount.Format(L"��%u����Ŀ", (UINT)m_vecCurChildAttrInfos.size());
    m_staListCount.SetWindowText(strFileCount);
}

void CMFCApplication1Dlg::ShowChildList(const UINT64& ui64FileNum, const CString& strDriverName)
{
    m_listFiles.DeleteAllItems();
    m_mapListItemDatas.clear();

    if (ui64FileNum == 0)
    {
        if (CNTFSHelper::GetInstance())
        {
            std::vector<CString> vecDriverNames = CNTFSHelper::GetInstance()->GetAllLogicDriversNames();
            for (const auto& driverName : vecDriverNames)
            {
                FileAttrInfo attrInfo;
                attrInfo.bIsDir = TRUE;
                attrInfo.strFilePath = driverName;
                attrInfo.ui64FileUniNum = 5;
                AddOneListItem(attrInfo, driverName);
            }
        }
    }
    if (ui64FileNum != 0 && CNTFSHelper::GetInstance())
    {
        CString strLastDriverName = CNTFSHelper::GetInstance()->GetCurDriverName();
        BOOL bDriverChange = (strLastDriverName.Compare(strDriverName) != 0);
        if (CNTFSHelper::GetInstance()->SetCurDriverInfo(strDriverName))
        {
            BOOL bSuc = ui64FileNum == m_ui64CurFileNum;
            if (!bSuc || bDriverChange)
            {
                // ��ˢ�½��棬��Ϊ��������п���������գ���ˢһ�£����������ȴ�״̬������������
                BeginWaitCursor();

                m_vecCurChildAttrInfos.clear();
                m_uiCurChildDirNum = 0;
                bSuc = CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(ui64FileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum);

                EndWaitCursor();
            }
            if (bSuc)
            {
                ItemData itemData;
                itemData.strLocalDriverName = strDriverName;
                itemData.ui64FileNum = ui64FileNum;
                itemData.uiChildDirNum = m_uiCurChildDirNum;
                UpdateOneItemChildDirNum(itemData);

                UINT uiNeedInsertNum = min(m_vecCurChildAttrInfos.size(), ONE_TIME_INSERT_LIST_SIZE);
                for (UINT ui = 0; ui < uiNeedInsertNum; ++ui)
                {
                    AddOneListItem(m_vecCurChildAttrInfos[ui], strDriverName);
                }
                if (uiNeedInsertNum < m_vecCurChildAttrInfos.size())
                {
                    m_listFiles.RedrawWindow();
                }
            }
        }
    }
    m_ui64CurFileNum = ui64FileNum;
    CNTFSHelper::GetInstance()->GetParentFileNumByFileNum(ui64FileNum, m_ui64ParentFileNum);

    // ����·��
    CString strFilePath;
    if (m_ui64ParentFileNum == 0)
    {
        if (!strDriverName.IsEmpty() && m_ui64CurFileNum != 0)
        {
            strFilePath = strDriverName + L":\\";
        }
    }
    else
    {
        if (CNTFSHelper::GetInstance()->GetFilePathByFileNum(ui64FileNum, strFilePath))
        {
            strFilePath += L"\\";
        }
    }
    m_staPath.SetWindowText(strFilePath);

    // ����Ŀ¼����Ŀ����
    CString strFileCount;
    strFileCount.Format(L"��%u����Ŀ", (UINT)m_vecCurChildAttrInfos.size());
    m_staListCount.SetWindowText(strFileCount);
}

CString CMFCApplication1Dlg::TimeToString(const SYSTEMTIME& systemTime)
{
    CString strTime;
    if (systemTime.wYear == 0 && systemTime.wMonth == 0 && systemTime.wDay == 0 && systemTime.wHour == 0 && systemTime.wMinute == 0)
    {
        return strTime;
    }
    strTime.Format(L"%02d/%02d/%02d %02d:%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute);
    return strTime;
}

CString CMFCApplication1Dlg::SizeToString(const UINT64& ui64FileSize)
{
    CString strSize;
    UINT64 ui64KBSize = ui64FileSize / 1024;
    strSize.Format(L"%I64u KB", ui64KBSize);
    return strSize;
}

SHFILEINFO CMFCApplication1Dlg::GetFileBaseInfo(const CString& strExtName)
{
    SHFILEINFO infoFile;
    SHGetFileInfo(strExtName.IsEmpty() ? L"file" : strExtName,
        FILE_ATTRIBUTE_NORMAL,
        &infoFile,
        sizeof(infoFile),
        SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
    return infoFile;
}

void CMFCApplication1Dlg::TreeSyncToList()
{
    CPoint pt;
    GetCursorPos(&pt);
    m_treeMain.ScreenToClient(&pt);
    UINT uFlags;
    HTREEITEM hItem = m_treeMain.HitTest(pt, &uFlags);

    if (hItem == m_hTreeRootItem)
    {
        ShowChildList(0, L"");
    }
    else
    {
        if (m_mapTreeItemDatas.find(hItem) != m_mapTreeItemDatas.end())
        {
            ItemData itemData;
            itemData = m_mapTreeItemDatas[hItem];
            if (itemData.ui64FileNum != m_ui64CurFileNum || itemData.strLocalDriverName != CNTFSHelper::GetInstance()->GetCurDriverName())
            {
                ShowChildList(itemData.ui64FileNum, itemData.strLocalDriverName);
            }
        }
    }
}

void CMFCApplication1Dlg::ListSyncToTree(BOOL bExpand)
{
    for (const auto& it : m_mapTreeItemDatas)
    {
        if (it.second.ui64FileNum == m_ui64CurFileNum)
        {
            if (it.second.ui64FileNum == 5 && it.second.strLocalDriverName != CNTFSHelper::GetInstance()->GetCurDriverName())
            {
                continue;
            }
            m_treeMain.SelectItem(it.first);
            break;
        }
    }

    if (bExpand)
    {
        m_treeMain.Expand(m_treeMain.GetSelectedItem(), TVM_EXPAND);
    }
}

void CMFCApplication1Dlg::SetTieMenuItemEnable()
{
    if (m_menuRButton1)
    {
        CCmdUI cmdUI;
        cmdUI.m_pMenu = m_menuRButton1;
        cmdUI.m_nIndex = 3;
        cmdUI.m_nIndexMax = 5;
        cmdUI.m_nID = m_menuRButton1->GetMenuItemID(3);
        cmdUI.Enable(m_bCopying);
        cmdUI.DoUpdate(this, FALSE);
    }

    if (m_menuRButton2)
    {
        CCmdUI cmdUI;
        cmdUI.m_pMenu = m_menuRButton2;
        cmdUI.m_nIndex = 0;
        cmdUI.m_nIndexMax = 1;
        cmdUI.m_nID = m_menuRButton2->GetMenuItemID(0);
        cmdUI.Enable(m_bCopying);
        cmdUI.DoUpdate(this, FALSE);
    }
}

void CMFCApplication1Dlg::AsyncMesssageBox(const CString& strCaption, const CString& strText)
{
    std::thread([strCaption, strText]()
    {
        ::MessageBox(NULL, strText, strCaption, MB_OK);
    }).detach();
    BeginWaitCursor();
}

void CMFCApplication1Dlg::CloseAsyncMessageBox(const CString& strCaption)
{
    EndWaitCursor();
    HWND hWnd = ::FindWindowEx(NULL, NULL, NULL, strCaption);
    if (hWnd && ::IsWindow(hWnd))
    {
        ::PostMessage(hWnd, WM_CLOSE, 0, 0); //�����ȷ����
    }
}

void CMFCApplication1Dlg::OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    *pResult = 0;

    if (pNMItemActivate)
    {
        if (m_mapListItemDatas.find(pNMItemActivate->iItem) != m_mapListItemDatas.end())
        {
            if (m_mapListItemDatas[pNMItemActivate->iItem].bIsDir)
            {
                ShowChildList(pNMItemActivate->iItem);
                ListSyncToTree(TRUE);
            }
            else
            {
                ShellExecute(NULL, L"open", m_mapListItemDatas[pNMItemActivate->iItem].strFilePath, NULL, NULL, SW_SHOWNORMAL);
            }
        }
    }
}


void CMFCApplication1Dlg::OnBnClickedButton1()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    UINT64 ui64FileNum = m_ui64ParentFileNum;
    ShowChildList(ui64FileNum, CNTFSHelper::GetInstance()->GetCurDriverName());
    if (ui64FileNum == 0)
    {
        m_treeMain.SelectItem(m_hTreeRootItem);
        return;
    }

    for (const auto& it : m_mapTreeItemDatas)
    {
        if (it.second.ui64FileNum == ui64FileNum)
        {
            if (ui64FileNum == 5 && it.second.strLocalDriverName != CNTFSHelper::GetInstance()->GetCurDriverName())
            {
                continue;
            }
            m_treeMain.SelectItem(it.first);
            break;
        }
    }
}


void CMFCApplication1Dlg::OnBnClickedButton2()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    int nItem = m_listFiles.GetSelectionMark();
    if (nItem >= 0)
    {
        ShowChildList(nItem);
        ListSyncToTree(TRUE);
    }
}

void CMFCApplication1Dlg::OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    TreeSyncToList();
    *pResult = 0;
}


void CMFCApplication1Dlg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    TreeSyncToList();
    *pResult = 0;
}


void CMFCApplication1Dlg::OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    *pResult = 0;
}


void CMFCApplication1Dlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    if (pNMLV)
    {
        if (m_mapListItemDatas.find(pNMLV->iItem) != m_mapListItemDatas.end())
        {
            m_staPath.SetWindowText(m_mapListItemDatas[pNMLV->iItem].strFilePath);
        }
    }
    *pResult = 0;
}


void CMFCApplication1Dlg::OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    int nNUm = m_listFiles.GetSelectedCount();
    if (m_listFiles.GetSelectedCount() == 1)
    {
        int nItem = m_listFiles.GetSelectionMark();
        if (m_mapListItemDatas.find(nItem) != m_mapListItemDatas.end())
        {
            if (!m_mapListItemDatas[nItem].bIsDir)
            {
                if (m_menu.GetSafeHmenu() == NULL)
                {
                    m_menu.LoadMenu(IDR_MENU1);
                }
                if (m_menuRButton1 == nullptr)
                {
                    m_menuRButton1 = m_menu.GetSubMenu(0);
                }
                CPoint myPoint;
                ClientToScreen(&myPoint);
                GetCursorPos(&myPoint); //���λ��
                CCmdUI cmdUI;
                cmdUI.m_pMenu = m_menuRButton1;
                cmdUI.m_nIndex = 3;
                cmdUI.m_nIndexMax = 5;
                cmdUI.m_nID = m_menuRButton1->GetMenuItemID(3);
                cmdUI.Enable(m_bCopying);
                cmdUI.DoUpdate(this, FALSE);
                m_menuRButton1->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
            }
        }
    }
    else
    {
        if (m_ui64CurFileNum != 0)
        {
            if (m_menu.GetSafeHmenu() == NULL)
            {
                m_menu.LoadMenu(IDR_MENU1);
            }
            if (m_menuRButton2 == nullptr)
            {
                m_menuRButton2 = m_menu.GetSubMenu(1);
            }
            CPoint myPoint;
            ClientToScreen(&myPoint);
            GetCursorPos(&myPoint); //���λ��
            CCmdUI cmdUI;
            cmdUI.m_pMenu = m_menuRButton2;
            cmdUI.m_nIndex = 0;
            cmdUI.m_nIndexMax = 1;
            cmdUI.m_nID = m_menuRButton2->GetMenuItemID(0);
            cmdUI.Enable(m_bCopying);
            cmdUI.DoUpdate(this, FALSE);
            m_menuRButton2->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, myPoint.x, myPoint.y, this);
        }
    }
}

void CMFCApplication1Dlg::OnOpen()
{
    // TODO: �ڴ���������������
    if (m_listFiles.GetSelectedCount() == 1)
    {
        int nItem = m_listFiles.GetSelectionMark();
        if (m_mapListItemDatas.find(nItem) != m_mapListItemDatas.end())
        {
            ShellExecute(NULL, L"open", m_mapListItemDatas[nItem].strFilePath, NULL, NULL, SW_SHOWNORMAL);
        }
    }
}

void CMFCApplication1Dlg::OnDelete()
{
    // TODO: �ڴ���������������
    if (m_listFiles.GetSelectedCount() == 1)
    {
        int nItem = m_listFiles.GetSelectionMark();
        if (m_mapListItemDatas.find(nItem) != m_mapListItemDatas.end())
        {
            DeleteFile(m_mapListItemDatas[nItem].strFilePath);
            AsyncMesssageBox(L"ɾ����ʾ", L"�ļ�ɾ����ɣ�����������ȡNTFS�ļ��ṹ...");
            while (true)
            {
                Sleep(500);
                m_uiCurChildDirNum = 0;
                m_vecCurChildAttrInfos.clear();
                if (CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(m_ui64CurFileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum, TRUE))
                {
                    BOOL bQuit = FALSE;
                    for (const auto& info : m_vecCurChildAttrInfos)
                    {
                        UINT64 ui64ParentFileNum = 0;
                        if ((CNTFSHelper::GetInstance()->GetParentFileNumByFileNum(info.ui64FileUniNum, ui64ParentFileNum) && ui64ParentFileNum != m_ui64CurFileNum) || info.strFilePath.Compare(m_mapListItemDatas[nItem].strFilePath) == 0)
                        {
                            bQuit = TRUE;
                            break;
                        }
                    }
                    if (!bQuit)
                    {
                        ShowChildList(m_ui64CurFileNum, CNTFSHelper::GetInstance()->GetCurDriverName());
                        break;
                    }
                }
            }
            CloseAsyncMessageBox(L"ɾ����ʾ");
        }
    }
}

void CMFCApplication1Dlg::OnCopy()
{
    // TODO: �ڴ���������������
    if (m_listFiles.GetSelectedCount() == 1)
    {
        m_bCut = FALSE;
        m_bCopying = TRUE;
        int nItem = m_listFiles.GetSelectionMark();
        if (m_mapListItemDatas.find(nItem) != m_mapListItemDatas.end())
        {
            m_ui64SrcFileNum = m_mapListItemDatas[nItem].ui64FileNum;
            m_ui64SrcFileSize = m_mapListItemDatas[nItem].ui64FileSize;
            m_strSrcFilePath = m_mapListItemDatas[nItem].strFilePath;

            SetTieMenuItemEnable();
        }
    }
}


void CMFCApplication1Dlg::OnTie()
{
    // TODO: �ڴ���������������
    if (!m_bCopying)
    {
        return;
    }

    TCHAR szPath[MAX_PATH + 1] = { 0 };
    std::vector<FileAttrInfo> vecFileAttrInfos;
    if (CNTFSHelper::GetInstance())
    {
        if (m_ui64ParentFileNum == 0)
        {
            CString strTmp = CNTFSHelper::GetInstance()->GetCurDriverName() + L":";
            memcpy(szPath, strTmp, strTmp.GetLength() * 2);
        }
        else
        {
            CString strCurPath;
            if (CNTFSHelper::GetInstance()->GetFilePathByFileNum(m_ui64CurFileNum, strCurPath))
            {
                memcpy(szPath, strCurPath, strCurPath.GetLength() * 2);
            }
        }
        PathAppend(szPath, PathFindFileName(m_strSrcFilePath));
    }

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
        if (AfxMessageBox(L"Ŀ��λ�ô���ͬ���ļ���ȡ������ֹ���β������Ƿ񸲸ǣ�", MB_OKCANCEL) != IDOK)
        {
            m_bCut = FALSE;
            m_bCopying = FALSE;
            m_ui64SrcFileNum = 0;
            m_ui64SrcFileSize = 0;
            m_strSrcFilePath.Empty();
            SetTieMenuItemEnable();
            return;
        }
        DeleteFile(szPath);
    }

    BOOL bSuc = FALSE;
    // �ֿ�� ֱ�ӷŵ����ȴ�������ȥ���������Լ�ά��������ͨ������ֵ֪���Ǳ�ִ�е����
    if (m_ui64SrcFileSize > 4 * 1024 * 1024)
    {
        CCopyProgressWnd copyProgressWnd(this);
        copyProgressWnd.SetCopyInfo(m_ui64SrcFileNum, m_ui64SrcFileSize, m_strSrcFilePath, szPath);
        bSuc = IDOK == copyProgressWnd.DoModal();
    }
    else
    {
        CNTFSHelper::GetInstance()->SetCurDriverInfo(m_strSrcFilePath.Mid(0, 1));
        bSuc = CNTFSHelper::GetInstance() && CNTFSHelper::GetInstance()->MyCopyFile(m_ui64SrcFileNum, m_ui64SrcFileSize, szPath);
        CNTFSHelper::GetInstance()->SetCurDriverInfo(CString(szPath[0]));
    }

    if (bSuc)
    {
        if (m_bCut)
        {
            DeleteFile(m_strSrcFilePath);
        }
        // ��������и��ļ���¼������ʱ�����⣬�����ļ��Ѿ������ɹ��ˣ������ļ���¼��û���£���ȡ���Ĳ��ԣ�
        // ����Ҫһֱˢ�£�ֱ���ļ���¼��ȡ�ɹ�Ϊֹ
         AsyncMesssageBox(L"������ʾ", L"�ļ����ݿ�����ɣ�����������ȡNTFS�ļ��ṹ...");
         while (true)
         {
             Sleep(500);
             m_uiCurChildDirNum = 0;
             m_vecCurChildAttrInfos.clear();
             if (CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(m_ui64CurFileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum, TRUE))
             {
                 BOOL bFinish = FALSE;
                 for (const auto& info : m_vecCurChildAttrInfos)
                 {
                     if (info.strFilePath.Compare(szPath) == 0)
                     {
                         bFinish = TRUE;
                         break;
                     }
                 }
                 if (bFinish)
                 {
                     break;
                 }
             }
         }
         ShowChildList(m_ui64CurFileNum, CNTFSHelper::GetInstance()->GetCurDriverName());
         CloseAsyncMessageBox(L"������ʾ");
    }
    else
    {
        AfxMessageBox(L"�ļ����ƻ���й��̳��ִ���");
    }

    m_bCut = FALSE;
    m_bCopying = FALSE;
    m_ui64SrcFileNum = 0;
    m_ui64SrcFileSize = 0;
    m_strSrcFilePath.Empty();
    SetTieMenuItemEnable();
}


void CMFCApplication1Dlg::OnCut()
{
    // TODO: �ڴ���������������
    if (m_listFiles.GetSelectedCount() == 1)
    {
        m_bCut = TRUE;
        m_bCopying = TRUE;
        int nItem = m_listFiles.GetSelectionMark();
        if (m_mapListItemDatas.find(nItem) != m_mapListItemDatas.end())
        {
            m_ui64SrcFileNum = m_mapListItemDatas[nItem].ui64FileNum;
            m_ui64SrcFileSize = m_mapListItemDatas[nItem].ui64FileSize;
            m_strSrcFilePath = m_mapListItemDatas[nItem].strFilePath;

            SetTieMenuItemEnable();
        }
    }
}


void CMFCApplication1Dlg::OnTie2()
{
    // TODO: �ڴ���������������
    if (!m_bCopying)
    {
        return;
    }

    TCHAR szPath[MAX_PATH + 1] = { 0 };
    std::vector<FileAttrInfo> vecFileAttrInfos;
    if (CNTFSHelper::GetInstance() )
    {
        if (m_ui64ParentFileNum == 0)
        {
            CString strTmp = CNTFSHelper::GetInstance()->GetCurDriverName() + L":";
            memcpy(szPath, strTmp, strTmp.GetLength() * 2);
        }
        else
        {
            CString strCurPath;
            if (CNTFSHelper::GetInstance()->GetFilePathByFileNum(m_ui64CurFileNum, strCurPath))
            {
                memcpy(szPath, strCurPath, strCurPath.GetLength() * 2);
            }
        }
        PathAppend(szPath, PathFindFileName(m_strSrcFilePath));
    }

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
        if (AfxMessageBox(L"Ŀ��λ�ô���ͬ���ļ���ȡ������ֹ���β������Ƿ񸲸ǣ�", MB_OKCANCEL) != IDOK)
        {
            m_bCut = FALSE;
            m_bCopying = FALSE;
            m_ui64SrcFileNum = 0;
            m_ui64SrcFileSize = 0;
            m_strSrcFilePath.Empty();
            SetTieMenuItemEnable();
            return;
        }
        DeleteFile(szPath);
    }

    BOOL bSuc = FALSE;
    // �ֿ�� ֱ�ӷŵ����ȴ�������ȥ���������Լ�ά��������ͨ������ֵ֪���Ǳ�ִ�е����
    if (m_ui64SrcFileSize > 4 * 1024 * 1024)
    {
        CCopyProgressWnd copyProgressWnd(this);
        copyProgressWnd.SetCopyInfo(m_ui64SrcFileNum, m_ui64SrcFileSize, m_strSrcFilePath, szPath);
        bSuc = IDOK == copyProgressWnd.DoModal();
    }
    else
    {
        CNTFSHelper::GetInstance()->SetCurDriverInfo(m_strSrcFilePath.Mid(0, 1));
        bSuc = CNTFSHelper::GetInstance() && CNTFSHelper::GetInstance()->MyCopyFile(m_ui64SrcFileNum, m_ui64SrcFileSize, szPath);
        CNTFSHelper::GetInstance()->SetCurDriverInfo(CString(szPath[0]));
    }

    if (bSuc)
    {
        if (m_bCut)
        {
            DeleteFile(m_strSrcFilePath);
        }
        // ��������и��ļ���¼������ʱ�����⣬�����ļ��Ѿ������ɹ��ˣ������ļ���¼��û���£���ȡ���Ĳ��ԣ�
        // ����Ҫһֱˢ�£�ֱ���ļ���¼��ȡ�ɹ�Ϊֹ
        AsyncMesssageBox(L"������ʾ", L"�ļ����ݿ�����ɣ�����������ȡNTFS�ļ��ṹ...");
        while (true)
        {
            Sleep(500);
            m_uiCurChildDirNum = 0;
            m_vecCurChildAttrInfos.clear();
            if (CNTFSHelper::GetInstance()->GetAllChildInfosByParentRefNum(m_ui64CurFileNum, m_vecCurChildAttrInfos, m_uiCurChildDirNum, TRUE))
            {
                BOOL bFinish = FALSE;
                for (const auto& info : m_vecCurChildAttrInfos)
                {
                    if (info.strFilePath.Compare(szPath) == 0)
                    {
                        bFinish = TRUE;
                        break;
                    }
                }
                if (bFinish)
                {
                    break;
                }
            }
        }
        ShowChildList(m_ui64CurFileNum, CNTFSHelper::GetInstance()->GetCurDriverName());
        CloseAsyncMessageBox(L"������ʾ");
    }
    else
    {
        AfxMessageBox(L"�ļ����ƻ���й��̳��ִ���");
    }

    m_bCut = FALSE;
    m_bCopying = FALSE;
    m_ui64SrcFileNum = 0;
    m_ui64SrcFileSize = 0;
    m_strSrcFilePath.Empty();
    SetTieMenuItemEnable();
}


void CMFCApplication1Dlg::OnUpdateTie(CCmdUI *pCmdUI)
{
    // TODO: �ڴ������������û����洦��������
}


void CMFCApplication1Dlg::OnUpdateTie2(CCmdUI *pCmdUI)
{
    // TODO: �ڴ������������û����洦��������
}


void CMFCApplication1Dlg::OnUpdateCopy(CCmdUI *pCmdUI)
{
    // TODO: �ڴ������������û����洦��������
}


void CMFCApplication1Dlg::OnUpdateCut(CCmdUI *pCmdUI)
{
    // TODO: �ڴ������������û����洦��������
}


void CMFCApplication1Dlg::OnLvnEndScrollList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    // �˹���Ҫ�� Internet Explorer 5.5 ����߰汾��
    // ���� _WIN32_IE ������ >= 0x0560��
    LPNMLVSCROLL pStateChanged = reinterpret_cast<LPNMLVSCROLL>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    UINT uiCurVisibleMaxItem = m_listFiles.GetTopIndex() + (m_listFiles.GetCountPerPage() + 1);
    UINT uiAllItemCount = m_listFiles.GetItemCount();
    if (uiCurVisibleMaxItem == uiAllItemCount && uiAllItemCount < m_vecCurChildAttrInfos.size())
    {
        UINT uiNeedInsertNum = min((m_vecCurChildAttrInfos.size() - uiAllItemCount), ONE_TIME_INSERT_LIST_SIZE);
        for (UINT ui = 0; ui < uiNeedInsertNum; ++ui)
        {
            AddOneListItem(m_vecCurChildAttrInfos[uiAllItemCount + ui], CNTFSHelper::GetInstance()->GetCurDriverName());
        }
    }

    *pResult = 0;
}


void CMFCApplication1Dlg::OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    *pResult = 0;

    if (pNMTreeView && pNMTreeView->action == 2)
    {
        m_treeMain.SetRedraw(FALSE);
        ExpandAnyTreeItem(pNMTreeView->itemNew.hItem);
        m_treeMain.SetRedraw(TRUE);
    }
}

void CMFCApplication1Dlg::OnTvnDeleteitemTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    *pResult = 0;

    if (pNMTreeView)
    {
        HTREEITEM hItem = pNMTreeView->itemOld.hItem;
        auto it = m_mapTreeItemDatas.find(hItem);
        if (it != m_mapTreeItemDatas.end())
        {
            m_mapTreeItemDatas.erase(it);
        }
    }
}
