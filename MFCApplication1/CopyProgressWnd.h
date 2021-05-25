#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CCopyProgressWnd �Ի���

class CCopyProgressWnd : public CDialogEx
{
	DECLARE_DYNAMIC(CCopyProgressWnd)

public:
	CCopyProgressWnd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCopyProgressWnd();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

    virtual BOOL OnInitDialog();
    void UpdateProgress(const double& dProgress);
    void SetCopyInfo(const UINT64& ui64FileNum, const UINT64& ui64FileSize, const CString& strSrcFilePath, const CString& strDestFilePath);

    afx_msg LRESULT OnUpdateProgress(WPARAM wParam = 0, LPARAM lParam = 0);
    afx_msg LRESULT OnEndProgressWnd(WPARAM wParam = 0, LPARAM lParam = 0);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
    CProgressCtrl m_progressCopy;
    CStatic m_staProgress;

    double m_dCurProgress = 0;

    UINT64 m_ui64FileNum = 0;
    UINT64 m_ui64FileSize = 0;
    CString m_strSrcFilePath;
    CString m_strDestFilePath;
};
