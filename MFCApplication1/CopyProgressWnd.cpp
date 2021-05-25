// CopyProgressWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "CopyProgressWnd.h"
#include "afxdialogex.h"
#include "NTFSHelper.h"

// CCopyProgressWnd 对话框

IMPLEMENT_DYNAMIC(CCopyProgressWnd, CDialogEx)

CCopyProgressWnd::CCopyProgressWnd(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CCopyProgressWnd::~CCopyProgressWnd()
{
}

BOOL CCopyProgressWnd::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    ShowWindow(SW_SHOWNORMAL);

    if (CNTFSHelper::GetInstance())
    {
        CNTFSHelper::GetInstance()->SetProgressWndHandle(GetSafeHwnd());
        CNTFSHelper::GetInstance()->SetCurDriverInfo(m_strSrcFilePath.Mid(0, 1));
        std::thread([=]() 
        {
            if (CNTFSHelper::GetInstance()->MyCopyFile(m_ui64FileNum, m_ui64FileSize, m_strDestFilePath))
            {
                CNTFSHelper::GetInstance()->SetCurDriverInfo(CString(m_strDestFilePath.Mid(0, 1)));
                PostMessage(MSG_END_PROGRESS_WND, (WPARAM)IDOK);
            }
            else
            {
                CNTFSHelper::GetInstance()->SetCurDriverInfo(CString(m_strDestFilePath.Mid(0, 1)));
                PostMessage(MSG_END_PROGRESS_WND, (WPARAM)IDCANCEL);
            }
        }).detach();
    }

    return TRUE;
}

void CCopyProgressWnd::UpdateProgress(const double& dProgress)
{
    m_dCurProgress = dProgress;
    if (m_dCurProgress < 0)
    {
        m_dCurProgress = 0;
    }
    if (m_dCurProgress > 100)
    {
        m_dCurProgress = 100;
    }
    m_progressCopy.SetPos(m_dCurProgress);
    CString strText;
    strText.Format(L"%d%%", (int)m_dCurProgress);
    m_staProgress.SetWindowText(strText);
}

void CCopyProgressWnd::SetCopyInfo(const UINT64& ui64FileNum, const UINT64& ui64FileSize, const CString& strSrcFilePath, const CString& strDestFilePath)
{
    m_ui64FileNum = ui64FileNum;
    m_ui64FileSize = ui64FileSize;
    m_strSrcFilePath = strSrcFilePath;
    m_strDestFilePath = strDestFilePath;
}

LRESULT CCopyProgressWnd::OnUpdateProgress(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    double dPercent = (double)wParam;
    UpdateProgress(dPercent);
    return S_OK;
}

LRESULT CCopyProgressWnd::OnEndProgressWnd(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    EndDialog((BOOL)wParam);
    return S_OK;
}

void CCopyProgressWnd::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_progressCopy);
    DDX_Control(pDX, IDC_STATIC2, m_staProgress);
}


BEGIN_MESSAGE_MAP(CCopyProgressWnd, CDialogEx)
    ON_MESSAGE(MSG_UPDATE_PROGRESS, &CCopyProgressWnd::OnUpdateProgress)
    ON_MESSAGE(MSG_END_PROGRESS_WND, &CCopyProgressWnd::OnEndProgressWnd)
END_MESSAGE_MAP()


// CCopyProgressWnd 消息处理程序
