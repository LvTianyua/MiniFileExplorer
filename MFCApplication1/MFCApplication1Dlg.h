
// MFCApplication1Dlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <vector>
#include <map>
#include "NTFSHelper.h"
#include "afxvslistbox.h"

typedef struct _ItemData
{
    UINT64 ui64FileNum = 0;                    // �ļ��ο���
    CString strLocalDriverName;                // �������̷����� eg��C
    CString strFilePath;                       // �ļ�����·��
    BOOL bIsDir = TRUE;                        // �ǲ����ļ���
    UINT64 ui64FileSize = 0;                   // �ļ���С
    UINT uiChildDirNum = 0;                    // ��ǰitem��Dir����
}ItemData, *PItemData;

// CMFCApplication1Dlg �Ի���
class CMFCApplication1Dlg : public CDialogEx
{
// ����
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
    afx_msg void OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnOpen();
    afx_msg void OnDelete();
    afx_msg void OnCopy();
    afx_msg void OnTie();
    afx_msg void OnCut();
    afx_msg void OnTie2();
    afx_msg void OnUpdateTie(CCmdUI *pCmdUI);
    afx_msg void OnUpdateTie2(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCopy(CCmdUI *pCmdUI);
    afx_msg void OnUpdateCut(CCmdUI *pCmdUI);
    afx_msg void OnLvnEndScrollList2(NMHDR *pNMHDR, LRESULT *pResult);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    virtual BOOL PreTranslateMessage(MSG* pMsg) override;

    // ��ʼ��ͼ���б�
    void InitTreeImageList();
    // Tree ��ĳһ���ڵ�
    void ExpandAnyTreeItem(const HTREEITEM& hTreeItem);
    // ���ݵ�ǰ�ڵ㣬����ӽڵ�
    void AddSubTreeItem(const HTREEITEM& hParentItem);
    // ���һ����TreeItem
    HTREEITEM AddOneTreeItem(const HTREEITEM& hParentItem, const CString& strFileName, const UINT64& ui64FileNum, const CString& strDriverName);
    // ����Item��Item����
    void UpdateOneItemChildDirNum(const ItemData& itemData);

    // ��ʼ��list
    void InitListFiles();
    // ���һ����listItem
    void AddOneListItem(const FileAttrInfo& fileAttrInfo, const CString& strDriverName);
    // ���ݵ�ǰĿ¼�ļ��ο���չʾȫ������list
    void ShowChildList(const int& nItemIndex);
    void ShowChildList(const UINT64& ui64FileNum, const CString& strDriverName, BOOL bForce = FALSE);

    // SYSTIMEתCString
    CString TimeToString(const SYSTEMTIME& systemTime);
    // SizeתCString
    CString SizeToString(const UINT64& ui64FileSize);
    // ���ݺ�׺����ȡ�ļ���Ϣ��ͼ�����������
    SHFILEINFO GetFileBaseInfo(const CString& strExtName);

    // Tree��Listͬ��
    void TreeSyncToList();
    // List��Treeͬ��
    void ListSyncToTree(BOOL bExpand = FALSE);

    // ����ճ����ť�Ƿ����
    void SetTieMenuItemEnable();
public:
    CTreeCtrl& GetTreeMainCtrl() { return m_treeMain; }

private:
    // ��ർ����
    CTreeCtrl m_treeMain;
    // �Ҳ��ļ��б�
    CListCtrl m_listFiles;
    // ��ǰ·����ʾ
    CStatic m_staPath;
    // TreeItem RootItem
    HTREEITEM m_hTreeRootItem;
    // TreeItem ImageList
    CImageList m_treeImageList;
    // TreeItemData
    std::map<HTREEITEM, ItemData> m_mapTreeItemDatas;
    // ListItem ImageList
    CImageList m_listImageList;
    // ListItemData
    std::map<int, ItemData> m_mapListItemDatas;
    // ��¼һ�µ�ǰ����Ŀ¼�ĸ��ο���
    UINT64  m_ui64ParentFileNum = 0;
    // ��¼һ�µ�ǰ����Ŀ¼�Ĳο���
    UINT64  m_ui64CurFileNum = 0;
    // ��׺����imagelist��ӳ��
    std::map<CString, int> m_mapExtIndex;
    // ��ǰĿ¼���������ʵ����
    std::vector<FileAttrInfo> m_vecCurChildAttrInfos;

    BOOL m_bCut = FALSE;
    BOOL m_bCopying = FALSE;
    UINT64 m_ui64SrcFileNum = 0;
    UINT64 m_ui64SrcFileSize = 0;
    UINT m_uiCurChildDirNum = 0;
    CString m_strSrcFilePath;

    CMenu m_menu;
    CMenu* m_menuRButton1 = nullptr;
    CMenu* m_menuRButton2 = nullptr;
public:
    afx_msg void OnTvnItemexpandingTree1(NMHDR *pNMHDR, LRESULT *pResult);
};
