#pragma once
#include <vector>
#include "SingleInstace.h"

#define MSG_UPDATE_PROGRESS  WM_USER + 0x01

#pragma pack(1)
typedef struct _NTFSDBR {
    BYTE JMP[3];	//跳转指令
    BYTE FsID[8];	//文件系统ID
    unsigned short int bytePerSector = 0;	//每扇区字节数
    BYTE secPerCluster = 0;		//每簇扇区数
    BYTE reservedBytes[2];	//2个保留字节
    BYTE zeroBytes[3];	//三个0字节
    BYTE unusedBytes1[2];	//2个未用字节
    BYTE mediaType = 0;//媒体类型
    BYTE unusedBytes2[2];	//2个未用字节
    unsigned short int secPerTrack = 0;	//每磁道扇区数
    unsigned short int Heads = 0;	//磁头数
    unsigned int hideSectors = 0;	//隐藏扇区数
    BYTE unusedBytes3[4];	//4个未用字节
    BYTE usedBytes[4];	//4个固定字节
    unsigned __int64 totalSectors = 0;	//总扇区数
    unsigned __int64 MFT = 0;	//MFT起始簇号
    unsigned __int64 MFTMirror = 0;	//MFTMirror文件起始簇号
    char fileRecord = 0;	//文件记录
    BYTE unusedBytes4[3];	//3个未用字节
    char indexSize = 0;	//索引缓冲区大小
    BYTE unusedBytes5[3];	//未用字节
    BYTE volumeSerialID64[8];	//卷序列号
    unsigned int checkSum = 0;	//校验和
    BYTE bootCode[426];	//引导代码
    BYTE endSignature[2];	//结束标志

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

    BOOL bIsDir = FALSE;            // 是不是目录
    CString strFilePath;            // 当前文件/文件夹 绝对路径（根目录为文件名为“.”）
    SYSTEMTIME stFileCreateTime;  // 文件创建时间
    SYSTEMTIME stFileModifyTime;  // 文件修改时间
    UINT64 ui64FileSize = 0;        // 实际大小
    UINT64 ui64FileUniNum = 0;      // 文件参考号

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

    UINT64 ui64BeginScluster = 0;            // 起始簇号
    UINT64 ui64UsedSclusters = 0;            // 占用多少簇
}DataInfo, *pDataInfo;

class CNTFSHelper : public CSingleInstace<CNTFSHelper>
{
private:
    ~CNTFSHelper();
public:
    // 设置当前盘符
    BOOL SetCurDriverInfo(const CString& strDriverName);
    // 获取当前盘符
    CString GetCurDriverName();

    // 获取所有逻辑驱动器名称集合
    std::vector<CString> GetAllLogicDriversNames();

    // 根据文件参考号获取文件记录（DeviceIoControl API方式）
    BOOL GetFileRecordByFileRefNum(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // 根据文件参考号获取文件记录（遍历磁盘全部扇区解析属性方式寻找，文件参考号就是MFT中第N项）
    BOOL GetFileRecordByFileRefNum2(const UINT64& ui64FileRefNum, PBYTE pBuffer);

    // Ntfs拷贝，通过源文件参考号以及源文件真实大小，拷贝到目标路径（绝对路径）
    BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath);

    // Ntfs根据当前目录参考号，获取全部需要显示的子项集合
    BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // 根据子文件参考号获取父文件参考号
    BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum);

    // 根据文件参考号获取文件名
    BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath);

    // 设置进度条窗口句柄
    void SetProgressWndHandle(HWND hProgressWnd);

protected:
    // 根据文件记录获取30H首地址，返回False可能是存在20属性
    BOOL _Get30HAttrSPosByFileRecord(const PBYTE pRecordBuffer, UINT& ui30HSpos);

    // 根据20属性读取正确名字空间的30属性首地址和对应的新文件记录buffer
    BOOL _Get30HAttrSPosFrom20HAttr(const PBYTE pRecordBuffer, UINT& ui30HSpos, PBYTE pNewRecordBuffer);

    // 根据20属性读取其中的A0对应的子项集合
    BOOL _GetA0HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

    // 根据20属性读取其中的90对应的子项集合
    BOOL _Get90HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum);

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
    BOOL _Get90HAttrChildAttrInfos(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecFileAttrLists);

    // 根据datarun列表获取索引项相关的信息
    BOOL _GetChildFileAttrInfoByRunList(const std::vector<DataInfo>& vecDataRunLists, std::vector<FileAttrInfo>& vecChildAttrInfos);

    // 针对文件解析80属性，根据文件记录获取文件真实数据
    // 返回值：0，失败 1，成功 FileDataBuffer就是文件数据 2，文件数据大于4m，存在是超大文件的可能性，只给出datarun信息，自己循环边读边写
    UINT _GetFileDataByFileRecord(const PBYTE pFileRecordBuffer, PBYTE pFileDataBuffer, std::vector<DataInfo>& vecDataRunList);

    // 针对超过4m的文件，进行分块读写
    BOOL _BigFileBlockReadAndWrite(const std::vector<DataInfo>& vecDataRunList, const CString& strWriteFilePath, const UINT64& ui64FileRealSize);

    // 将缓冲区数据写入文件
    BOOL _WriteFileFromBuffer(const PBYTE pBuffer, const UINT64& ui64WriteLength, const CString& strFilePath, BOOL bTruncate = FALSE);

private:
    CString                                         m_strCurDriverName;         // 当前打开的盘符，返回到磁盘列表时应置空
    HANDLE                                          m_hanCurDriver = NULL;             // 当前盘符句柄
    HWND                                            m_hProgressWnd = NULL;
    HANDLE                                          m_hFile = NULL;             // 拷贝文件的句柄
};

