#pragma once
#include "SingleInstace.h"
#include "INTFSHelper.h"

typedef struct _DataCompleteInfo
{
    DataInfo dataInfo;
    UINT uiFirstFileNum = 0;
    UINT uiFinalFileNum = 0;
} DataCompleteInfo, *PDataCompleteInfo;


class CNTFSHelper : public CSingleInstace<CNTFSHelper>, public INTFSHelper
{
private:
    ~CNTFSHelper();
public:
    // ���õ�ǰ�̷�
    virtual BOOL SetCurDriverInfo(const CString& strDriverName) override;
    // ��ȡ��ǰ�̷�
    virtual CString GetCurDriverName() override;

    // ��ȡ�����߼����������Ƽ���
    virtual std::vector<CString> GetAllLogicDriversNames() override;

    // Ntfs������ͨ��Դ�ļ��ο����Լ�Դ�ļ���ʵ��С��������Ŀ��·��������·����
    virtual BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath) override;

    // Ntfs���ݵ�ǰĿ¼�ο��ţ���ȡȫ����Ҫ��ʾ�������
    virtual BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum, BOOL bForceFresh = FALSE) override;

    // �������ļ��ο��Ż�ȡ���ļ��ο���
    virtual BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum) override;

    // �����ļ��ο��Ż�ȡ�ļ���
    virtual BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath) override;

    // ���ý��������ھ��
    virtual void SetProgressWndHandle(const HWND& hProgressWnd) override;

    // CStringת����QString
    static QString CStringToQString(const CString& strCs);

    // QStringת����CString
    static CString QStringToCString(const QString& strQs);

protected:
    // �����ļ��ο��Ż�ȡ�ļ���¼��DeviceIoControl API��ʽ��
    BOOL _GetFileRecordByFileRefNum(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // �����ļ��ο��Ż�ȡ�ļ���¼����������ȫ�������������Է�ʽѰ�ң��ļ��ο��ž���MFT�е�N�
    BOOL _GetFileRecordByFileRefNum2(const UINT64& ui64FileRefNum, PBYTE pBuffer, BOOL bFresh = FALSE);

    // �����ļ���¼��ȡ30H�׵�ַ������False�����Ǵ���20����
    BOOL _Get30HAttrSPosByFileRecord(const PBYTE pRecordBuffer, UINT& ui30HSpos);

    // ����20���Զ�ȡ��ȷ���ֿռ��30�����׵�ַ�Ͷ�Ӧ�����ļ���¼buffer
    BOOL _Get30HAttrSPosFrom20HAttr(const PBYTE pRecordBuffer, UINT& ui30HSpos, PBYTE pNewRecordBuffer);

    // ����20���Զ�ȡ���е�A0��Ӧ�������
    BOOL _GetA0HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // ����20���Զ�ȡ���е�90��Ӧ�������
    BOOL _Get90HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

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
    BOOL _Get90HAttrChildAttrInfos(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecFileAttrLists);

    // ����datarun�б��ȡ��������ص���Ϣ
    BOOL _GetChildFileAttrInfoByRunList(const CString& strParentPath, const std::vector<DataInfo>& vecDataRunLists, std::vector<FileAttrInfo>& vecChildAttrInfos);

    // ����ļ�����80���ԣ������ļ���¼��ȡ�ļ���ʵ����
    // ����ֵ��0��ʧ�� 1���ɹ� FileDataBuffer�����ļ����� 2���ļ����ݴ���4m�������ǳ����ļ��Ŀ����ԣ�ֻ����datarun��Ϣ���Լ�ѭ���߶���д
    UINT _GetFileDataByFileRecord(const PBYTE pFileRecordBuffer, PBYTE pFileDataBuffer, std::vector<DataInfo>& vecDataRunList);

    // ��Գ���4m���ļ������зֿ��д
    BOOL _BigFileBlockReadAndWrite(const std::vector<DataInfo>& vecDataRunList, const CString& strWriteFilePath, const UINT64& ui64FileRealSize);

    // ������������д���ļ�
    BOOL _WriteFileFromBuffer(const PBYTE pBuffer, const UINT64& ui64WriteLength, const CString& strFilePath, BOOL bTruncate = FALSE);

    // ���ļ���¼�����ȡ20�����е�80���Զ�Ӧ���ļ���¼�е�daturun�б�
    BOOL _GetDataRunBy80AttrFrom20Attr(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunInfos);

    // ��MFTdatarun�ж�ȡָ���ļ��ŵ��ļ���¼
    BOOL _GetFileBufferByFileNumFrom80DataRun(const UINT64 ui64FileNum, PBYTE pFileRecordBuffer);

    // ��ʼ������
    BOOL _InitCurDriver();

    void _ClearDataMapBuffer(const CString& strLastDriverName);

    template<typename T>
    void _SafeDelete(T* ptr) 
    {
        if (ptr)
        {
            delete ptr;
            ptr = nullptr;
        }
    }

    template<typename T>
    void _SafeDeleteBuffer(T* ptr)
    {
        if (ptr)
        {
            delete[] ptr;
            ptr = nullptr;
        }
    }

private:
    CString                                         m_strCurDriverName;                // ��ǰ�򿪵��̷�
    HANDLE                                          m_hanCurDriver = NULL;             // ��ǰ�̷����
    HWND                                            m_hProgressWnd = NULL;             // ���������ھ��
    HANDLE                                          m_hFile = NULL;                    // �����ļ��ľ��
    std::map<UINT64, PBYTE>                         m_mapFileNumRecordBuffer;          // �ļ��ο��Ŷ�Ӧ�ļ���¼����
    std::vector<DataCompleteInfo>                   m_vecMFTDataCompRunList;           // MFT��datarunlist
    std::map<CString, std::vector<DataCompleteInfo>> m_mapDriverCompInfos;             // ���̣�datarunlistӳ��
    std::map<CString, std::map<UINT64, PBYTE>> m_mapDriverFileNumBuffers;              // ���̣�FileNumBufferӳ��
    std::vector<CString>                            m_vecFilterNames;
    BOOL                                            m_bInit = FALSE;
};

