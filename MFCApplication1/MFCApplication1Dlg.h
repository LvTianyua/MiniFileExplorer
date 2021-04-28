
// MFCApplication1Dlg.h : 头文件
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
    UINT64 ui64FileNum = 0;                    // 文件参考号
    CString strLocalDriverName;                // 所在盘盘符名称 eg：C
    CString strFilePath;                       // 文件绝对路径
    BOOL bIsDir = TRUE;                        // 是不是文件夹
    UINT64 ui64FileSize = 0;                   // 文件大小
    UINT uiChildDirNum = 0;                    // 当前item子Dir数量
}ItemData, *PItemData;

// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

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

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    virtual BOOL PreTranslateMessage(MSG* pMsg) override;

    // 初始化图标列表
    void InitTreeImageList();
    // Tree 打开某一个节点
    void ExpandAnyTreeItem(const HTREEITEM& hTreeItem);
    // 根据当前节点，添加子节点
    void AddSubTreeItem(const HTREEITEM& hParentItem);
    // 添加一个新TreeItem
    HTREEITEM AddOneTreeItem(const HTREEITEM& hParentItem, const CString& strFileName, const UINT64& ui64FileNum, const CString& strDriverName);
    // 更新Item子Item数量
    void UpdateOneItemChildDirNum(const ItemData& itemData);

    // 初始化list
    void InitListFiles();
    // 添加一個新listItem
    void AddOneListItem(const FileAttrInfo& fileAttrInfo, const CString& strDriverName);
    // 根据当前目录文件参考号展示全部子项list
    void ShowChildList(const int& nItemIndex);
    void ShowChildList(const UINT64& ui64FileNum, const CString& strDriverName, BOOL bForce = FALSE);

    // SYSTIME转CString
    CString TimeToString(const SYSTEMTIME& systemTime);
    // Size转CString
    CString SizeToString(const UINT64& ui64FileSize);
    // 根据后缀名获取文件信息（图标和类型名）
    SHFILEINFO GetFileBaseInfo(const CString& strExtName);

    // Tree向List同步
    void TreeSyncToList();
    // List向Tree同步
    void ListSyncToTree(BOOL bExpand = FALSE);

    // 设置粘贴按钮是否可用
    void SetTieMenuItemEnable();
public:
    CTreeCtrl& GetTreeMainCtrl() { return m_treeMain; }

private:
    // 左侧导航树
    CTreeCtrl m_treeMain;
    // 右侧文件列表
    CListCtrl m_listFiles;
    // 当前路径显示
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
    // 记录一下当前所在目录的父参考号
    UINT64  m_ui64ParentFileNum = 0;
    // 记录一下当前所在目录的参考号
    UINT64  m_ui64CurFileNum = 0;
    // 后缀名和imagelist的映射
    std::map<CString, int> m_mapExtIndex;
    // 当前目录下子项集合真实数据
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
