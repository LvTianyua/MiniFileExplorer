#pragma once
#include "stdafx.h"
#include <atlstr.h>
#include <vector>
#include <map>
#include <QtWinExtras/QtWinExtras>

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

    UINT64 ui64BeginScluster = 0;            // 起始簇号
    UINT64 ui64UsedSclusters = 0;            // 占用多少簇
}DataInfo, *pDataInfo;

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
    BYTE byNameSpace = 0;           // 文件名空间

    _FileAttrInfo()
    {
        ZeroMemory(&stFileCreateTime, sizeof(stFileCreateTime));
        ZeroMemory(&stFileModifyTime, sizeof(stFileModifyTime));
    }
}FileAttrInfo, *pFileAttrInfo;

Q_DECLARE_METATYPE(FileAttrInfo)
