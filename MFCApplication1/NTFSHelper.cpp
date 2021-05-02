#include "stdafx.h"
#include "NTFSHelper.h"
#include <memory>
#include <winioctl.h>
#include <algorithm>
#include <xutility>
#include <string>
#include <map>

#define ONE_SECTOR_SIZE             512         //单扇区大小（字节）
#define ONE_CLUSTER_SIZE            8 * 512     //单簇大小（字节）
#define ONE_FILE_RECORD_SIZE        1024        //单文件记录大小（字节）
#define ONE_FILE_BLOCK_SIZE         4 * 1024 * 1024 //单文件分块大小（字节）

CNTFSHelper::~CNTFSHelper()
{
    if (m_hanCurDriver)
    {
        CloseHandle(m_hanCurDriver);
        m_hanCurDriver = NULL;
    }
}

std::vector<CString> CNTFSHelper::GetAllLogicDriversNames()
{
    TCHAR szBuf[MAX_PATH];
    memset(szBuf, 0, MAX_PATH);
    DWORD dwLen = GetLogicalDriveStrings(sizeof(szBuf) / sizeof(TCHAR), szBuf);

    std::vector<CString> vecDriversNames;
    for (TCHAR * s = szBuf; *s; s += _tcslen(s) + 1)
    {
        if (_IsLetter(*s))
        {
            vecDriversNames.push_back(CString(*s));
        }
    }

    for (auto it = vecDriversNames.begin(); it != vecDriversNames.end();)
    {
        HANDLE hDriver = NULL;
        NTFSDBR dbrInfo;
        if (_GetDriverHandleByDriverName(*it, hDriver) && _GetDBRInfo(hDriver, dbrInfo))
        {
            if (!_IsNTFSDriver(dbrInfo))
            {
                it = vecDriversNames.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    return vecDriversNames;
}

BOOL CNTFSHelper::_GetDBRInfo(NTFSDBR& dbrInfo)
{
    BYTE buffer[sizeof(NTFSDBR) + 1];
    memset(buffer, 0, sizeof(NTFSDBR) + 1);
    if (_GetAnySectionBuffer(0, sizeof(NTFSDBR), buffer))
    {
        memcpy(&dbrInfo, &buffer, sizeof(NTFSDBR));
        return TRUE;
    }
    return FALSE;
}

BOOL CNTFSHelper::_GetDBRInfo(const HANDLE& hDriver, NTFSDBR& dbrInfo)
{
    BYTE buffer[sizeof(NTFSDBR) + 1];
    memset(buffer, 0, sizeof(NTFSDBR) + 1);
    if (_GetAnySectionBuffer(hDriver, 0, sizeof(NTFSDBR), buffer))
    {
        memcpy(&dbrInfo, &buffer, sizeof(NTFSDBR));
        return TRUE;
    }
    return FALSE;
}

BOOL CNTFSHelper::_IsNTFSDriver(const NTFSDBR& dbrInfo)
{
    CString strFileId = CString(dbrInfo.FsID);
    return strFileId.Find(L"NTFS") == 0;
}

UINT64 CNTFSHelper::_GetMFTStartPositionByDBR(const NTFSDBR& dbrInfo)
{
    return dbrInfo.MFT*dbrInfo.secPerCluster*dbrInfo.bytePerSector;
}

BOOL CNTFSHelper::_GetAnySectionBuffer(const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer)
{
    //校验参数
    if (ui64SPos % ONE_SECTOR_SIZE != 0 || ui64Len % ONE_SECTOR_SIZE != 0)
    {
        return FALSE;
    }

    DWORD dwSize;
    LARGE_INTEGER li = { 0 };
    li.QuadPart = ui64SPos;
    if (!SetFilePointerEx(m_hanCurDriver, li, &li, FILE_BEGIN))
    {
        return FALSE;
    }
    if (!ReadFile(m_hanCurDriver, pBuffer, ui64Len, &dwSize, NULL) || dwSize != ui64Len)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetAnySectionBuffer(const HANDLE& hDriver, const UINT64& ui64SPos, const UINT64& ui64Len, PBYTE pBuffer)
{
    //校验参数
    if (ui64SPos % ONE_SECTOR_SIZE != 0 || ui64Len % ONE_SECTOR_SIZE != 0)
    {
        return FALSE;
    }

    DWORD dwSize;
    LARGE_INTEGER li = { 0 };
    li.QuadPart = ui64SPos;
    if (!SetFilePointerEx(hDriver, li, &li, FILE_BEGIN))
    {
        return FALSE;
    }
    if (!ReadFile(hDriver, pBuffer, ui64Len, &dwSize, NULL) || dwSize != ui64Len)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetFileRecordByFileRefNum(const UINT64& ui64FileRefNum, PBYTE pBuffer)
{
    DWORD returnByte = 0;
    NTFS_VOLUME_DATA_BUFFER ntfsVolumeDataBuffer;

    //获取文件记录大小
    if (!DeviceIoControl(m_hanCurDriver, FSCTL_GET_NTFS_VOLUME_DATA, NULL,
        0, &ntfsVolumeDataBuffer, sizeof(NTFS_VOLUME_DATA_BUFFER), &returnByte, NULL))
    {
        return FALSE;
    }

    // 设置输入参数：文件参考号
    NTFS_FILE_RECORD_INPUT_BUFFER ntfsFileRecordInputBuffer;
    ntfsFileRecordInputBuffer.FileReferenceNumber.QuadPart = ui64FileRefNum;

    // 这里将outbuffer分配在堆上面，主要是因为NTFS_FILE_RECORD_OUTPUT_BUFFER结构体带个可变长度的数组，在栈上分配会导致数组长度改变的时候，数组里面的内存
    // 会覆盖其他栈上有效区域的内存，导致很多局部变量等失效，产生无法预知的问题！！！
    PNTFS_FILE_RECORD_OUTPUT_BUFFER pNtfsFileRecordOutputBuffer = new NTFS_FILE_RECORD_OUTPUT_BUFFER[sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1];
    ZeroMemory(pNtfsFileRecordOutputBuffer, sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1);
    if (!DeviceIoControl(m_hanCurDriver,
        FSCTL_GET_NTFS_FILE_RECORD,
        &ntfsFileRecordInputBuffer,
        sizeof(NTFS_FILE_RECORD_INPUT_BUFFER),
        pNtfsFileRecordOutputBuffer,
        sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1,//MSDN中有说明
        &returnByte,
        NULL))
    {
        if (pNtfsFileRecordOutputBuffer)
        {
            delete[] pNtfsFileRecordOutputBuffer;
            pNtfsFileRecordOutputBuffer = nullptr;
        }
        return FALSE;
    }
    else
    {
        if (pNtfsFileRecordOutputBuffer)
        {
            memcpy(pBuffer, &(pNtfsFileRecordOutputBuffer->FileRecordBuffer), pNtfsFileRecordOutputBuffer->FileRecordLength);
            delete[] pNtfsFileRecordOutputBuffer;
            pNtfsFileRecordOutputBuffer = nullptr;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetFileRecordByFileRefNum2(const UINT64& ui64FileRefNum, PBYTE pBuffer)
{
    // 1.拿到DBR
    NTFSDBR dbrInfo;
    _GetDBRInfo(dbrInfo);

    // 2.根据dbr获取MFT首地址
    UINT64 ui64MFTSPos = _GetMFTStartPositionByDBR(dbrInfo);

    // 3.开始遍历寻找第ui64FileRefNum项
    UINT64 ui64Num = 0;
    while (ui64Num <= ui64MFTSPos)
    {
        // 读磁盘判断当前遍历到的首地址是不是0，要是0就偏移到下个1024（一个文件记录长，直到遍历到需要的位置）
        BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
        ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
        if (_GetAnySectionBuffer(ui64MFTSPos, ONE_FILE_RECORD_SIZE, buffer))
        {
            if (buffer[0] != 0)
            {
                if (ui64Num == ui64FileRefNum)
                {
                    memcpy(pBuffer, &buffer, ONE_FILE_RECORD_SIZE);
                    return TRUE;
                }
                ++ui64Num;
            }
            ui64MFTSPos += ONE_FILE_RECORD_SIZE;
        }
    }
    return FALSE;
}

BOOL CNTFSHelper::_Get30HAttrSPosByFileRecord(const PBYTE pRecordBuffer, UINT& ui30HSpos)
{
    // 1.分析文件记录头部 找到第一个属性位置（记录第一个属性偏移首地址在文件记录首地址偏移0x14位置，占用2字节）
    memcpy(&ui30HSpos, &pRecordBuffer[0x14], 2);

    // 2.开始顺次寻找属性，目标30H属性
    // 分析第一个属性类型，10H属性，常驻，此处用不到直接跳到下一个属性
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);
        ui30HSpos += uiFirstAttrLength;
    }

    // 分析第二个属性，20H属性非常驻，有的有，有的没有，先解析类型，如果是20H需要特殊处理，如果是30H就解析
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);
        ui30HSpos += uiFirstAttrLength;
        memcpy(&uiSecondAttrType, &pRecordBuffer[ui30HSpos], 4);
        if (uiSecondAttrType == 0x30)
        {
            UINT uiSPos = ui30HSpos;
            while (uiSecondAttrType == 0x30)
            {
                BYTE byNameSpace = 4;
                memcpy(&byNameSpace, &pRecordBuffer[uiSPos + 0x18 + 0x41], 1);
                if (byNameSpace == 0 || byNameSpace == 1)
                {
                    ui30HSpos = uiSPos;
                    return TRUE;
                }
                memcpy(&uiFirstAttrLength, &pRecordBuffer[uiSPos + 0x04], 4);
                uiSPos += uiFirstAttrLength;
                uiSecondAttrType = 0;
                memcpy(&uiSecondAttrType, &pRecordBuffer[uiSPos], 4);
            }
            return TRUE;
        }
    }
    else if (uiSecondAttrType == 0x30)
    {
        UINT uiSPos = ui30HSpos;
        while (uiSecondAttrType == 0x30)
        {
            BYTE byNameSpace = 4;
            memcpy(&byNameSpace, &pRecordBuffer[uiSPos + 0x18 + 0x41], 1);
            if (byNameSpace == 0 || byNameSpace == 1)
            {
                ui30HSpos = uiSPos;
                return TRUE;
            }
            memcpy(&uiFirstAttrLength, &pRecordBuffer[uiSPos + 0x04], 4);
            uiSPos += uiFirstAttrLength;
            uiSecondAttrType = 0;
            memcpy(&uiSecondAttrType, &pRecordBuffer[uiSPos], 4);
        }
        return TRUE;
    }

    return FALSE;
}

BOOL CNTFSHelper::_Get30HAttrSPosFrom20HAttr(const PBYTE pRecordBuffer, UINT& ui30HSpos, PBYTE pNewRecordBuffer)
{
    // 1.分析文件记录头部 找到第一个属性位置（记录第一个属性偏移首地址在文件记录首地址偏移0x14位置，占用2字节）
    memcpy(&ui30HSpos, &pRecordBuffer[0x14], 2);

    // 2.开始顺次寻找属性，目标30H属性
    // 分析第一个属性类型，10H属性，常驻，此处用不到直接跳到下一个属性
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);
        ui30HSpos += uiFirstAttrLength;
    }

    // 分析第二个属性，如果不是20属性，返回false，从20提取正确名字空间30和新的文件记录buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);

        // 遍历20属性列表，寻找30属性，先判断参考号是不是和本文件记录参考号一致，一致不管
        UINT uiSpos = ui30HSpos + 0x18;
        UINT uiLength = uiFirstAttrLength + ui30HSpos;
        while (uiSpos < uiLength)
        {
            memcpy(&uiSecondAttrType, &pRecordBuffer[uiSpos], 4);
            if (uiSecondAttrType != 0x30)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[uiSpos + 0x04], 2);
                uiSpos += uiFirstAttrLength;
                continue;
            }

            UINT64 ui64FileNum = 0;
            memcpy(&ui64FileNum, &pRecordBuffer[uiSpos + 0x10], 6);

            UINT64 ui64CurFileNum = 0;
            UINT ui64CurFileNumXP = 0;
            memcpy(&ui64CurFileNum, &pRecordBuffer[0x20], 6);
            memcpy(&ui64CurFileNumXP, &pRecordBuffer[0x2C], 4);

            if (ui64FileNum == ui64CurFileNum || ui64FileNum == ui64CurFileNumXP)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[uiSpos + 0x04], 2);
                uiSpos += uiFirstAttrLength;
                continue;
            }

            if (_GetFileRecordByFileRefNum(ui64FileNum, pNewRecordBuffer))
            {
                ui30HSpos = 0;
                return _Get30HAttrSPosByFileRecord(pNewRecordBuffer, ui30HSpos);
            }

            memcpy(&uiFirstAttrLength, &pRecordBuffer[uiSpos + 0x04], 2);
            uiSpos += uiFirstAttrLength;
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetA0HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum)
{
    // 1.分析文件记录头部 找到第一个属性位置（记录第一个属性偏移首地址在文件记录首地址偏移0x14位置，占用2字节）
    UINT uiA0HSPos = 0;
    memcpy(&uiA0HSPos, &pRecordBuffer[0x14], 2);

    // 2.开始顺次寻找属性，目标A0H属性
    // 分析第一个属性类型，10H属性，常驻，此处用不到直接跳到下一个属性
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[uiA0HSPos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 4);
        uiA0HSPos += uiFirstAttrLength;
    }

    // 分析第二个属性，如果不是20属性，返回false，从20提取A0对应的新的文件记录buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[uiA0HSPos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 4);

        // 遍历20里面的属性列表找A0，找不到返回False
        UINT uiLength = uiA0HSPos + uiFirstAttrLength;
        uiA0HSPos += 0x18;
        while (uiA0HSPos < uiLength)
        {
            memcpy(&uiSecondAttrType, &pRecordBuffer[uiA0HSPos], 4);
            if (uiSecondAttrType != 0xA0)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 2);
                uiA0HSPos += uiFirstAttrLength;
                continue;
            }

            UINT64 ui64FileNum = 0;
            memcpy(&ui64FileNum, &pRecordBuffer[uiA0HSPos + 0x10], 6);

            UINT64 ui64CurFileNum = 0;
            UINT ui64CurFileNumXP = 0;
            memcpy(&ui64CurFileNum, &pRecordBuffer[0x20], 6);
            memcpy(&ui64CurFileNumXP, &pRecordBuffer[0x2C], 4);

            if (ui64FileNum == ui64CurFileNum || ui64FileNum == ui64CurFileNumXP)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 2);
                uiA0HSPos += uiFirstAttrLength;
                continue;
            }

            BYTE newRecordBuffer[ONE_FILE_RECORD_SIZE + 1] = { 0 };
            if (_GetFileRecordByFileRefNum(ui64FileNum, newRecordBuffer))
            {
                // 遍历datarun，拿到全部簇流的起始位置和占用长度（单位：簇）
                std::vector<DataInfo> vecDataRunLists;
                if (CNTFSHelper::GetInstance()->_GetA0HAttrDataRunLists(newRecordBuffer, vecDataRunLists))
                {
                    // 从datarun获取所有子项
                    if (CNTFSHelper::GetInstance()->_GetChildFileAttrInfoByRunList(vecDataRunLists, vecChildAttrInfos))
                    {
                        //_SortChildInfos(vecChildAttrInfos, uiDirNum);
                        return TRUE;
                    }
                }
            }

            break;
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_Get90HAttrChildListFrom20HAttr(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum)
{
    // 1.分析文件记录头部 找到第一个属性位置（记录第一个属性偏移首地址在文件记录首地址偏移0x14位置，占用2字节）
    UINT ui90HSPos = 0;
    memcpy(&ui90HSPos, &pRecordBuffer[0x14], 2);

    // 2.开始顺次寻找属性，目标A0H属性
    // 分析第一个属性类型，10H属性，常驻，此处用不到直接跳到下一个属性
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui90HSPos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 4);
        ui90HSPos += uiFirstAttrLength;
    }

    // 分析第二个属性，如果不是20属性，返回false，从20提取A0对应的新的文件记录buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[ui90HSPos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 4);

        // 遍历20里面的属性列表找A0，找不到返回False
        UINT uiLength = ui90HSPos + uiFirstAttrLength;
        ui90HSPos += 0x18;
        while (ui90HSPos < uiLength)
        {
            memcpy(&uiSecondAttrType, &pRecordBuffer[ui90HSPos], 4);
            if (uiSecondAttrType != 0x90)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 2);
                ui90HSPos += uiFirstAttrLength;
                continue;
            }

            UINT64 ui64FileNum = 0;
            memcpy(&ui64FileNum, &pRecordBuffer[ui90HSPos + 0x10], 6);

            UINT64 ui64CurFileNum = 0;
            UINT ui64CurFileNumXP = 0;
            memcpy(&ui64CurFileNum, &pRecordBuffer[0x20], 6);
            memcpy(&ui64CurFileNumXP, &pRecordBuffer[0x2C], 4);

            if (ui64FileNum == ui64CurFileNum || ui64FileNum == ui64CurFileNumXP)
            {
                memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 2);
                ui90HSPos += uiFirstAttrLength;
                continue;
            }

            BYTE newRecordBuffer[ONE_FILE_RECORD_SIZE + 1] = { 0 };
            if (_GetFileRecordByFileRefNum(ui64FileNum, newRecordBuffer))
            {
                if (_Get90HAttrChildAttrInfos(newRecordBuffer, vecChildAttrInfos))
                {
                    //_SortChildInfos(vecChildAttrInfos, uiDirNum);
                    return TRUE;
                }
            }

            break;
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_FindAnyAttrSPosByFileRecord(const PBYTE pRecordBuffer, const UINT& uiAttrType, UINT& uiAttrSPos, UINT& uiAttrLength)
{
    // 1.分析文件记录头部 找到第一个属性位置（记录第一个属性偏移首地址在文件记录首地址偏移0x14位置，占用2字节）
    memcpy(&uiAttrSPos, &pRecordBuffer[0x14], 2);

    while (true)
    {
        // 如果uiAttrSPos已经超过一个文件记录，说明没找到，直接返回
        if (uiAttrSPos >= ONE_FILE_RECORD_SIZE)
        {
            return FALSE;
        }

        // 开始顺次寻找属性，目标uiAttrType属性
        // 判断类型，uiAttrType返回，其他类型偏移到下个类型起始位置继续循环
        UINT uiAttrCurType = 0;
        memcpy(&uiAttrCurType, &pRecordBuffer[uiAttrSPos], 4);
        memcpy(&uiAttrLength, &pRecordBuffer[uiAttrSPos + 0x04], 4);
        if (uiAttrLength == 0)
        {
            return FALSE;
        }

        if (uiAttrType == uiAttrCurType)
        {
            return TRUE;
        }
        uiAttrSPos += uiAttrLength;
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetA0HAttrDataRunSPos(const PBYTE pRecordBuffer, unsigned short& usDataRunSPos)
{
    // 先获取A0属性起始偏移
    UINT uiA0HSPos = 0;
    UINT uiA0HLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0xA0, uiA0HSPos, uiA0HLength))
    {
        // 数据流起始地址位于0x20偏移
        memcpy(&usDataRunSPos, &pRecordBuffer[uiA0HSPos + 0x20], 2);
        usDataRunSPos += uiA0HSPos;
        return TRUE;
    }
    return FALSE;
}

bool DataInfoCompare(DataInfo dataInfoA, DataInfo dataInfoB)
{
    return dataInfoA.ui64BeginScluster < dataInfoB.ui64BeginScluster;
}

BOOL CNTFSHelper::_GetDataRunList(const PBYTE pRecordBuffer, const UINT& uiDatarunSPos, const UINT& uiDataRunLength, std::vector<DataInfo>& vecDatarunList)
{
    // 从文件记录中截取datarun数据
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    memcpy(&buffer, &pRecordBuffer[uiDatarunSPos], uiDataRunLength);

    std::map<DataInfo, DataInfo> mapXiShuData;
    // 循环遍历datarunlist获取信息
    for (UINT i = 0; i < uiDataRunLength;)
    {
        if (buffer[i] == 0)
        {
            // 结束返回
            sort(vecDatarunList.begin(), vecDatarunList.end(), DataInfoCompare);
            for (auto info : mapXiShuData)
            {
                if (info.second.ui64UsedSclusters == 0)
                {
                    vecDatarunList.insert(vecDatarunList.begin(), info.second);
                }
                else
                {
                    vecDatarunList.insert(std::find(vecDatarunList.begin(), vecDatarunList.end(), info.second), info.first);
                }
            }
            return TRUE;
        }
        // 先解析起始簇号和大小占用分别使用了多少字节
        const unsigned char ucSPosNum = buffer[i] >> 4;
        const unsigned char ucSize = buffer[i] & 0x0F;

        // 特殊情况，存在稀疏文件的话，ucSPosNum会为0，这样就记一下他前面的那个是哪个，然后最后再插进去
        DataInfo dataInfo;
        memcpy(&dataInfo.ui64UsedSclusters, &buffer[i + 1], ucSize);
        UINT64 ui64BeginScluster = 0;
        if (ucSPosNum != 0)
        {
            memcpy(&ui64BeginScluster, &buffer[i + ucSize + 1], min(ucSPosNum, 8));
        }
        if (ucSPosNum != 0 && i > 0)
        {
            if (vecDatarunList.empty())
            {
                return FALSE;
            }
            if (!_CalcNonFirstDataRunDevForLast(vecDatarunList[vecDatarunList.size() - 1].ui64BeginScluster, ui64BeginScluster, min(ucSPosNum, 8)))
            {
                return FALSE;
            }
        }
        dataInfo.ui64BeginScluster = ui64BeginScluster;
        if (!dataInfo.IsValid())
        {
            return FALSE;
        }
        if (dataInfo.ui64BeginScluster == 0)
        {
            DataInfo lastDataInfo;
            if (!vecDatarunList.empty())
            {
                lastDataInfo = vecDatarunList[vecDatarunList.size() - 1];
            }
           mapXiShuData.insert(std::make_pair(dataInfo, lastDataInfo));
        }
        else
        {
            vecDatarunList.push_back(dataInfo);
        }
        i += (ucSPosNum + ucSize + 1);
    }
    return FALSE;
}

BOOL CNTFSHelper::_CalcNonFirstDataRunDevForLast(const UINT64& ui64LastBeginCluster, UINT64& ui64BeginCluster, const UINT& uiNum)
{
    if (ui64BeginCluster == 0 || uiNum == 0)
    {
        return FALSE;
    }

    // 先判断符号位
    UINT64 ui64Sym = 0x80;
    UINT64 ui64Symbol = (UINT64)(ui64Sym << ((uiNum - 1) * 8));
    UINT64 ui64Symbol1 = ui64BeginCluster & ui64Symbol;
    if (ui64Symbol1 == ui64Symbol)
    {
        // 将64位前面的空位全部填充1，即为真实的偏移量
        ui64BeginCluster |= (0xFFFFFFFFFFFFFFFF << (uiNum * 8));
    }
    // 计算出实际的起始簇号
    ui64BeginCluster += ui64LastBeginCluster;

    return TRUE;
}

BOOL CNTFSHelper::_GetOneFileAttrInfoByDataRunBuffer(const PBYTE pDataRunBuffer, const UINT& uiIndexSPos, FileAttrInfo& fileAttrInfo)
{
    // 1.是否是目录
    UINT64 ui64AttrSign = 0;
    memcpy(&ui64AttrSign, &pDataRunBuffer[uiIndexSPos + 0x48], 8);
    // 先判断是不是系统且隐藏文件，系统文件不显示，不记录
    if (ui64AttrSign == 0 || (ui64AttrSign & 0x0006) == 0x0006)
    {
        return FALSE;
    }
    fileAttrInfo.bIsDir = (ui64AttrSign & 0x10000000) == 0x10000000;

    // 2.文件参考号(这个字段整个8个字节，但是后两个字节时序列号，只读前六位就可以)
    memcpy(&fileAttrInfo.ui64FileUniNum, &pDataRunBuffer[uiIndexSPos], 6);
    if (fileAttrInfo.ui64FileUniNum == 0)
    {
        return FALSE;
    }

    // 3.创建时间
    UINT64 ui64FileCreateTime = 0;
    memcpy(&ui64FileCreateTime, &pDataRunBuffer[uiIndexSPos + 0x18], 8);
    if (!_TimeFromRecordToSystemTime(ui64FileCreateTime, fileAttrInfo.stFileCreateTime))
    {
        return FALSE;
    }

    // 4.修改时间
    UINT64 ui64FileModifyTime = 0;
    memcpy(&ui64FileModifyTime, &pDataRunBuffer[uiIndexSPos + 0x20], 8);
    if (!_TimeFromRecordToSystemTime(ui64FileModifyTime, fileAttrInfo.stFileModifyTime))
    {
        return FALSE;
    }

    // 5.实际大小
    memcpy(&(fileAttrInfo.ui64FileSize), &pDataRunBuffer[uiIndexSPos + 0x40], 8);

    // 6.递归获取全路径
    BYTE recordBuffer[ONE_FILE_RECORD_SIZE + 1];
    if (_GetFileRecordByFileRefNum(fileAttrInfo.ui64FileUniNum, recordBuffer))
    {
        return _AutoGetFullPath(recordBuffer, fileAttrInfo.strFilePath);
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetFileDataByDataRun(const std::vector<DataInfo>& vecDataRunList, PBYTE pFileData)
{
    // 先检查下，读的范围不能超过4m
    UINT64 ui64FileDataSize = 0;
    for (auto datainfo : vecDataRunList)
    {
        ui64FileDataSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
    }
    if (ui64FileDataSize > ONE_FILE_BLOCK_SIZE)
    {
        return FALSE;
    }

    // 正式开始读
    UINT64 ui64FinishSize = 0;
    for (auto datainfo : vecDataRunList)
    {
        PBYTE pBuffer = (PBYTE)VirtualAlloc(NULL, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (datainfo.ui64BeginScluster == 0 && datainfo.ui64UsedSclusters != 0)
        {
            memcpy(&pFileData[ui64FinishSize], pBuffer, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE);
            ui64FinishSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
        }
        else
        {
            if (_GetAnySectionBuffer(datainfo.ui64BeginScluster * ONE_CLUSTER_SIZE, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE, pBuffer))
            {
                memcpy(&pFileData[ui64FinishSize], pBuffer, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE);
                ui64FinishSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
            }
            else
            {
                VirtualFree(pBuffer, 0, MEM_RELEASE);
                return FALSE;
            }
        }
        VirtualFree(pBuffer, 0, MEM_RELEASE);
    }

    // 校验一下数据长度
    if (ui64FinishSize != ui64FileDataSize)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetDriverHandleByDriverName(const CString& strDriverName, HANDLE& handle)
{
    CString strDriverRealName;
    strDriverRealName.Format(L"\\\\.\\%s:", strDriverName);

    //获取对应逻辑驱动器句柄
    handle = CreateFile(strDriverRealName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    return INVALID_HANDLE_VALUE != handle;
}

bool CompareFileAttrInfo(const FileAttrInfo& lInfo, const FileAttrInfo& rInfo)
{
    if (lInfo.bIsDir)
    {
        if (rInfo.bIsDir)
        {
            return lInfo.ui64FileUniNum < rInfo.ui64FileUniNum;
        }
        else
        {
            return true;
        }
    }
    else
    {
        if (rInfo.bIsDir)
        {
            return false;
        }
        else
        {
            return lInfo.ui64FileUniNum < rInfo.ui64FileUniNum;
        }
    }
}

void CNTFSHelper::_SortChildInfos(std::vector<FileAttrInfo>& vecChildInfos, UINT& uiDirNum)
{
    if (vecChildInfos.empty())
    {
        return;
    }

    // 先把文件的抽出来，只留下文件夹的，再把文件的插进去
    // 先去重
    std::sort(vecChildInfos.begin(), vecChildInfos.end(), CompareFileAttrInfo);
    std::vector<FileAttrInfo> ::iterator newit = std::unique(vecChildInfos.begin(), vecChildInfos.end());
    vecChildInfos.erase(newit, vecChildInfos.end());

    for (auto& info : vecChildInfos)
    {
        if (info.bIsDir)
        {
            uiDirNum++;
        }
        else
        {
            break;
        }
    }
}

BOOL CNTFSHelper::_WriteFileFromBuffer(const PBYTE pBuffer, const UINT64& ui64WriteLength, const CString& strFilePath, BOOL bTruncate)
{
    // 不是追加写，先创建新文件
    if (!bTruncate && PathFileExists(strFilePath))
    {
        BOOL bCopySuccess = FALSE;
        for (int i = 0; i < 5; ++i)
        {
            if (DeleteFile(strFilePath))
            {
                bCopySuccess = TRUE;
                break;
            }
            Sleep(200);
        }
        if (!bCopySuccess)
        {
            return FALSE;
        }
    }

    // 打开或创建文件，也加个重试
    if (m_hFile == NULL)
    {
        int iRetryTimes = 0;
        while (true)
        {
            m_hFile = CreateFile(strFilePath,
                GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                bTruncate ? OPEN_EXISTING : CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (m_hFile == INVALID_HANDLE_VALUE)
            {
                DWORD dwError = GetLastError();
                if (dwError == 32)
                {
                    CloseHandle(m_hFile);
                    m_hFile = NULL;
                }
            }
            else
            {
                break;
            }
            if (iRetryTimes == 10)
            {
                return FALSE;
            }
            ++iRetryTimes;
            Sleep(200);
        }
    }

    LARGE_INTEGER li = {0};
    BOOL bRet = SetFilePointerEx(m_hFile, li, &li, FILE_END);
    // 写文件，失败最多每0.2秒重试一次，重试5次，一直不成功返回直接报错
    for (int i = 0; i < 5; ++i)
    {
        DWORD dwRealWriteSize = 0;
        if (WriteFile(m_hFile, pBuffer, ui64WriteLength, &dwRealWriteSize, NULL) && ui64WriteLength == dwRealWriteSize)
        {
            return TRUE;
        }
        Sleep(200);
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetDataRunBy80AttrFrom20Attr(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunInfos)
{
    UINT ui20AttrSPos = 0;
    UINT uiAttrLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0x20, ui20AttrSPos, uiAttrLength))
    {
        UINT uiLength = ui20AttrSPos + uiAttrLength;
        ui20AttrSPos += 0x18;
        UINT uiAttrType = 0;
        uiAttrLength = 0;
        while (ui20AttrSPos < uiLength)
        {
            memcpy(&uiAttrType, &pRecordBuffer[ui20AttrSPos], 4);
            if (uiAttrType != 0x80)
            {
                memcpy(&uiAttrLength, &pRecordBuffer[ui20AttrSPos + 0x04], 2);
                ui20AttrSPos += uiAttrLength;
                continue;
            }

            UINT64 ui64FileNum = 0;
            memcpy(&ui64FileNum, &pRecordBuffer[ui20AttrSPos + 0x10], 6);

            UINT64 ui64CurFileNum = 0;
            UINT ui64CurFileNumXP = 0;
            memcpy(&ui64CurFileNum, &pRecordBuffer[0x20], 6);
            memcpy(&ui64CurFileNumXP, &pRecordBuffer[0x2C], 4);

            if (ui64FileNum == ui64CurFileNum || ui64FileNum == ui64CurFileNumXP)
            {
                memcpy(&uiAttrLength, &pRecordBuffer[ui20AttrSPos + 0x04], 2);
                ui20AttrSPos += uiAttrLength;
                continue;
            }

            BYTE newRecordBuffer[ONE_FILE_RECORD_SIZE + 1] = { 0 };
            if (_GetFileRecordByFileRefNum(ui64FileNum, newRecordBuffer))
            {
                UINT ui80AttrSPos = 0;
                UINT ui80AttrLength = 0;
                if (_FindAnyAttrSPosByFileRecord(newRecordBuffer, 0x80, ui80AttrSPos, ui80AttrLength))
                {
                    UINT uiDataRunSPos = ui80AttrSPos + 0x40;
                    UINT uiDataRunLength = ui80AttrLength - 0x40;
                    std::vector<DataInfo> tmpDataRunList;
                    if (_GetDataRunList(newRecordBuffer, uiDataRunSPos, uiDataRunLength, tmpDataRunList))
                    {
                        vecDataRunInfos.insert(vecDataRunInfos.end(), tmpDataRunList.begin(), tmpDataRunList.end());
                    }
                }
            }

            // 有可能有多个80，继续循环
            memcpy(&uiAttrLength, &pRecordBuffer[ui20AttrSPos + 0x04], 2);
            ui20AttrSPos += uiAttrLength;
        }
    }

    return !vecDataRunInfos.empty();
}

BOOL CNTFSHelper::MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath)
{
    // 1.读取源文件文件记录
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    if (_GetFileRecordByFileRefNum(ui64SrcFileNum, buffer))
    {
        // 2.根据源文件大小，判断需不需要分块读写，不分块就直接开辟空间，一次性读出来，再一次性写到目标路径
        std::vector<DataInfo> vecDataInfos;
        if (ui64SrcFileSize <= ONE_FILE_BLOCK_SIZE)
        {
            PBYTE pFlieDataBuffer = (PBYTE)VirtualAlloc(NULL, ui64SrcFileSize + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (CNTFSHelper::GetInstance()->_GetFileDataByFileRecord(buffer, pFlieDataBuffer, vecDataInfos) != 1)
            {
                return FALSE;
            }
            BOOL bRet = _WriteFileFromBuffer(pFlieDataBuffer, ui64SrcFileSize, strDestPath);
            VirtualFree(pFlieDataBuffer, 0, MEM_RELEASE);
            if (m_hFile)
            {
                CloseHandle(m_hFile);
                m_hFile = NULL;
            }
            return bRet;
        }
        else
        {
            // 分块读写不需要在这一层分配内存，下一层接口会自己维护
            if (CNTFSHelper::GetInstance()->_GetFileDataByFileRecord(buffer, nullptr, vecDataInfos) != 2)
            {
                return FALSE;
            }
            return _BigFileBlockReadAndWrite(vecDataInfos, strDestPath, ui64SrcFileSize);
        }
    }
    return FALSE;
}

BOOL CNTFSHelper::GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum)
{
    // 1.根据当前dir文件参考号获取对应文件记录
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    if (_GetFileRecordByFileRefNum(ui64ParentRefNum, buffer))
    {
        UINT uiAttrSPos = 0;
        UINT uiAttrLength = 0;
        BOOL bHave20HAttr = FALSE;
        UINT uiDirNumIn20HAttr = 0;
        std::vector<FileAttrInfo> vecChildInfosIn20Attr;
        // 先看有没有20属性，有的话，先去20属性里面把对应的A0和90的子项集合拿出来
        if (_FindAnyAttrSPosByFileRecord(buffer, 0x20, uiAttrSPos, uiAttrLength))
        {
            bHave20HAttr = TRUE;
            if (!_GetA0HAttrChildListFrom20HAttr(buffer, vecChildInfosIn20Attr, uiDirNumIn20HAttr))
            {
                _Get90HAttrChildListFrom20HAttr(buffer, vecChildInfosIn20Attr, uiDirNumIn20HAttr);
            }
        }

        uiAttrSPos = 0;
        uiAttrLength = 0;
        // 进一步遍历分析子项，首先看看有没有A0H属性，有就直接分析A0，没有就只分析90属性
        if (_FindAnyAttrSPosByFileRecord(buffer, 0xA0, uiAttrSPos, uiAttrLength))
        {
            // 遍历datarun，拿到全部簇流的起始位置和占用长度（单位：簇）
            std::vector<DataInfo> vecDataRunLists;
            if (CNTFSHelper::GetInstance()->_GetA0HAttrDataRunLists(buffer, vecDataRunLists))
            {
                // 从datarun获取所有子项
                if (CNTFSHelper::GetInstance()->_GetChildFileAttrInfoByRunList(vecDataRunLists, vecChildAttrInfos))
                {
                    if (bHave20HAttr)
                    {
                        vecChildAttrInfos.insert(vecChildAttrInfos.end(), vecChildInfosIn20Attr.begin(), vecChildInfosIn20Attr.end());
                    }
                    _SortChildInfos(vecChildAttrInfos, uiDirNum);
                    return TRUE;
                }
            }
        }
        else
        {
            // 90索引项就在属性体里面，直接拿就行
            uiAttrSPos = 0;
            UINT uiAttrLength = 0;
            if (_FindAnyAttrSPosByFileRecord(buffer, 0x90, uiAttrSPos, uiAttrLength))
            {
                if (_Get90HAttrChildAttrInfos(buffer, vecChildAttrInfos))
                {
                    if (bHave20HAttr)
                    {
                        vecChildAttrInfos.insert(vecChildAttrInfos.end(), vecChildInfosIn20Attr.begin(), vecChildInfosIn20Attr.end());
                    }
                    _SortChildInfos(vecChildAttrInfos, uiDirNum);
                    return TRUE;
                }
            }
            else if (bHave20HAttr)
            {
				vecChildAttrInfos.insert(vecChildAttrInfos.end(), vecChildInfosIn20Attr.begin(), vecChildInfosIn20Attr.end());
				_SortChildInfos(vecChildAttrInfos, uiDirNum);
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum)
{
    if (ui64FileNum == 5 || ui64FileNum == 0)
    {
        ui64ParentFileNum = 0;
        return TRUE;
    }

    // 从30属性读父参考号
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    if (_GetFileRecordByFileRefNum(ui64FileNum, buffer))
    {
        UINT uiAttrSPos = 0;
        UINT uiAttrLength = 0;
        if (_FindAnyAttrSPosByFileRecord(buffer, 0x30, uiAttrSPos, uiAttrLength))
        {
            if (_GetParentFileNumBy30HAttr(buffer, uiAttrSPos, ui64ParentFileNum))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL CNTFSHelper::GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath)
{
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1] = {0};
    if (_GetFileRecordByFileRefNum(ui64FileNum, buffer))
    {
        return _AutoGetFullPath(buffer, strFilePath);
    }
    return FALSE;
}

void CNTFSHelper::SetProgressWndHandle(const HWND& hProgressWnd)
{
    m_hProgressWnd = hProgressWnd;
}

BOOL CNTFSHelper::_GetA0HAttrDataRunLists(const PBYTE pRecordBuffer, std::vector<DataInfo>& vecDataRunLists)
{
    unsigned short usDataRunSPos = 0;
    if (_GetA0HAttrDataRunSPos(pRecordBuffer, usDataRunSPos))
    {
        // 1.获取A0属性长度-datarun起始位置=datarun长度 ，遍历不能超过这个长度
        UINT uiA0AttrSPos = 0;
        UINT uiAttrLength = 0;
        if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0xA0, uiA0AttrSPos, uiAttrLength))
        {
            UINT uiDatarunMaxLength = uiA0AttrSPos + uiAttrLength - usDataRunSPos;
            return _GetDataRunList(pRecordBuffer, usDataRunSPos, uiDatarunMaxLength, vecDataRunLists);
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_Get90HAttrChildAttrInfos(const PBYTE pRecordBuffer, std::vector<FileAttrInfo>& vecFileAttrLists)
{
    // 1.获取A0属性起始位置-datarun起始位置=datarun长度 ，遍历不能超过这个长度
    UINT ui90AttrSPos = 0;
    UINT ui90AttrLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0x90, ui90AttrSPos, ui90AttrLength))
    {
        UINT uiFirstIndexSPos = ui90AttrSPos + 0x40;
        UINT uiIndexSize = ui90AttrSPos + ui90AttrLength;

        // 遍历所有索引项，拿到相关信息
        while (uiFirstIndexSPos < uiIndexSize)
        {
            unsigned short usOneIndexSize = 0;
            memcpy(&usOneIndexSize, &pRecordBuffer[uiFirstIndexSPos + 0x08], 2);

            // 解析索引项
            FileAttrInfo attrInfo;
            if (_GetOneFileAttrInfoByDataRunBuffer(pRecordBuffer, uiFirstIndexSPos, attrInfo))
            {
                vecFileAttrLists.push_back(attrInfo);
            }

            // 偏移到下个索引起始位置继续循环
            uiFirstIndexSPos += usOneIndexSize;
        }
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetChildFileAttrInfoByRunList(const std::vector<DataInfo>& vecDataRunLists, std::vector<FileAttrInfo>& vecChildAttrInfos)
{
    for (auto datainfo : vecDataRunLists)
    {
        PBYTE pBuffer = (PBYTE)VirtualAlloc(NULL, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (_GetAnySectionBuffer(datainfo.ui64BeginScluster * ONE_CLUSTER_SIZE, datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE, pBuffer))
        {
            for (int i = 0; i < datainfo.ui64UsedSclusters; ++i)
            {
                // 先解析索引头，拿到第一个索引项的偏移和索引总大小（超出总大小的视为无效索引）
                UINT uiSPos = i * ONE_CLUSTER_SIZE;
                UINT uiIndexOffset = 0;
                memcpy(&uiIndexOffset, &pBuffer[uiSPos + 0x18], 4);

                UINT uiIndexSize = 0;
                memcpy(&uiIndexSize, &pBuffer[uiSPos + 0x1C], 4);

                // 遍历所有索引项，拿到相关信息
                UINT uiIndexSPos = uiSPos + uiIndexOffset + 0x18;
                while (uiIndexSPos < uiIndexSize + uiSPos)
                {
                    unsigned short usOneIndexSize = 0;
                    memcpy(&usOneIndexSize, &pBuffer[uiIndexSPos + 0x08], 2);

                    // 解析索引项
                    FileAttrInfo attrInfo;
                    if (_GetOneFileAttrInfoByDataRunBuffer(pBuffer, uiIndexSPos, attrInfo))
                    {
                        vecChildAttrInfos.push_back(attrInfo);
                    }

                    // 偏移到下个索引起始位置继续循环
                    uiIndexSPos += usOneIndexSize;
                }
            }
        }
        else
        {
            VirtualFree(pBuffer, 0, MEM_RELEASE);
            return FALSE;
        }
        VirtualFree(pBuffer, 0, MEM_RELEASE);
    }

    return TRUE;
}

UINT CNTFSHelper::_GetFileDataByFileRecord(const PBYTE pFileRecordBuffer, PBYTE pFileDataBuffer, std::vector<DataInfo>& vecDataRunList)
{
    // 1.从属性头判断80属性是不是常驻，常驻在文件记录里读数据，非常驻要去找datarun
    UINT ui80AttrSPos = 0;
    UINT ui80AttrLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pFileRecordBuffer, 0x80, ui80AttrSPos, ui80AttrLength))
    {
        BYTE byFlag = 2;
        memcpy(&byFlag, &pFileRecordBuffer[ui80AttrSPos + 0x08], 1);
        BOOL bPermanent = byFlag == 0;

        // 2.先处理常驻，直接读文件记录里面的数据
        if (bPermanent)
        {
            // 属性总长度
            UINT uiAttrLength = 0;
            memcpy(&uiAttrLength, &pFileRecordBuffer[ui80AttrSPos + 0x04], 4);

            // 属性体（文件真实数据）长度
            UINT uiAttrBodyLength = uiAttrLength - 0x18;

            if (pFileDataBuffer)
            {
                memcpy(pFileDataBuffer, &pFileRecordBuffer[ui80AttrSPos + 0x18], uiAttrBodyLength);
            }

            return 1;
        }
        else
        {
            // 非常驻就需要去读datarun位置，然后从文件记录外的具体位置读数据，这里可能涉及到文件真实数据
            // 巨大的问题，所以可能需要分块读
            // 这里还要考虑20属性的问题，先判断下有没有20属性，如果有，先把里面的80对应的文件记录里面的80属性中的datarun拿出来存一下
            // 再与本文件记录的80属性下面的datarun合并（如果存在，合并方式为追加）
            std::vector<DataInfo> vecDataRunListIn20Attr;
            _GetDataRunBy80AttrFrom20Attr(pFileRecordBuffer, vecDataRunListIn20Attr);

            UINT uiDataRunSPos = ui80AttrSPos + 0x40;
            UINT uiAttrLength = 0;
            memcpy(&uiAttrLength, &pFileRecordBuffer[ui80AttrSPos + 0x04], 4);
            UINT uiDataRunLength = uiAttrLength - 0x40;
            if (_GetDataRunList(pFileRecordBuffer, uiDataRunSPos, uiDataRunLength, vecDataRunList))
            {
                if (!vecDataRunListIn20Attr.empty())
                {
                    vecDataRunList.insert(vecDataRunList.end(), vecDataRunListIn20Attr.begin(), vecDataRunListIn20Attr.end());
                }
                UINT64 ui64AllFileDataSize = 0;
                for (auto dataInfo : vecDataRunList)
                {
                    ui64AllFileDataSize += dataInfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
                }
                if (ui64AllFileDataSize > ONE_FILE_BLOCK_SIZE)
                {
                    return 2;
                }
                if (pFileDataBuffer)
                {
                    return _GetFileDataByDataRun(vecDataRunList, pFileDataBuffer);
                }
            }
        }
    }
    else
    {
        // 没有80属性，说明文件的datarun全在20里面
        if (_GetDataRunBy80AttrFrom20Attr(pFileRecordBuffer, vecDataRunList))
        {
            UINT64 ui64AllFileDataSize = 0;
            for (auto dataInfo : vecDataRunList)
            {
                ui64AllFileDataSize += dataInfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
            }
            if (ui64AllFileDataSize > ONE_FILE_BLOCK_SIZE)
            {
                return 2;
            }
            if (pFileDataBuffer)
            {
                return _GetFileDataByDataRun(vecDataRunList, pFileDataBuffer);
            }
        }
    }

    return 0;
}

BOOL CNTFSHelper::_BigFileBlockReadAndWrite(const std::vector<DataInfo>& vecDataRunList, const CString& strWriteFilePath, const UINT64& ui64FileRealSize)
{
    // 正式开始读
    UINT64 ui64AllFileSize = 0;
    for each (auto datainfo in vecDataRunList)
    {
        ui64AllFileSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
    }

    // 记录一下分配的数据区域大小和文件实际大小的差值（补位的字节数，下面写文件的时候结尾要减掉）
    UINT uiFillSize = ui64AllFileSize - ui64FileRealSize;
    
    double dPercent = 0;
    BOOL bTranCate = FALSE;
    UINT uiTime = 0;
    for each (auto datainfo in vecDataRunList)
    {
        ++uiTime;
        UINT64 ui64FinishSize = 0;
        UINT64 ui64RemainSize = datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
        while (ui64RemainSize > 0)
        {
            UINT64 ui64ReadSize = min(ui64RemainSize, ONE_FILE_BLOCK_SIZE);
            PBYTE pBuffer = (PBYTE)VirtualAlloc(NULL, ui64ReadSize + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (datainfo.ui64BeginScluster == 0 && datainfo.ui64UsedSclusters != 0)
            {                
                // 最后一块的时候，写的时候要剪掉之前保存的结尾的填充位
                if (!_WriteFileFromBuffer(pBuffer, (uiTime == vecDataRunList.size() && ui64RemainSize <= ui64ReadSize) ? ui64ReadSize - uiFillSize : ui64ReadSize, strWriteFilePath, bTranCate))
                {
                    VirtualFree(pBuffer, 0, MEM_RELEASE);
                    if (m_hFile)
                    {
                        CloseHandle(m_hFile);
                        m_hFile = NULL;
                    }
                    return FALSE;
                }
                bTranCate = TRUE;
                ui64FinishSize += ui64ReadSize;
                ui64RemainSize = ui64RemainSize > ui64ReadSize ? ui64RemainSize - ui64ReadSize : 0;
            }
            else
            {
                if (_GetAnySectionBuffer(datainfo.ui64BeginScluster * ONE_CLUSTER_SIZE + ui64FinishSize, ui64ReadSize, pBuffer))
                {
                    // 最后一块的时候，写的时候要剪掉之前保存的结尾的填充位
                    if (!_WriteFileFromBuffer(pBuffer, (uiTime == vecDataRunList.size() && ui64RemainSize <= ui64ReadSize) ? ui64ReadSize - uiFillSize : ui64ReadSize, strWriteFilePath, bTranCate))
                    {
                        VirtualFree(pBuffer, 0, MEM_RELEASE);
                        if (m_hFile)
                        {
                            CloseHandle(m_hFile);
                            m_hFile = NULL;
                        }
                        return FALSE;
                    }
                    bTranCate = TRUE;
                    ui64FinishSize += ui64ReadSize;
                    ui64RemainSize = ui64RemainSize > ui64ReadSize ? ui64RemainSize - ui64ReadSize : 0;
                }
                else
                {
                    VirtualFree(pBuffer, 0, MEM_RELEASE);
                    if (m_hFile)
                    {
                        CloseHandle(m_hFile);
                        m_hFile = NULL;
                    }
                    return FALSE;
                }
            }
            VirtualFree(pBuffer, 0, MEM_RELEASE);

            // 计算下进度，post给ui
            dPercent += (double)(ui64ReadSize * 100.00 / ui64AllFileSize);
            // POST
            if (::IsWindow(m_hProgressWnd))
            {
                ::SendMessage(m_hProgressWnd, MSG_UPDATE_PROGRESS, (WPARAM)dPercent, 0);
            }
        }
        // 每一块datarun读完都校验一下
        if (ui64FinishSize != datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE || ui64RemainSize != 0)
        {
            if (m_hFile)
            {
                CloseHandle(m_hFile);
                m_hFile = NULL;
            }
            return FALSE;
        }
    }

    if (m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = NULL;
    }
    return TRUE;
}

BOOL CNTFSHelper::_GetFileNameByFileRecord(const PBYTE pRecordBuffer, CString& strFileName)
{
    UINT ui30HAttrSPos = 0;
    if (_Get30HAttrSPosByFileRecord(pRecordBuffer, ui30HAttrSPos))
    {
        return _GetFileNameBy30HAttr(pRecordBuffer, ui30HAttrSPos, strFileName);
    }
    else
    {
        BYTE byNewBuffer[ONE_FILE_RECORD_SIZE + 1] = { 0 };
        if (_Get30HAttrSPosFrom20HAttr(pRecordBuffer, ui30HAttrSPos, byNewBuffer))
        {
            return _GetFileNameBy30HAttr(byNewBuffer, ui30HAttrSPos, strFileName);
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetFileNameBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, CString& strFileName)
{
    // 文件属性包含属性头和属性体，属性头没啥用，所以先偏移起始位置到属性体起始位置
    UINT uiAttrBodySPos = ui30HSPos + 0x18;

    // 1.文件名长度
    BYTE fileNameSize = 0;
    memcpy(&fileNameSize, &pRecordBuffer[uiAttrBodySPos + 0x40], 1);

    // 2.文件名
    TCHAR* pNameBuffer = new TCHAR[fileNameSize * 2 + 2];
    ZeroMemory(pNameBuffer, fileNameSize * 2 + 2);
    memcpy(pNameBuffer, &pRecordBuffer[uiAttrBodySPos + 0x42], fileNameSize * 2);

    strFileName = CString(pNameBuffer);
    if (pNameBuffer)
    {
        delete[] pNameBuffer;
        pNameBuffer = nullptr;
    }
    return !strFileName.IsEmpty();
}

BOOL CNTFSHelper::_GetParentFileNumByFileRecord(const PBYTE pRecordBuffer, UINT64& ui64ParentFileNum)
{
    UINT ui30HAttrSPos = 0;
    if (_Get30HAttrSPosByFileRecord(pRecordBuffer, ui30HAttrSPos))
    {
        return _GetParentFileNumBy30HAttr(pRecordBuffer, ui30HAttrSPos, ui64ParentFileNum);
    }
    else
    {
        BYTE byNewBuffer[ONE_FILE_RECORD_SIZE + 1] = { 0 };
        if (_Get30HAttrSPosFrom20HAttr(pRecordBuffer, ui30HAttrSPos, byNewBuffer))
        {
            return _GetParentFileNumBy30HAttr(byNewBuffer, ui30HAttrSPos, ui64ParentFileNum);
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_GetParentFileNumBy30HAttr(const PBYTE pRecordBuffer, const UINT& ui30HSPos, UINT64& ui64ParentFileNum)
{
    // 文件属性包含属性头和属性体，属性头没啥用，所以先偏移起始位置到属性体起始位置
    UINT uiAttrBodySPos = ui30HSPos + 0x18;

    // 1.父目录参考号
    memcpy(&ui64ParentFileNum, &pRecordBuffer[uiAttrBodySPos], 6);

    return ui64ParentFileNum != 0;
}

BOOL CNTFSHelper::_AutoGetFullPath(const PBYTE pRecordBuffer, CString& strPath)
{
    // 1.获取当前文件的文件名
    CString strCurFileName;
    if (_GetFileNameByFileRecord(pRecordBuffer, strCurFileName))
    {
        // 如果文件名为“.”说明是根目录，直接返回
        if (strCurFileName.CompareNoCase(L".") == 0)
        {
            strPath = strCurFileName;
            return TRUE;
        }
        TCHAR buffer1[MAX_PATH * 2 + 2] = { 0 };
        memcpy(&buffer1, strCurFileName, strCurFileName.GetLength() * 2);
        if (!PathAppend(buffer1, strPath))
        {
            return FALSE;
        }
        strPath = buffer1;

        // 2.获取父目录文件参考号
        UINT64 ui64ParentFileNum = 0;
        if (_GetParentFileNumByFileRecord(pRecordBuffer, ui64ParentFileNum))
        {
            // 如果父目录文件参考号是5，说明已经是根目录了，拼上盘符返回
            if (ui64ParentFileNum == 5)
            {
                CString strFilePath = m_strCurDriverName;
                strFilePath += L":";
                TCHAR buffer[MAX_PATH * 2 + 2] = {0};
                memcpy(&buffer, strFilePath, strFilePath.GetLength() * 2);
                if (!PathAppend(buffer, strPath))
                {
                    return FALSE;
                }
                strPath = buffer;
                return TRUE;
            }
            else
            {
                // 3.获取父目录文件记录
                BYTE byParentBuffer[ONE_FILE_RECORD_SIZE + 1];
                ZeroMemory(byParentBuffer, ONE_FILE_RECORD_SIZE + 1);
                if (_GetFileRecordByFileRefNum(ui64ParentFileNum, byParentBuffer))
                {
                    return _AutoGetFullPath(byParentBuffer, strPath);
                }
            }
        }
    }

    return FALSE;
}

BOOL CNTFSHelper::_TimeFromRecordToSystemTime(const UINT64& ui64Time, SYSTEMTIME& systemTime)
{
    FILETIME fileTime;
    fileTime.dwLowDateTime = (DWORD)ui64Time;
    fileTime.dwHighDateTime = (DWORD)(ui64Time >> 32);
    return FileTimeToSystemTime(&fileTime, &systemTime);
}

BOOL CNTFSHelper::SetCurDriverInfo(const CString& strDriverName)
{
    if (strDriverName == m_strCurDriverName)
    {
        return TRUE;
    }
    _ResetCurDriverInfo();
    m_strCurDriverName = strDriverName;
    return _GetDriverHandleByDriverName(strDriverName, m_hanCurDriver);
}

CString CNTFSHelper::GetCurDriverName()
{
    return m_strCurDriverName;
}

void CNTFSHelper::_ResetCurDriverInfo()
{
    m_strCurDriverName = L"";
    if (m_hanCurDriver)
    {
        CloseHandle(m_hanCurDriver);
        m_hanCurDriver = NULL;
    }
}

BOOL CNTFSHelper::_IsLetter(const TCHAR& str)
{
    if ((str >= L'a' && str <= L'z') || (str >= L'A' && str <= L'Z'))
    {
        return TRUE;
    }
    return FALSE;
}
