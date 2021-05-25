#include "TableModel.h"
#include "NTFSHelper.h"
#include "MyTableView.h"

QTableModel::QTableModel(QObject *parent /*= nullptr*/)
    : QAbstractTableModel(parent)
{
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
    FileAttrInfo attrInfo;
    if (!index.isValid() || !GetAttrInfoByIndex(index, attrInfo))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
        {
            CString strPath = attrInfo.strFilePath;
            strPath = PathFindFileName(strPath);
            return CNTFSHelper::CStringToQString(strPath);
        }
        case 1:
        {
            return TimeToString(attrInfo.stFileModifyTime);
        }
        case 2:
        {
            return attrInfo.ui64FileUniNum == 5 ? u8"NTFS卷" : (attrInfo.bIsDir ? u8"文件夹" : GetFileTypeNameByFilePath(attrInfo.strFilePath));
        }
        case 3:
        {
            return attrInfo.bIsDir ? "" : SizeToString(attrInfo.ui64FileSize);
        }
        default:
            break;
        }
    }
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
        return GetIconByFileAttr(attrInfo);
    }
    else if (role == Qt::BackgroundRole)
    {
        if (index.row() == m_rowHover)
        {
            return QColor(229,243,255);
        }
    }

    return QVariant();
}

int QTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_vecAttrInfos.size();
}

int QTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 4;
}

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case 0:
            {
                return u8"文件名";
            }
            case 1:
            {
                return u8"修改日期";
            }
            case 2:
            {
                return u8"类型";
            }
            case 3:
            {
                return u8"大小";
            }
            default:
                break;
            }
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool QTableModel::GetAttrInfoByIndex(const QModelIndex& index, FileAttrInfo& attrInfo) const
{
    if (!index.isValid() || m_vecAttrInfos.empty() || m_vecAttrInfos.size() <= index.row())
    {
        return false;
    }

    attrInfo = m_vecAttrInfos[index.row()];
    return true;
}

void QTableModel::SetHoverRow(int row)
{
    m_rowHover = row;
}

QString QTableModel::TimeToString(const SYSTEMTIME& systemTime) const
{
    if (systemTime.wYear == 0 && systemTime.wMonth == 0 && systemTime.wDay == 0 && systemTime.wHour == 0 && systemTime.wMinute == 0)
    {
        return QString();
    }
    CString strTmp;
    strTmp.Format(L"%02d/%02d/%02d %02d:%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute);
    return CNTFSHelper::CStringToQString(strTmp);
}

QString QTableModel::SizeToString(const UINT64& ui64FileSize) const
{
    QString strSize;
    UINT64 ui64KBSize = ui64FileSize / 1024;
    if (ui64FileSize % 1024 > 0)
    {
        ++ui64KBSize;
    }
    return strSize.setNum(ui64KBSize) + u8" KB";
}

QIcon QTableModel::GetIconByFileAttr(const FileAttrInfo& attrInfo) const
{
    if (attrInfo.ui64FileUniNum == 0)
    {
        return QIcon(":/QtWidgetsApplication1/res/computer.png");
    }
    else if (attrInfo.ui64FileUniNum == 5)
    {
        TCHAR sysDir[MAX_PATH + 1];
        ::GetSystemDirectory(sysDir, sizeof(sysDir));
        CString sysDisk(sysDir[0]);
        if (sysDisk.CompareNoCase(attrInfo.strFilePath) == 0)
        {
            return QIcon(":/QtWidgetsApplication1/res/driversys.png");
        }
        else
        {
            return QIcon(":/QtWidgetsApplication1/res/driver.png");
        }
    }
    else
    {
        if (attrInfo.bIsDir)
        {
            return QIcon(":/QtWidgetsApplication1/res/dir.png");
        }
        else
        {
            SHFILEINFO infoFile;
            SHGetFileInfo(attrInfo.strFilePath,
                FILE_ATTRIBUTE_NORMAL,
                &infoFile,
                sizeof(infoFile),
                SHGFI_ICON);

            QIcon icon(QtWin::fromHICON(infoFile.hIcon));
            ::DestroyIcon(infoFile.hIcon);
            return icon;
        }
    }
}

QString QTableModel::GetFileTypeNameByFilePath(const CString& strFilePath) const
{
    SHFILEINFO infoFile;
    SHGetFileInfo(strFilePath,
        FILE_ATTRIBUTE_NORMAL,
        &infoFile,
        sizeof(infoFile),
        SHGFI_TYPENAME);

    return QString::fromWCharArray(infoFile.szTypeName);
}