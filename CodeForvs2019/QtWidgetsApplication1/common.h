#pragma once
#include "stdafx.h"
#include <atlstr.h>
#include <vector>
#include <map>
#include <QtWinExtras/QtWinExtras>

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

Q_DECLARE_METATYPE(FileAttrInfo)
