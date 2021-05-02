#include "stdafx.h"
#include "NTFSHelper.h"
#include <memory>
#include <winioctl.h>
#include <algorithm>
#include <xutility>
#include <string>
#include <map>

#define ONE_SECTOR_SIZE             512         //��������С���ֽڣ�
#define ONE_CLUSTER_SIZE            8 * 512     //���ش�С���ֽڣ�
#define ONE_FILE_RECORD_SIZE        1024        //���ļ���¼��С���ֽڣ�
#define ONE_FILE_BLOCK_SIZE         4 * 1024 * 1024 //���ļ��ֿ��С���ֽڣ�

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
    //У�����
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
    //У�����
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

    //��ȡ�ļ���¼��С
    if (!DeviceIoControl(m_hanCurDriver, FSCTL_GET_NTFS_VOLUME_DATA, NULL,
        0, &ntfsVolumeDataBuffer, sizeof(NTFS_VOLUME_DATA_BUFFER), &returnByte, NULL))
    {
        return FALSE;
    }

    // ��������������ļ��ο���
    NTFS_FILE_RECORD_INPUT_BUFFER ntfsFileRecordInputBuffer;
    ntfsFileRecordInputBuffer.FileReferenceNumber.QuadPart = ui64FileRefNum;

    // ���ｫoutbuffer�����ڶ����棬��Ҫ����ΪNTFS_FILE_RECORD_OUTPUT_BUFFER�ṹ������ɱ䳤�ȵ����飬��ջ�Ϸ���ᵼ�����鳤�ȸı��ʱ������������ڴ�
    // �Ḳ������ջ����Ч������ڴ棬���ºܶ�ֲ�������ʧЧ�������޷�Ԥ֪�����⣡����
    PNTFS_FILE_RECORD_OUTPUT_BUFFER pNtfsFileRecordOutputBuffer = new NTFS_FILE_RECORD_OUTPUT_BUFFER[sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1];
    ZeroMemory(pNtfsFileRecordOutputBuffer, sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1);
    if (!DeviceIoControl(m_hanCurDriver,
        FSCTL_GET_NTFS_FILE_RECORD,
        &ntfsFileRecordInputBuffer,
        sizeof(NTFS_FILE_RECORD_INPUT_BUFFER),
        pNtfsFileRecordOutputBuffer,
        sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + ntfsVolumeDataBuffer.BytesPerFileRecordSegment - 1,//MSDN����˵��
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
    // 1.�õ�DBR
    NTFSDBR dbrInfo;
    _GetDBRInfo(dbrInfo);

    // 2.����dbr��ȡMFT�׵�ַ
    UINT64 ui64MFTSPos = _GetMFTStartPositionByDBR(dbrInfo);

    // 3.��ʼ����Ѱ�ҵ�ui64FileRefNum��
    UINT64 ui64Num = 0;
    while (ui64Num <= ui64MFTSPos)
    {
        // �������жϵ�ǰ���������׵�ַ�ǲ���0��Ҫ��0��ƫ�Ƶ��¸�1024��һ���ļ���¼����ֱ����������Ҫ��λ�ã�
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
    // 1.�����ļ���¼ͷ�� �ҵ���һ������λ�ã���¼��һ������ƫ���׵�ַ���ļ���¼�׵�ַƫ��0x14λ�ã�ռ��2�ֽڣ�
    memcpy(&ui30HSpos, &pRecordBuffer[0x14], 2);

    // 2.��ʼ˳��Ѱ�����ԣ�Ŀ��30H����
    // ������һ���������ͣ�10H���ԣ���פ���˴��ò���ֱ��������һ������
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);
        ui30HSpos += uiFirstAttrLength;
    }

    // �����ڶ������ԣ�20H���Էǳ�פ���е��У��е�û�У��Ƚ������ͣ������20H��Ҫ���⴦�������30H�ͽ���
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
    // 1.�����ļ���¼ͷ�� �ҵ���һ������λ�ã���¼��һ������ƫ���׵�ַ���ļ���¼�׵�ַƫ��0x14λ�ã�ռ��2�ֽڣ�
    memcpy(&ui30HSpos, &pRecordBuffer[0x14], 2);

    // 2.��ʼ˳��Ѱ�����ԣ�Ŀ��30H����
    // ������һ���������ͣ�10H���ԣ���פ���˴��ò���ֱ��������һ������
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);
        ui30HSpos += uiFirstAttrLength;
    }

    // �����ڶ������ԣ��������20���ԣ�����false����20��ȡ��ȷ���ֿռ�30���µ��ļ���¼buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[ui30HSpos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui30HSpos + 0x04], 4);

        // ����20�����б�Ѱ��30���ԣ����жϲο����ǲ��Ǻͱ��ļ���¼�ο���һ�£�һ�²���
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
    // 1.�����ļ���¼ͷ�� �ҵ���һ������λ�ã���¼��һ������ƫ���׵�ַ���ļ���¼�׵�ַƫ��0x14λ�ã�ռ��2�ֽڣ�
    UINT uiA0HSPos = 0;
    memcpy(&uiA0HSPos, &pRecordBuffer[0x14], 2);

    // 2.��ʼ˳��Ѱ�����ԣ�Ŀ��A0H����
    // ������һ���������ͣ�10H���ԣ���פ���˴��ò���ֱ��������һ������
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[uiA0HSPos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 4);
        uiA0HSPos += uiFirstAttrLength;
    }

    // �����ڶ������ԣ��������20���ԣ�����false����20��ȡA0��Ӧ���µ��ļ���¼buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[uiA0HSPos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[uiA0HSPos + 0x04], 4);

        // ����20����������б���A0���Ҳ�������False
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
                // ����datarun���õ�ȫ����������ʼλ�ú�ռ�ó��ȣ���λ���أ�
                std::vector<DataInfo> vecDataRunLists;
                if (CNTFSHelper::GetInstance()->_GetA0HAttrDataRunLists(newRecordBuffer, vecDataRunLists))
                {
                    // ��datarun��ȡ��������
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
    // 1.�����ļ���¼ͷ�� �ҵ���һ������λ�ã���¼��һ������ƫ���׵�ַ���ļ���¼�׵�ַƫ��0x14λ�ã�ռ��2�ֽڣ�
    UINT ui90HSPos = 0;
    memcpy(&ui90HSPos, &pRecordBuffer[0x14], 2);

    // 2.��ʼ˳��Ѱ�����ԣ�Ŀ��A0H����
    // ������һ���������ͣ�10H���ԣ���פ���˴��ò���ֱ��������һ������
    UINT uiFirstAttrLength = 0;
    UINT uiFirstAttrType = 0;
    memcpy(&uiFirstAttrType, &pRecordBuffer[ui90HSPos], 4);
    if (uiFirstAttrType == 0x10)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 4);
        ui90HSPos += uiFirstAttrLength;
    }

    // �����ڶ������ԣ��������20���ԣ�����false����20��ȡA0��Ӧ���µ��ļ���¼buffer
    UINT uiSecondAttrType = 0;
    memcpy(&uiSecondAttrType, &pRecordBuffer[ui90HSPos], 4);
    if (uiSecondAttrType == 0x20)
    {
        memcpy(&uiFirstAttrLength, &pRecordBuffer[ui90HSPos + 0x04], 4);

        // ����20����������б���A0���Ҳ�������False
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
    // 1.�����ļ���¼ͷ�� �ҵ���һ������λ�ã���¼��һ������ƫ���׵�ַ���ļ���¼�׵�ַƫ��0x14λ�ã�ռ��2�ֽڣ�
    memcpy(&uiAttrSPos, &pRecordBuffer[0x14], 2);

    while (true)
    {
        // ���uiAttrSPos�Ѿ�����һ���ļ���¼��˵��û�ҵ���ֱ�ӷ���
        if (uiAttrSPos >= ONE_FILE_RECORD_SIZE)
        {
            return FALSE;
        }

        // ��ʼ˳��Ѱ�����ԣ�Ŀ��uiAttrType����
        // �ж����ͣ�uiAttrType���أ���������ƫ�Ƶ��¸�������ʼλ�ü���ѭ��
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
    // �Ȼ�ȡA0������ʼƫ��
    UINT uiA0HSPos = 0;
    UINT uiA0HLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0xA0, uiA0HSPos, uiA0HLength))
    {
        // ��������ʼ��ַλ��0x20ƫ��
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
    // ���ļ���¼�н�ȡdatarun����
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    memcpy(&buffer, &pRecordBuffer[uiDatarunSPos], uiDataRunLength);

    std::map<DataInfo, DataInfo> mapXiShuData;
    // ѭ������datarunlist��ȡ��Ϣ
    for (UINT i = 0; i < uiDataRunLength;)
    {
        if (buffer[i] == 0)
        {
            // ��������
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
        // �Ƚ�����ʼ�غźʹ�Сռ�÷ֱ�ʹ���˶����ֽ�
        const unsigned char ucSPosNum = buffer[i] >> 4;
        const unsigned char ucSize = buffer[i] & 0x0F;

        // �������������ϡ���ļ��Ļ���ucSPosNum��Ϊ0�������ͼ�һ����ǰ����Ǹ����ĸ���Ȼ������ٲ��ȥ
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

    // ���жϷ���λ
    UINT64 ui64Sym = 0x80;
    UINT64 ui64Symbol = (UINT64)(ui64Sym << ((uiNum - 1) * 8));
    UINT64 ui64Symbol1 = ui64BeginCluster & ui64Symbol;
    if (ui64Symbol1 == ui64Symbol)
    {
        // ��64λǰ��Ŀ�λȫ�����1����Ϊ��ʵ��ƫ����
        ui64BeginCluster |= (0xFFFFFFFFFFFFFFFF << (uiNum * 8));
    }
    // �����ʵ�ʵ���ʼ�غ�
    ui64BeginCluster += ui64LastBeginCluster;

    return TRUE;
}

BOOL CNTFSHelper::_GetOneFileAttrInfoByDataRunBuffer(const PBYTE pDataRunBuffer, const UINT& uiIndexSPos, FileAttrInfo& fileAttrInfo)
{
    // 1.�Ƿ���Ŀ¼
    UINT64 ui64AttrSign = 0;
    memcpy(&ui64AttrSign, &pDataRunBuffer[uiIndexSPos + 0x48], 8);
    // ���ж��ǲ���ϵͳ�������ļ���ϵͳ�ļ�����ʾ������¼
    if (ui64AttrSign == 0 || (ui64AttrSign & 0x0006) == 0x0006)
    {
        return FALSE;
    }
    fileAttrInfo.bIsDir = (ui64AttrSign & 0x10000000) == 0x10000000;

    // 2.�ļ��ο���(����ֶ�����8���ֽڣ����Ǻ������ֽ�ʱ���кţ�ֻ��ǰ��λ�Ϳ���)
    memcpy(&fileAttrInfo.ui64FileUniNum, &pDataRunBuffer[uiIndexSPos], 6);
    if (fileAttrInfo.ui64FileUniNum == 0)
    {
        return FALSE;
    }

    // 3.����ʱ��
    UINT64 ui64FileCreateTime = 0;
    memcpy(&ui64FileCreateTime, &pDataRunBuffer[uiIndexSPos + 0x18], 8);
    if (!_TimeFromRecordToSystemTime(ui64FileCreateTime, fileAttrInfo.stFileCreateTime))
    {
        return FALSE;
    }

    // 4.�޸�ʱ��
    UINT64 ui64FileModifyTime = 0;
    memcpy(&ui64FileModifyTime, &pDataRunBuffer[uiIndexSPos + 0x20], 8);
    if (!_TimeFromRecordToSystemTime(ui64FileModifyTime, fileAttrInfo.stFileModifyTime))
    {
        return FALSE;
    }

    // 5.ʵ�ʴ�С
    memcpy(&(fileAttrInfo.ui64FileSize), &pDataRunBuffer[uiIndexSPos + 0x40], 8);

    // 6.�ݹ��ȡȫ·��
    BYTE recordBuffer[ONE_FILE_RECORD_SIZE + 1];
    if (_GetFileRecordByFileRefNum(fileAttrInfo.ui64FileUniNum, recordBuffer))
    {
        return _AutoGetFullPath(recordBuffer, fileAttrInfo.strFilePath);
    }

    return TRUE;
}

BOOL CNTFSHelper::_GetFileDataByDataRun(const std::vector<DataInfo>& vecDataRunList, PBYTE pFileData)
{
    // �ȼ���£����ķ�Χ���ܳ���4m
    UINT64 ui64FileDataSize = 0;
    for (auto datainfo : vecDataRunList)
    {
        ui64FileDataSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
    }
    if (ui64FileDataSize > ONE_FILE_BLOCK_SIZE)
    {
        return FALSE;
    }

    // ��ʽ��ʼ��
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

    // У��һ�����ݳ���
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

    //��ȡ��Ӧ�߼����������
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

    // �Ȱ��ļ��ĳ������ֻ�����ļ��еģ��ٰ��ļ��Ĳ��ȥ
    // ��ȥ��
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
    // ����׷��д���ȴ������ļ�
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

    // �򿪻򴴽��ļ���Ҳ�Ӹ�����
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
    // д�ļ���ʧ�����ÿ0.2������һ�Σ�����5�Σ�һֱ���ɹ�����ֱ�ӱ���
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

            // �п����ж��80������ѭ��
            memcpy(&uiAttrLength, &pRecordBuffer[ui20AttrSPos + 0x04], 2);
            ui20AttrSPos += uiAttrLength;
        }
    }

    return !vecDataRunInfos.empty();
}

BOOL CNTFSHelper::MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath)
{
    // 1.��ȡԴ�ļ��ļ���¼
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    if (_GetFileRecordByFileRefNum(ui64SrcFileNum, buffer))
    {
        // 2.����Դ�ļ���С���ж��費��Ҫ�ֿ��д�����ֿ��ֱ�ӿ��ٿռ䣬һ���Զ���������һ����д��Ŀ��·��
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
            // �ֿ��д����Ҫ����һ������ڴ棬��һ��ӿڻ��Լ�ά��
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
    // 1.���ݵ�ǰdir�ļ��ο��Ż�ȡ��Ӧ�ļ���¼
    BYTE buffer[ONE_FILE_RECORD_SIZE + 1];
    ZeroMemory(buffer, ONE_FILE_RECORD_SIZE + 1);
    if (_GetFileRecordByFileRefNum(ui64ParentRefNum, buffer))
    {
        UINT uiAttrSPos = 0;
        UINT uiAttrLength = 0;
        BOOL bHave20HAttr = FALSE;
        UINT uiDirNumIn20HAttr = 0;
        std::vector<FileAttrInfo> vecChildInfosIn20Attr;
        // �ȿ���û��20���ԣ��еĻ�����ȥ20��������Ѷ�Ӧ��A0��90��������ó���
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
        // ��һ����������������ȿ�����û��A0H���ԣ��о�ֱ�ӷ���A0��û�о�ֻ����90����
        if (_FindAnyAttrSPosByFileRecord(buffer, 0xA0, uiAttrSPos, uiAttrLength))
        {
            // ����datarun���õ�ȫ����������ʼλ�ú�ռ�ó��ȣ���λ���أ�
            std::vector<DataInfo> vecDataRunLists;
            if (CNTFSHelper::GetInstance()->_GetA0HAttrDataRunLists(buffer, vecDataRunLists))
            {
                // ��datarun��ȡ��������
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
            // 90������������������棬ֱ���þ���
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

    // ��30���Զ����ο���
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
        // 1.��ȡA0���Գ���-datarun��ʼλ��=datarun���� ���������ܳ����������
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
    // 1.��ȡA0������ʼλ��-datarun��ʼλ��=datarun���� ���������ܳ����������
    UINT ui90AttrSPos = 0;
    UINT ui90AttrLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pRecordBuffer, 0x90, ui90AttrSPos, ui90AttrLength))
    {
        UINT uiFirstIndexSPos = ui90AttrSPos + 0x40;
        UINT uiIndexSize = ui90AttrSPos + ui90AttrLength;

        // ��������������õ������Ϣ
        while (uiFirstIndexSPos < uiIndexSize)
        {
            unsigned short usOneIndexSize = 0;
            memcpy(&usOneIndexSize, &pRecordBuffer[uiFirstIndexSPos + 0x08], 2);

            // ����������
            FileAttrInfo attrInfo;
            if (_GetOneFileAttrInfoByDataRunBuffer(pRecordBuffer, uiFirstIndexSPos, attrInfo))
            {
                vecFileAttrLists.push_back(attrInfo);
            }

            // ƫ�Ƶ��¸�������ʼλ�ü���ѭ��
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
                // �Ƚ�������ͷ���õ���һ���������ƫ�ƺ������ܴ�С�������ܴ�С����Ϊ��Ч������
                UINT uiSPos = i * ONE_CLUSTER_SIZE;
                UINT uiIndexOffset = 0;
                memcpy(&uiIndexOffset, &pBuffer[uiSPos + 0x18], 4);

                UINT uiIndexSize = 0;
                memcpy(&uiIndexSize, &pBuffer[uiSPos + 0x1C], 4);

                // ��������������õ������Ϣ
                UINT uiIndexSPos = uiSPos + uiIndexOffset + 0x18;
                while (uiIndexSPos < uiIndexSize + uiSPos)
                {
                    unsigned short usOneIndexSize = 0;
                    memcpy(&usOneIndexSize, &pBuffer[uiIndexSPos + 0x08], 2);

                    // ����������
                    FileAttrInfo attrInfo;
                    if (_GetOneFileAttrInfoByDataRunBuffer(pBuffer, uiIndexSPos, attrInfo))
                    {
                        vecChildAttrInfos.push_back(attrInfo);
                    }

                    // ƫ�Ƶ��¸�������ʼλ�ü���ѭ��
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
    // 1.������ͷ�ж�80�����ǲ��ǳ�פ����פ���ļ���¼������ݣ��ǳ�פҪȥ��datarun
    UINT ui80AttrSPos = 0;
    UINT ui80AttrLength = 0;
    if (_FindAnyAttrSPosByFileRecord(pFileRecordBuffer, 0x80, ui80AttrSPos, ui80AttrLength))
    {
        BYTE byFlag = 2;
        memcpy(&byFlag, &pFileRecordBuffer[ui80AttrSPos + 0x08], 1);
        BOOL bPermanent = byFlag == 0;

        // 2.�ȴ���פ��ֱ�Ӷ��ļ���¼���������
        if (bPermanent)
        {
            // �����ܳ���
            UINT uiAttrLength = 0;
            memcpy(&uiAttrLength, &pFileRecordBuffer[ui80AttrSPos + 0x04], 4);

            // �����壨�ļ���ʵ���ݣ�����
            UINT uiAttrBodyLength = uiAttrLength - 0x18;

            if (pFileDataBuffer)
            {
                memcpy(pFileDataBuffer, &pFileRecordBuffer[ui80AttrSPos + 0x18], uiAttrBodyLength);
            }

            return 1;
        }
        else
        {
            // �ǳ�פ����Ҫȥ��datarunλ�ã�Ȼ����ļ���¼��ľ���λ�ö����ݣ���������漰���ļ���ʵ����
            // �޴�����⣬���Կ�����Ҫ�ֿ��
            // ���ﻹҪ����20���Ե����⣬���ж�����û��20���ԣ�����У��Ȱ������80��Ӧ���ļ���¼�����80�����е�datarun�ó�����һ��
            // ���뱾�ļ���¼��80���������datarun�ϲ���������ڣ��ϲ���ʽΪ׷�ӣ�
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
        // û��80���ԣ�˵���ļ���datarunȫ��20����
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
    // ��ʽ��ʼ��
    UINT64 ui64AllFileSize = 0;
    for each (auto datainfo in vecDataRunList)
    {
        ui64AllFileSize += datainfo.ui64UsedSclusters * ONE_CLUSTER_SIZE;
    }

    // ��¼һ�·�������������С���ļ�ʵ�ʴ�С�Ĳ�ֵ����λ���ֽ���������д�ļ���ʱ���βҪ������
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
                // ���һ���ʱ��д��ʱ��Ҫ����֮ǰ����Ľ�β�����λ
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
                    // ���һ���ʱ��д��ʱ��Ҫ����֮ǰ����Ľ�β�����λ
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

            // �����½��ȣ�post��ui
            dPercent += (double)(ui64ReadSize * 100.00 / ui64AllFileSize);
            // POST
            if (::IsWindow(m_hProgressWnd))
            {
                ::SendMessage(m_hProgressWnd, MSG_UPDATE_PROGRESS, (WPARAM)dPercent, 0);
            }
        }
        // ÿһ��datarun���궼У��һ��
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
    // �ļ����԰�������ͷ�������壬����ͷûɶ�ã�������ƫ����ʼλ�õ���������ʼλ��
    UINT uiAttrBodySPos = ui30HSPos + 0x18;

    // 1.�ļ�������
    BYTE fileNameSize = 0;
    memcpy(&fileNameSize, &pRecordBuffer[uiAttrBodySPos + 0x40], 1);

    // 2.�ļ���
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
    // �ļ����԰�������ͷ�������壬����ͷûɶ�ã�������ƫ����ʼλ�õ���������ʼλ��
    UINT uiAttrBodySPos = ui30HSPos + 0x18;

    // 1.��Ŀ¼�ο���
    memcpy(&ui64ParentFileNum, &pRecordBuffer[uiAttrBodySPos], 6);

    return ui64ParentFileNum != 0;
}

BOOL CNTFSHelper::_AutoGetFullPath(const PBYTE pRecordBuffer, CString& strPath)
{
    // 1.��ȡ��ǰ�ļ����ļ���
    CString strCurFileName;
    if (_GetFileNameByFileRecord(pRecordBuffer, strCurFileName))
    {
        // ����ļ���Ϊ��.��˵���Ǹ�Ŀ¼��ֱ�ӷ���
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

        // 2.��ȡ��Ŀ¼�ļ��ο���
        UINT64 ui64ParentFileNum = 0;
        if (_GetParentFileNumByFileRecord(pRecordBuffer, ui64ParentFileNum))
        {
            // �����Ŀ¼�ļ��ο�����5��˵���Ѿ��Ǹ�Ŀ¼�ˣ�ƴ���̷�����
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
                // 3.��ȡ��Ŀ¼�ļ���¼
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
