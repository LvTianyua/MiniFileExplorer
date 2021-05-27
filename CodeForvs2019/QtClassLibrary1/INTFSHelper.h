#pragma once
#include "common.h"

class INTFSHelper
{
public:

    /**
    *
    *  设置当前磁盘信息
    * @param strDriverName（IN） 磁盘名称，eg：C,D,E...
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual BOOL SetCurDriverInfo(const CString& strDriverName) = 0;

    /**
    *
    *  获取当前磁盘名称
    *
    * @return 磁盘名称
    */
    virtual CString GetCurDriverName() = 0;

    /**
    *
    *  获取所有逻辑驱动器名称集合
    *
    * @return 当前计算机下逻辑磁盘名称集合
    */
    virtual std::vector<CString> GetAllLogicDriversNames() = 0;

    /**
    *
    *  Ntfs拷贝，通过源文件参考号以及源文件真实大小，拷贝到目标路径（绝对路径）
    * @param ui64SrcFileNum（IN） 源文件文件参考号
    * @param ui64SrcFileSize（IN） 源文件文件真实大小
    * @param strDestPath（IN） 目标文件路径
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual BOOL MyCopyFile(const UINT64& ui64SrcFileNum, const UINT64& ui64SrcFileSize, const CString& strDestPath) = 0;

    /**
    *
    *  Ntfs根据当前目录参考号，获取全部需要显示的子项集合
    * @param ui64ParentRefNum（IN） 父目录参考号
    * @param vecChildAttrInfos（OUT） 子项文件属性集合
    * @param uiDirNum（OUT） 子项中目录的数量
    * @param bForceFresh（IN） 是否强制刷新
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual BOOL GetAllChildInfosByParentRefNum(const UINT64& ui64ParentRefNum, std::vector<FileAttrInfo>& vecChildAttrInfos, UINT& uiDirNum, BOOL bForceFresh) = 0;

    /**
    *
    *  根据子文件参考号获取父文件参考号
    * @param ui64FileNum（IN） 文件参考号
    * @param ui64ParentFileNum（OUT） 父目录参考号
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual BOOL GetParentFileNumByFileNum(const UINT64& ui64FileNum, UINT64& ui64ParentFileNum) = 0;

    /**
    *
    *  根据文件参考号获取文件路径
    * @param ui64FileNum（IN） 文件参考号
    * @param strFilePath（OUT） 文件路径
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual BOOL GetFilePathByFileNum(const UINT64& ui64FileNum, CString& strFilePath) = 0;

    /**
    *
    *  设置进度条窗口句柄
    * @param hProgressWnd（IN） 进度窗口句柄
    *
    * @return 成功 TRUE 失败 FALSE
    */
    virtual void SetProgressWndHandle(const HWND& hProgressWnd) = 0;

    /**
    *
    *  取消拷贝任务
    *
    */
    virtual void CancelCopyTask() = 0;

    /**
    *
    *  获取上一次拷贝失败任务是不是因为取消失败的
    *
    * @return 是 TRUE 否 FALSE
    */
    virtual BOOL IsCopyTaskByCancel() = 0;

    /**
    *
    *  重置拷贝任务取消状态的标志
    *
    */
    virtual void ResetCopyTaskFlag() = 0;
};

