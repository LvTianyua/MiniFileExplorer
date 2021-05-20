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
    // 设置当前盘符
    virtual BOOL SetCurDriverInfo(const CString& strDriverName) override;
    // 获取当前盘符
    virtual CString GetCurDriverName() override;

    // 获取所有逻辑驱动器名称集合
    virtual std::vector<CString> GetAllLogicDriversNames() override;

    // Ntfs拷贝，通过源文件参考号以及源文件真实大小，拷贝到目标路径（绝对路径）
    virtual BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath) override;

    // Ntfs根据当前目录参考号，获取全部需要显示的子项集合
    virtual BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum, BOOL bForceFresh = FALSE) override;

    // 根据子文件参考号获取父文件参考号
    virtual BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum) override;

    // 根据文件参考号获取文件名
    virtual BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath) override;

    // 设置进度条窗口句柄
    virtual void SetProgressWndHandle(const HWND& hProgressWnd) override;

    // CString转换成QString
    static QString CStringToQString(const CString& strCs);

    // QString转换成CString
    static CString QStringToCString(const QString& strQs);

protected:
    // 根据文件参考号获取文件记录（DeviceIoControl API方式）
    BOOL _GetFileRecordByFileRefNum(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // 根据文件参考号获取文件记录（遍历磁盘全部扇区解析属性方式寻找，文件参考号就是MFT中第N项）
    BOOL _GetFileRecordByFileRefNum2(const UINT64& ui64FileRefNum, PBYTE pBuffer, BOOL bFresh = FALSE);

    // 根据文件记录获取30H首地址，返回False可能是存在20属性
    BOOL _Get30HAttrSPosByFileRecord(const PBYTE pRecordBuffer, UINT& ui30HSpos);

    // 根据20属性读取正确名字空间的30属性首地址和对应的新文件记录buffer
    BOOL _Get30HAttrSPosFrom20HAttr(const PBYTE pRecordBuffer, UINT& ui30HSpos, PBYTE pNewRecordBuffer);

    // 根据20属性读取其中的A0对应的子项集合
    BOOL _GetA0HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // 根据20属性读取其中的90对应的子项集合
    BOOL _Get90HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // 根据30H属性获取文件名
    BOOL _GetFileNameBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, CString& strFileName);

    // 根据文件记录获取父目录参考号
    BOOL _GetParentFileNumByFileRecord(const PBYTE pRecordBuffer, UINT64& ui64ParentFileNum);

    // 根据30H属性获取父目录参考号
    BOOL _GetParentFileNumBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, UINT64& ui64ParentFileNum);

    // 根据文件记录递归获取文件全路径
    BOOL _AutoGetFullPath(const PBYTE pRecordBuffer, CString& strPath);

    // 文件记录中提取的时间转换为SYSTEMTIME
    BOOL _TimeFromRecordToSystemTime(const UINT64& ui64Time, SYSTEMTIME& systemTime);

    // 获取A0属性datarun起始偏移
    BOOL _GetA0HAttrDataRunSPos(const PBYTE pRecordBuffer, unsigned short& usDataRunSPos);

    // 根据datarun起始位置，长度获取datarun数据
    BOOL _GetDataRunList(const PBYTE pRecordBuffer, const UINT& uiDatarunSPos, const UINT& uiDataRunLength, std::vector<DataInfo>& vecDatarunList);

    // 计算非首个datarun的起始簇号相对于上一个datarun的偏移值
    BOOL _CalcNonFirstDataRunDevForLast(const UINT64& ui64LastBeginCluster, UINT64& ui64BeginCluster, const UINT& uiNum);

    // 从datarun数据流中读取一组FileAttrInfo信息
    BOOL _GetOneFileAttrInfoByDataRunBuffer(const PBYTE pDataRunBuffer, const UINT& uiIndexSPos, FileAttrInfo& fileAttrInfo);

    // 读取80对应datarun的filedata（最大读取4m，超过需要循环读取）
    BOOL _GetFileDataByDataRun(const std::vector<DataInfo>& vecDataRunList, PBYTE pFileData);

    // 根据盘符名字获取磁盘句柄
    BOOL _GetDriverHandleByDriverName(const CString& strDriverName, HANDLE& handle);

    // 对获取到的子项进行排序，按照先文件夹再文件的顺序
    void _SortChildInfos(std::vector<FileAttrInfo>& vecChildInfos, UINT& uiDirNum);

    // 判断一个字符是不是字母
    BOOL _IsLetter(const TCHAR& str);

    // 重置当前盘符
    void _ResetCurDriverInfo();

    // 获取dbr信息
    BOOL _GetDBRInfo(NTFSDBR& dbrInfo);
    BOOL _GetDBRInfo(const HANDLE& hDriver, NTFSDBR& dbrInfo);

    // 根据dbr信息判断是不是ntfs盘
    BOOL _IsNTFSDriver(const NTFSDBR& dbrInfo);

    // 根据dbr获取MFT表的起始偏移量
    UINT64 _GetMFTStartPositionByDBR(const NTFSDBR& dbrInfo);

    // 根据首地址(512字节对齐，必须是512倍数)，读取数据长度必须为512倍数，获取指定段数据
    BOOL _GetAnySectionBuffer(const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer);
    BOOL _GetAnySectionBuffer(const HANDLE& hDriver, const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer);

    // 根据文件记录获取文件名
    BOOL _GetFileNameByFileRecord(const PBYTE pRecordBuffer, CString& strFileName);

    // 遍历属性列表寻找指定属性
    BOOL _FindAnyAttrSPosByFileRecord(const PBYTE pRecordBuffer, const UINT& uiAttrType, UINT& uiAttrSPos, UINT& uiAttrLength);

    // 获取A0属性指向的datarun列表
    BOOL _GetA0HAttrDataRunLists(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunLists);

    // 获取90属性指向的datarun列表
    BOOL _Get90HAttrChildAttrInfos(const PBYTE pRecordBuffer, const CString& strParentPath, std::vector<FileAttrInfo>& vecFileAttrLists);

    // 根据datarun列表获取索引项相关的信息
    BOOL _GetChildFileAttrInfoByRunList(const CString& strParentPath, const std::vector<DataInfo>& vecDataRunLists, std::vector<FileAttrInfo>& vecChildAttrInfos);

    // 针对文件解析80属性，根据文件记录获取文件真实数据
    // 返回值：0，失败 1，成功 FileDataBuffer就是文件数据 2，文件数据大于4m，存在是超大文件的可能性，只给出datarun信息，自己循环边读边写
    UINT _GetFileDataByFileRecord(const PBYTE pFileRecordBuffer, PBYTE pFileDataBuffer, std::vector<DataInfo>& vecDataRunList);

    // 针对超过4m的文件，进行分块读写
    BOOL _BigFileBlockReadAndWrite(const std::vector<DataInfo>& vecDataRunList, const CString& strWriteFilePath, const UINT64& ui64FileRealSize);

    // 将缓冲区数据写入文件
    BOOL _WriteFileFromBuffer(const PBYTE pBuffer, const UINT64& ui64WriteLength, const CString& strFilePath, BOOL bTruncate = FALSE);

    // 从文件记录里面读取20属性中的80属性对应的文件记录中的daturun列表
    BOOL _GetDataRunBy80AttrFrom20Attr(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunInfos);

    // 从MFTdatarun中读取指定文件号的文件记录
    BOOL _GetFileBufferByFileNumFrom80DataRun(const UINT64 ui64FileNum, PBYTE pFileRecordBuffer);

    // 初始化磁盘
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
    CString                                         m_strCurDriverName;                // 当前打开的盘符
    HANDLE                                          m_hanCurDriver = NULL;             // 当前盘符句柄
    HWND                                            m_hProgressWnd = NULL;             // 进度条窗口句柄
    HANDLE                                          m_hFile = NULL;                    // 拷贝文件的句柄
    std::map<UINT64, PBYTE>                         m_mapFileNumRecordBuffer;          // 文件参考号对应文件记录集合
    std::vector<DataCompleteInfo>                   m_vecMFTDataCompRunList;           // MFT的datarunlist
    std::map<CString, std::vector<DataCompleteInfo>> m_mapDriverCompInfos;             // 磁盘，datarunlist映射
    std::map<CString, std::map<UINT64, PBYTE>> m_mapDriverFileNumBuffers;              // 磁盘，FileNumBuffer映射
    std::vector<CString>                            m_vecFilterNames;
    BOOL                                            m_bInit = FALSE;
};

