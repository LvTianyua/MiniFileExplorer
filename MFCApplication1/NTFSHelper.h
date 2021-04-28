#pragma once
#include <vector>
#include "SingleInstace.h"

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

    _FileAttrInfo()
    {
        ZeroMemory(&stFileCreateTime, sizeof(stFileCreateTime));
        ZeroMemory(&stFileModifyTime, sizeof(stFileModifyTime));
    }
}FileAttrInfo, *pFileAttrInfo;

typedef struct _DataInfo
{
    BOOL IsValid()
    {
        return ui64BeginScluster != 0 && ui64UsedSclusters != 0;
    }

    UINT64 ui64BeginScluster = 0;            // ��ʼ�غ�
    UINT64 ui64UsedSclusters = 0;            // ռ�ö��ٴ�
}DataInfo, *pDataInfo;

class CNTFSHelper : public CSingleInstace<CNTFSHelper>
{
private:
    ~CNTFSHelper();
public:
    // ���õ�ǰ�̷�
    BOOL SetCurDriverInfo(const CString& strDriverName);
    // ��ȡ��ǰ�̷�
    CString GetCurDriverName();

    // ��ȡ�����߼����������Ƽ���
    std::vector<CString> GetAllLogicDriversNames();

    // �����ļ��ο��Ż�ȡ�ļ���¼��DeviceIoControl API��ʽ��
    BOOL GetFileRecordByFileRefNum(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // �����ļ��ο��Ż�ȡ�ļ���¼����������ȫ�������������Է�ʽѰ�ң��ļ��ο��ž���MFT�е�N�
    BOOL GetFileRecordByFileRefNum2(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // Ntfs������ͨ��Դ�ļ��ο����Լ�Դ�ļ���ʵ��С��������Ŀ��·��������·����
    BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath);

    // Ntfs���ݵ�ǰĿ¼�ο��ţ���ȡȫ����Ҫ��ʾ�������
    BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // �������ļ��ο��Ż�ȡ���ļ��ο���
    BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum);

    // �����ļ��ο��Ż�ȡ�ļ���
    BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath);

    // ���ý��������ھ��
    void SetProgressWndHandle(HWND hProgressWnd);

protected:
    // �����ļ���¼��ȡ30H�׵�ַ������False�����Ǵ���20����
    BOOL _Get30HAttrSPosByFileRecord(const PBYTE pRecordBuffer, UINT& ui30HSpos);

    // ����20���Զ�ȡ��ȷ���ֿռ��30�����׵�ַ�Ͷ�Ӧ�����ļ���¼buffer
    BOOL _Get30HAttrSPosFrom20HAttr(const PBYTE pRecordBuffer, UINT& ui30HSpos, PBYTE pNewRecordBuffer);

    // ����20���Զ�ȡ���е�A0��Ӧ�������
    BOOL _GetA0HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // ����20���Զ�ȡ���е�90��Ӧ�������
    BOOL _Get90HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // ����30H���Ի�ȡ�ļ���
    BOOL _GetFileNameBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, CString& strFileName);

    // �����ļ���¼��ȡ��Ŀ¼�ο���
    BOOL _GetParentFileNumByFileRecord(const PBYTE pRecordBuffer, UINT64& ui64ParentFileNum);

    // ����30H���Ի�ȡ��Ŀ¼�ο���
    BOOL _GetParentFileNumBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, UINT64& ui64ParentFileNum);

    // �����ļ���¼�ݹ��ȡ�ļ�ȫ·��
    BOOL _AutoGetFullPath(const PBYTE pRecordBuffer, CString& strPath);

    // �ļ���¼����ȡ��ʱ��ת��ΪSYSTEMTIME
    BOOL _TimeFromRecordToSystemTime(const UINT64& ui64Time, SYSTEMTIME& systemTime);

    // ��ȡA0����datarun��ʼƫ��
    BOOL _GetA0HAttrDataRunSPos(const PBYTE pRecordBuffer, unsigned short& usDataRunSPos);

    // ����datarun��ʼλ�ã����Ȼ�ȡdatarun����
    BOOL _GetDataRunList(const PBYTE pRecordBuffer, const UINT& uiDatarunSPos, const UINT& uiDataRunLength, std::vector<DataInfo>& vecDatarunList);

    // ������׸�datarun����ʼ�غ��������һ��datarun��ƫ��ֵ
    BOOL _CalcNonFirstDataRunDevForLast(const UINT64& ui64LastBeginCluster, UINT64& ui64BeginCluster, const UINT& uiNum);

    // ��datarun�������ж�ȡһ��FileAttrInfo��Ϣ
    BOOL _GetOneFileAttrInfoByDataRunBuffer(const PBYTE pDataRunBuffer, const UINT& uiIndexSPos, FileAttrInfo& fileAttrInfo);

    // ��ȡ80��Ӧdatarun��filedata������ȡ4m��������Ҫѭ����ȡ��
    BOOL _GetFileDataByDataRun(const std::vector<DataInfo>& vecDataRunList, PBYTE pFileData);

    // �����̷����ֻ�ȡ���̾��
    BOOL _GetDriverHandleByDriverName(const CString& strDriverName, HANDLE& handle);

    // �Ի�ȡ��������������򣬰������ļ������ļ���˳��
    void _SortChildInfos(std::vector<FileAttrInfo>& vecChildInfos, UINT& uiDirNum);

    // �ж�һ���ַ��ǲ�����ĸ
    BOOL _IsLetter(const TCHAR& str);

    // ���õ�ǰ�̷�
    void _ResetCurDriverInfo();

    // ��ȡdbr��Ϣ
    BOOL _GetDBRInfo(NTFSDBR& dbrInfo);
    BOOL _GetDBRInfo(const HANDLE& hDriver, NTFSDBR& dbrInfo);

    // ����dbr��Ϣ�ж��ǲ���ntfs��
    BOOL _IsNTFSDriver(const NTFSDBR& dbrInfo);

    // ����dbr��ȡMFT�����ʼƫ����
    UINT64 _GetMFTStartPositionByDBR(const NTFSDBR& dbrInfo);

    // �����׵�ַ(512�ֽڶ��룬������512����)����ȡ���ݳ��ȱ���Ϊ512��������ȡָ��������
    BOOL _GetAnySectionBuffer(const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer);
    BOOL _GetAnySectionBuffer(const HANDLE& hDriver, const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer);

    // �����ļ���¼��ȡ�ļ���
    BOOL _GetFileNameByFileRecord(const PBYTE pRecordBuffer, CString& strFileName);

    // ���������б�Ѱ��ָ������
    BOOL _FindAnyAttrSPosByFileRecord(const PBYTE pRecordBuffer, const UINT& uiAttrType, UINT& uiAttrSPos, UINT& uiAttrLength);

    // ��ȡA0����ָ���datarun�б�
    BOOL _GetA0HAttrDataRunLists(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunLists);

    // ��ȡ90����ָ���datarun�б�
    BOOL _Get90HAttrChildAttrInfos(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecFileAttrLists);

    // ����datarun�б��ȡ��������ص���Ϣ
    BOOL _GetChildFileAttrInfoByRunList(const std::vector<DataInfo>& vecDataRunLists, std::vector<FileAttrInfo>& vecChildAttrInfos);

    // ����ļ�����80���ԣ������ļ���¼��ȡ�ļ���ʵ����
    // ����ֵ��0��ʧ�� 1���ɹ� FileDataBuffer�����ļ����� 2���ļ����ݴ���4m�������ǳ����ļ��Ŀ����ԣ�ֻ����datarun��Ϣ���Լ�ѭ���߶���д
    UINT _GetFileDataByFileRecord(const PBYTE pFileRecordBuffer, PBYTE pFileDataBuffer, std::vector<DataInfo>& vecDataRunList);

    // ��Գ���4m���ļ������зֿ��д
    BOOL _BigFileBlockReadAndWrite(const std::vector<DataInfo>& vecDataRunList, const CString& strWriteFilePath, const UINT64& ui64FileRealSize);

    // ������������д���ļ�
    BOOL _WriteFileFromBuffer(const PBYTE pBuffer, const UINT64& ui64WriteLength, const CString& strFilePath, BOOL bTruncate = FALSE);

private:
    CString                                         m_strCurDriverName;         // ��ǰ�򿪵��̷������ص������б�ʱӦ�ÿ�
    HANDLE                                          m_hanCurDriver = NULL;             // ��ǰ�̷����
    HWND                                            m_hProgressWnd = NULL;
    HANDLE                                          m_hFile = NULL;             // �����ļ��ľ��
};

