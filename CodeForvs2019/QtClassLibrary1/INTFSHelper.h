#pragma once
#include "common.h"

class INTFSHelper
{
public:

    /**
    *
    *  ���õ�ǰ������Ϣ
    * @param strDriverName��IN�� �������ƣ�eg��C,D,E...
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual BOOL SetCurDriverInfo(const CString& strDriverName) = 0;

    /**
    *
    *  ��ȡ��ǰ��������
    *
    * @return ��������
    */
    virtual CString GetCurDriverName() = 0;

    /**
    *
    *  ��ȡ�����߼����������Ƽ���
    *
    * @return ��ǰ��������߼��������Ƽ���
    */
    virtual std::vector<CString> GetAllLogicDriversNames() = 0;

    /**
    *
    *  Ntfs������ͨ��Դ�ļ��ο����Լ�Դ�ļ���ʵ��С��������Ŀ��·��������·����
    * @param ui64SrcFileNum��IN�� Դ�ļ��ļ��ο���
    * @param ui64SrcFileSize��IN�� Դ�ļ��ļ���ʵ��С
    * @param strDestPath��IN�� Ŀ���ļ�·��
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath) = 0;

    /**
    *
    *  Ntfs���ݵ�ǰĿ¼�ο��ţ���ȡȫ����Ҫ��ʾ�������
    * @param ui64ParentRefNum��IN�� ��Ŀ¼�ο���
    * @param vecChildAttrInfos��OUT�� �����ļ����Լ���
    * @param uiDirNum��OUT�� ������Ŀ¼������
    * @param bForceFresh��IN�� �Ƿ�ǿ��ˢ��
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum, BOOL bForceFresh) = 0;

    /**
    *
    *  �������ļ��ο��Ż�ȡ���ļ��ο���
    * @param ui64FileNum��IN�� �ļ��ο���
    * @param ui64ParentFileNum��OUT�� ��Ŀ¼�ο���
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum) = 0;

    /**
    *
    *  �����ļ��ο��Ż�ȡ�ļ�·��
    * @param ui64FileNum��IN�� �ļ��ο���
    * @param strFilePath��OUT�� �ļ�·��
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath) = 0;

    /**
    *
    *  ���ý��������ھ��
    * @param hProgressWnd��IN�� ���ȴ��ھ��
    *
    * @return �ɹ� TRUE ʧ�� FALSE
    */
    virtual void SetProgressWndHandle(const HWND& hProgressWnd) = 0;

    /**
    *
    *  ȡ����������
    *
    */
    virtual void CancelCopyTask() = 0;

    /**
    *
    *  ��ȡ��һ�ο���ʧ�������ǲ�����Ϊȡ��ʧ�ܵ�
    *
    * @return �� TRUE �� FALSE
    */
    virtual BOOL IsCopyTaskByCancel() = 0;

    /**
    *
    *  ���ÿ�������ȡ��״̬�ı�־
    *
    */
    virtual void ResetCopyTaskFlag() = 0;
};

