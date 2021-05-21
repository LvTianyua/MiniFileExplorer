#include "QtWidgetsApplication1.h"
#include "stdafx.h"
#include "common.h"
#include "Dbghelp.h"
#pragma comment(lib, "Dbghelp.lib")

LONG ExceptionCrashHandler(EXCEPTION_POINTERS* pException)
{
    // 创建Dump文件
    CString strFileName;
    DWORD dwTicket = GetTickCount();
    strFileName.Format(L"Exception_%u.dmp", dwTicket);
    HANDLE hDumpFile = CreateFile(strFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    // Dump信息
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;
    // 写入Dump文件内容
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    CloseHandle(hDumpFile);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    //抓捕异常文件
    ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionCrashHandler);  //cash代码

    QApplication a(argc, argv);
    QtWidgetsApplication1 w;
    w.show();
    return a.exec();
}
