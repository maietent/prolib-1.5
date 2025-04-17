#include "stdafx.h"
#include "MBMatchServer.h"
#include <MCrashDump.h>
#include <MMatchConfig.h>
#include <thread>
#include <iostream>

#define SUPPORT_EXCEPTIONHANDLING

static bool GetRecommandLogFileName(char* pszBuf)
{
	if (PathIsDirectory("Log") == FALSE)
		CreateDirectory("Log", NULL);

	time_t		tClock;
	struct tm* ptmTime;

	time(&tClock);
	ptmTime = localtime(&tClock);

	char szFileName[_MAX_DIR];

	int nFooter = 1;
	while (TRUE) {
		sprintf(szFileName, "Log/MatchLog_%02d-%02d-%02d-%d.txt",
			ptmTime->tm_year + 1900, ptmTime->tm_mon + 1, ptmTime->tm_mday, nFooter);

		if (PathFileExists(szFileName) == FALSE)
			break;

		nFooter++;
		if (nFooter > 100) return false;
	}
	strcpy(pszBuf, szFileName);
	return true;
}

int main()
try
{
    char szModuleFileName[_MAX_DIR] = { 0, };
    GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
    PathRemoveFileSpec(szModuleFileName);
    SetCurrentDirectory(szModuleFileName);

    char szLogFileName[_MAX_DIR];
    if (GetRecommandLogFileName(szLogFileName) == false)
        return FALSE;

    InitLog(
        MLOGSTYLE_DEBUGSTRING |
        MLOGSTYLE_FILE, szLogFileName);

#ifdef SUPPORT_EXCEPTIONHANDLING
    char szDumpFileName[_MAX_DIR];
    strcpy(szDumpFileName, szLogFileName);
    strcat(szDumpFileName, ".dmp");

    char szDmpFileName[MAX_PATH] = { 0, };
    strcpy(szDmpFileName, &szDumpFileName[4]);

    char szTxtFileName[MAX_PATH] = { 0, };
    strcpy(szTxtFileName, &szLogFileName[4]);

    try
    {
#endif
        auto* m_pMatchServer = new MBMatchServer;

        if (m_pMatchServer->Create(6000)) {
            for (;;) {
                m_pMatchServer->Run();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        else {
            mlog("MBMatchServer::Create() failed");
            return -1;
        }
#ifdef SUPPORT_EXCEPTIONHANDLING
    }
    catch (...)
    {
        CrashExceptionDump(nullptr, szDumpFileName, true);
        MMatchServer::GetInstance()->CheckMemoryTest();
        MGetServerStatusSingleton()->Dump();

        char szScriptName[MAX_PATH] = { 0, };
        wsprintf(szScriptName, "Log\\UpLoadScript.txt");
        FILE* fp = fopen(szScriptName, "w+");
        if (fp != NULL)
        {
            char szServerID[MAX_PATH] = { 0, };
            sprintf(szServerID, "%d", MGetServerConfig()->GetServerID());
            char szLogDir[MAX_PATH] = { 0, };
            strcpy(szLogDir, "C:\\GunzServer\\MatchServer\\Log");

            fprintf(fp, "user\nserver\nshsrhfxkd\nbinary\ncd LogFile\n");
            fprintf(fp, "lcd %s\n", szLogDir);
            fprintf(fp, "put %s_%s\n", szServerID, szDmpFileName);
            fprintf(fp, "put %s_%s\nbye\n", szServerID, szTxtFileName);
            fclose(fp);

            char temp_arg[256] = { 0, };
            wsprintf(temp_arg, "%s %s %s", szServerID, szDmpFileName, szTxtFileName);

            ShellExecute(NULL, _T("open"), _T("UpLoadLogFile.bat"), _T(temp_arg), NULL, SW_HIDE);
        }
    }
#endif

    return 0;
}
catch (std::runtime_error& e)
{
    mlog("Exception: %s", e.what());
    throw;
}
