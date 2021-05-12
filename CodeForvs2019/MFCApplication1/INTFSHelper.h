#pragma once
#include "stdafx.h"
#include <vector>

#define MSG_UPDATE_PROGRESS  WM_USER + 0x01

#pragma pack(1)
typedef struct _NTFSDBR {
    BYTE JMP[3];	//��תָ��
    BYTE FsID[8];	//�ļ�ϵͳID
    unsigned short int bytePerSector = 0;	//ÿ�����ֽ���
    BYTE secPerCluster = 0;		//ÿ��������
    BYTE reservedBytes[2];	//2�������ֽ�
    BYTE zeroBytes[3];	//����0�ֽ�
    BYTE unusedBytes1[2];	//2��δ���ֽ�
    BYTE mediaType = 0;//ý������
    BYTE unusedBytes2[2];	//2��δ���ֽ�
    unsigned short int secPerTrack = 0;	//ÿ�ŵ�������
    unsigned short int Heads = 0;	//��ͷ��
    unsigned int hideSectors = 0;	//����������
    BYTE unusedBytes3[4];	//4��δ���ֽ�
    BYTE usedBytes[4];	//4���̶��ֽ�
    unsigned __int64 totalSectors = 0;	//��������
    unsigned __int64 MFT = 0;	//MFT��ʼ�غ�
    unsigned __int64 MFTMirror = 0;	//MFTMirror�ļ���ʼ�غ�
    char fileRecord = 0;	//�ļ���¼
    BYTE unusedBytes4[3];	//3��δ���ֽ�
    char indexSize = 0;	//������������С
    BYTE unusedBytes5[3];	//δ���ֽ�
    BYTE volumeSerialID64[8];	//�����к�
    unsigned int checkSum = 0;	//У���
    BYTE bootCode[426];	//��������
    BYTE endSignature[2];	//������־

    _NTFSDBR()
    {
        ZeroMemory(JMP, 3);
        ZeroMemory(FsID, 8);
        ZeroMemory(reservedBytes, 2);
        ZeroMemory(zeroBytes, 3);
        ZeroMemory(unusedBytes1, 2);
        ZeroMemory(unusedBytes2, 2);
        ZeroMemory(unusedBytes3, 4);
        ZeroMemory(usedBytes, 4);
        ZeroMemory(unusedBytes4, 3);
        ZeroMemory(unusedBytes5, 3);
        ZeroMemory(volumeSerialID64, 8);
        ZeroMemory(bootCode, 426);
        ZeroMemory(endSignature, 2);
    }
}NTFSDBR, *pNTFSDBR;
#pragma pack()

typedef struct _DataInfo
{
    bool operator==(const _DataInfo &rhs) const
    {
        return ui64BeginScluster == rhs.ui64BeginScluster && ui64UsedSclusters == rhs.ui64UsedSclusters;
    }

    bool operator<(const _DataInfo &rhs) const
    {
        return ui64BeginScluster < rhs.ui64BeginScluster;
    }

    BOOL IsValid()
    {
        return ui64UsedSclusters != 0;
    }

    UINT64 ui64BeginScluster = 0;            // ��ʼ�غ�
    UINT64 ui64UsedSclusters = 0;            // ռ�ö��ٴ�
}DataInfo, *pDataInfo;

typedef struct _FileAttrInfo
{
    bool operator==(const _FileAttrInfo &rhs) const
    {
        return ui64FileUniNum == rhs.ui64FileUniNum;
    }

    BOOL bIsDir = FALSE;            // �ǲ���Ŀ¼
    CString strFilePath;            // ��ǰ�ļ�/�ļ��� ����·������Ŀ¼Ϊ�ļ���Ϊ��.����
    SYSTEMTIME stFileCreateTime;  // �ļ�����ʱ��
    SYSTEMTIME stFileModifyTime;  // �ļ��޸�ʱ��
    UINT64 ui64FileSize = 0;        // ʵ�ʴ�С
    UINT64 ui64FileUniNum = 0;      // �ļ��ο���
    BYTE byNameSpace = 0;           // �ļ����ռ�

    _FileAttrInfo()
    {
        ZeroMemory(&stFileCreateTime, sizeof(stFileCreateTime));
        ZeroMemory(&stFileModifyTime, sizeof(stFileModifyTime));
    }
}FileAttrInfo, *pFileAttrInfo;

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
};

