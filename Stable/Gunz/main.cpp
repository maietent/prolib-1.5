#include "stdafx.h"
//#include "../MatchServer/vld/vld.h"


#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

//#include "AntiShotbot.h"

//#include "AnticheatMain.h"

#include "dxerr.h"

#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"
#include "HMAC_SHA1.h"

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

//#include "mempool.h"
#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"
#include "SecurityTest.h"
#include "CheckReturnCallStack.h"
#include "ZGameInput.h"

#include "../sdk/md5/md5.h"

//#include "ACMain.h"

RMODEPARAMS	g_ModeParams={800,600,true,D3DFMT_X8R8G8B8};

#define SUPPORT_EXCEPTIONHANDLING

#ifdef LOCALE_NHNUSA
#include "ZNHN_USA.h"
#include "ZNHN_USA_Report.h"
#include "ZNHN_USA_Poll.h"
#endif

//void ZPerformHooks();

RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];// = "ReleaseDate : 12/22/2003";

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;

HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

#include <iostream>
#include <fstream>

void CreateConsole()
{
	if (AllocConsole())
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);

		freopen_s(&fp, "CONIN$", "r", stdin);

		SetConsoleTitle("MLog Output");

		system("cls");

		std::cout << "MLog console initialized.\n";
	}
	else
	{
		std::cerr << "Failed to allocate console.\n";
	}
}

void CloseConsole()
{
	FreeConsole();
}

void zexit(int returnCode)
{
	// 게임가드는 제대로 delete되어야 오류발생시 자체 로그를 올바르게 남길 수 있다.
	// 그냥 exit()해도 ZGameGuard를 싱글턴으로 만들었기 때문에 소멸자에서 게임가드가 delete되지만 어째서인지 그때 크래시가 일어난다.
	// exit()하기 전에 게임가드를 수동으로 해제하면 그런 문제가 일어나지 않는다.
	// 해킹 검출 등의 이유로 클라이언트 종료시 exit하지말고 zexit를 쓰자.
	exit(returnCode);
}

void CrcFailExitApp() { 
#ifdef _PUBLISH
	PostMessage(g_hWnd, WM_CLOSE, 0, 0); 
#else
	int* crash = NULL;
	*crash = 0;
#endif
}

void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

//list<HANDLE>	g_FontMemHandles;
static bool bFirstLoad = true;

RRESULT OnCreate(void *pParam)
{
	g_App.PreCheckArguments();

	//Custom: skip loading dynamic models if config is set to true
	if (Z_VIDEO_DYNAMIC_MODELS == true)
	{
		RMesh::SetPartsMeshLoadingSkip(1);
	}
	string strFileLenzFlare("System/LenzFlare.xml");
	RCreateLenzFlare(strFileLenzFlare.c_str());
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");

	sprintf_s( cstrReleaseDate, "GunZ - %s", ZGetSVNRevisionSTRING().c_str());
	mlog(cstrReleaseDate); mlog("\n");
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);

	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(  1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768 );

	mlog("InitialLoading success.\n");

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR"*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, DEFAULT_FONT_HEIGHT, 1.0f) )

	{
		mlog("Failed to create the default font\n");
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}


	g_pDC = new MDrawContextR2(RGetDevice());

#ifndef _FASTDEBUG
	if( ZGetInitialLoading()->IsUseEnable() )
	{
		ZGetInitialLoading()->AddBitmap(0, "Interface/default/loading/loading_adult.jpg");
		ZGetInitialLoading()->AddBitmapBar( "Interface/default/loading/loading.bmp" );

		ZGetInitialLoading()->SetPercentage( 0.0f );
		ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true );
	}
#endif
	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("Interface initialized\n");

	ZLoadingProgress appLoading("application");
	if(!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);


	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);


    
	// Default Key
	for(int i=0; i<ZACTION_COUNT; i++)
	{
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if(keyDesc.nVirtualKeyAlt!=-1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

//	ParseParameter(g_szCmdLine);

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	static const char *szDone = "Done.";
	ZGetInitialLoading()->SetLoadingStr(szDone);
	if( ZGetInitialLoading()->IsUseEnable() )
	{
#ifndef _FASTDEBUG
		ZGetInitialLoading()->SetPercentage( 100.f );
		ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 ,true  );
#endif
		ZGetInitialLoading()->Release();
	}

	mlog("main : OnCreate() done\n");

	SetFocus(g_hWnd);

	bFirstLoad = false;


	return R_OK;
}


bool CheckDll(char* fileName, BYTE* SHA1_Value)
{
	BYTE digest[20];
	BYTE Key[GUNZ_HMAC_KEY_LENGTH];

	memset(Key, 0, 20);
	memcpy(Key, GUNZ_HMAC_KEY, strlen(GUNZ_HMAC_KEY));

	CHMAC_SHA1 HMAC_SHA1 ;
	HMAC_SHA1.HMAC_SHA1_file(fileName, Key, GUNZ_HMAC_KEY_LENGTH, digest) ;

	if(memcmp(digest, SHA1_Value, 20) ==0)
	{
		return true;
	}

	return false;
}



RRESULT OnDestroy(void *pParam)
{
	mlog("Clean-up triggered.\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("Interface finalized\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("Game input destroyed\n");

	RGetShaderMgr()->Release();
	ZGetConfiguration()->Destroy();

	mlog("Game configuration destroyed\n");

	delete g_pDC;

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR"*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MFontManager::Destroy();
	mlog("MFontManager destroyed\n");
	MBitmapManager::Destroy();
	mlog("Bitmap manager destroyed\n");
	MBitmapManager::DestroyAniBitmap();
	mlog("Animation bitmap destroyed\n");

	ZBasicInfoItem::Release();

	ZGetStencilLight()->Destroy();
	mlog("Stencil light destroyed\n");
	LightSource::Release();
	mlog("Light source released\n");

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	mlog("LenzFlare destroyed\n");
	RAnimationFileMgr::GetInstance()->Destroy();
	mlog("Animation file manager destroyed\n");
	
	ZStringResManager::ResetInstance();
	mlog("String resource manager reset\n");

	mlog("Clean-up finished\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	__BP(100, "main::OnUpdate");
	g_pInput->Update();

	g_App.OnUpdate();


	return R_OK;
}

RRESULT OnUpdateInput(void* param)
{
	///TODO: decouple gameinput
	return R_OK;
}

extern MDrawContextR2* g_pDC;

RRESULT OnRender(void* pParam)
{
	__BP(101, "main::OnRender");
	if (!RIsActive() && RIsFullScreen())
	{
		__EP(101);
		return R_NOTREADY;
	}

	g_App.OnDraw();

	if (ZIsActionKeyPressed(ZACTION_SCREENSHOT)) {
		if (g_App.GetGameInterface())
			g_App.GetGameInterface()->GetBandiCapturer()->CaptureImage();// ->SaveScreenShot();
	}

	if (ZIsActionKeyPressed(ZACTION_MOVING_PICTURE))
	{	// 동영상 캡쳐...2008.10.02
		if (g_App.GetGameInterface())
			g_App.GetGameInterface()->GetBandiCapturer()->ToggleStart();
	}

	if (ZGetGameInterface()->GetBandiCapturer() != NULL)
		ZGetGameInterface()->GetBandiCapturer()->DrawCapture(g_pDC);

	/* CUSTOM FPS Counter */

	char szBuffer[64];
	sprintf_s(szBuffer, sizeof(szBuffer), "FPS: %i", static_cast<int>(std::round(1000.f / g_fFPS)));
	MFont* pFont = ZCombatInterface::GetGameFont();
	pFont = MFontManager::Get("FONTa10_O2Wht");
	g_pDC->SetFont(pFont);
	g_pDC->SetColor(MCOLOR(0xFFffffff));
	g_pDC->Text((g_pDC->GetClipRect().w - g_pDC->GetFont()->GetWidth(szBuffer) - 10), 10, szBuffer);

	__EP(101);

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	MBitmapR2::m_dwStateBlock=NULL;

	g_App.OnInvalidate();
	
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
			pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

RRESULT OnActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);
	return R_OK;
}

RRESULT OnDeActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);
	return R_OK;
}

RRESULT OnError(void *pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
		{
			D3DADAPTER_IDENTIFIER9 *ai=RGetAdapterID();
			char szLog[512];
			ZTransMsg( szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret!=IDYES)
				return R_UNKNOWN;
		}
		break;
	case RERROR_CANNOT_CREATE_D3D:
		{
			ShowCursor(TRUE);

			char szLog[512];
			sprintf(szLog, ZMsg( MSG_DIRECTX_NOT_INSTALL));

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret==IDYES)
			{
				ShellExecute(g_hWnd, "open", ZMsg( MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL); 
			}
		}
		break;

	};

	return R_OK;
}

void SetModeParams()
{
	g_ModeParams.bFullScreen = ZGetConfiguration()->GetVideo()->bFullScreen;
	g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
	g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8;

}

void ResetAppResource()
{
	GunzState state = ZGetGameInterface()->GetState();
	ZGetStringResManager()->ReInit();

	ZGetConfiguration()->Load_StringResDependent();

	g_App.InitLocale(true);
	g_App.ReInitializeGameInterface();
	g_App.ReInitializeStageOptionInterface();
	ZGetGameInterface()->SetFocusEnable(true);
	ZGetGameInterface()->SetFocus();
	ZGetGameInterface()->Show(true);
	MGetMatchItemDescMgr()->Clear();
	if (!MGetMatchItemDescMgr()->ReadCache())
	{
		if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ZITEM_DESC,ZGetFileSystem()))
		{
			MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEM_DESC);
		}
		if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ZITEM_DESC_LOCALE ,ZGetFileSystem()))
		{
			MLog("Error while Read Item Descriptor %s\n", FILENAME_ZITEM_DESC_LOCALE);
		}

		MGetMatchItemDescMgr()->WriteCache();
	}
	ZGetEmblemInterface()->Destroy();
	ZGetEmblemInterface()->Create();
	ZGetGameInterface()->SetState(state);

	ZGetGameInterface()->m_sbRemainClientConnectionForResetApp = false;
}

int FindStringPos(char* str,char* word)
{
	if(!str || str[0]==0)	return -1;
	if(!word || word[0]==0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for(int i=0;i<str_len;i++) {
		c = str[i];
		if(c == word[0]) {

			bCheck = true;

			for(int j=1;j<word_len;j++) {
				if(str[i+j]!=word[j]) {
					bCheck = false;
					break;
				}
			}

			if(bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
	if(!pName) return false;

	FILE *fp;
	fp = fopen( "mlog.txt", "r" );
	if(fp==NULL)  return false;

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pBuffer = new char [size];
	pBuffer[size] = 0;

	fread(pBuffer,1,size,fp);

	fclose(fp);

	int posSource = FindStringPos(pBuffer,"ublish");
	if(posSource==-1) return false;

	int posA = FindStringPos(pBuffer+posSource,"Function Name");
	int posB = posA + FindStringPos(pBuffer+posSource+posA,"\n");

	if(posA==-1) return false;
	if(posB==-1) return false;

	posA += 16;

	int memsize = posB-posA;
	memcpy(pName,&pBuffer[posA+posSource],memsize);

	pName[memsize] = 0;

	delete [] pBuffer;

	for(int i=0;i<memsize;i++) {
		if(pName[i]==':') {
			pName[i] = '-';
		}
	}

	return true;
}

void HandleExceptionLog()
{
	#define ERROR_REPORT_FOLDER	"ReportError"

	extern char* logfilename;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(ERROR_REPORT_FOLDER, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory("ReportError", NULL)) {
			MessageBox(g_hWnd, "ReportError 폴더를 생성할 수 없습니다.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
			return;
		}
	} else 	{
		FindClose(hFind);
	}

	if(ZGetCharacterManager()) {
		ZGetCharacterManager()->OutputDebugString_CharacterState();
	}

	ZGameClient* pClient = (ZGameClient*)ZGameClient::GetInstance();

	char* pszCharName = NULL;
	MUID uidChar;
	MMatchObjCache* pObj;
	char szPlayer[128];

	if( pClient ) {

		uidChar = pClient->GetPlayerUID();
		pObj = pClient->FindObjCache(uidChar);
		if (pObj)
			pszCharName = pObj->GetName();

		wsprintf(szPlayer, "%s(%d%d)", pszCharName?pszCharName:"Unknown", uidChar.High, uidChar.Low);
	}
	else { 
		wsprintf(szPlayer, "Unknown(-1.-1)");
	}

	if (pClient) {

		time_t currtime;
		time(&currtime);
		struct tm* pTM = localtime(&currtime);

		char cFuncName[1024];

		if(FindCrashFunc(cFuncName)==false) {
			strcpy(cFuncName,"Unknown Error");
		}

		char szFileName[_MAX_DIR], szDumpFileName[_MAX_DIR];
		wsprintf(szFileName, "%s_%s_%.2d%.2d_%.2d%.2d_%s_%s", cFuncName,
				APPLICATION_NAME, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "mlog.txt");
		wsprintf(szDumpFileName, "%s.dmp", szFileName);

		char szFullFileName[_MAX_DIR], szDumpFullFileName[_MAX_DIR];
		wsprintf(szFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szFileName);
		wsprintf(szDumpFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szDumpFileName);

		if (CopyFile("mlog.txt", szFullFileName, TRUE))
		{
			CopyFile("Gunz.dmp", szDumpFullFileName, TRUE);

			 //BAReport 실행
			char szCmd[4048];
			char szRemoteFileName[_MAX_DIR], szRemoteDumpFileName[_MAX_DIR];
			wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szFileName);
			wsprintf(szRemoteDumpFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szDumpFileName);

			wsprintf(szCmd, "BAReport app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s,%s;remotefile=%s,%s", 
				APPLICATION_NAME, ZGetConfiguration()->GetBAReportAddr(), szPlayer, szFullFileName, szDumpFullFileName, szRemoteFileName, szRemoteDumpFileName);

			WinExec(szCmd, SW_SHOW);

			FILE *file = fopen("bareportpara.txt","w+");
			fprintf(file,szCmd);
			fclose(file);
		}
	}

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SYSCHAR:
			if(ZIsLaunchDevelop() && wParam==VK_RETURN)
			{
#ifndef _PUBLISH
				RFrame_ToggleFullScreen();
#endif
				return 0;
			}
			break;

		case WM_CREATE:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_DESTROY:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_SETCURSOR:
		{
			if (ZApplication::GetGameInterface())
				ZApplication::GetGameInterface()->OnResetCursor();
			return TRUE; // prevent Windows from setting cursor to window class cursor
		}break;
		case WM_ENTERIDLE:
			RFrame_UpdateRender();
			break;
		case WM_KEYDOWN:
			{
				bool b = false;
		}break;
		case WM_DPICHANGED:
		{
			const int iDpi = GetDpiForWindow(g_hWnd);
			const int dpiScaledWidth = MulDiv(RGetScreenWidth(), iDpi, 96);
			const int dpiScaledHeight = MulDiv(RGetScreenHeight(), iDpi, 96);

			RAdjustWindow(&g_ModeParams);

			g_pDefFont->Destroy();

			if (g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, DEFAULT_FONT_HEIGHT, 1.0f) == false)
			{
				mlog("Fail to Create defualt font : MFontR2 / main.cpp.. onCreate\n");
				g_pDefFont->Destroy();
				SAFE_DELETE(g_pDefFont);
				g_pDefFont = NULL;
			}
		}break;
		case WM_SYSCOMMAND:
		{
		}
		break;
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
	{
		if (ZGetGameInterface() && ZGetGameInterface()->IsReservedResetApp())	// for language changing
		{
			ZGetGameInterface()->ReserveResetApp(false);
			ResetAppResource();
		}

		return 0;
	}

	// thread safe하기위해 넣음
	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool CheckFileList()
{
	MZFileSystem *pfs = ZApplication::GetFileSystem();
	MZFile mzf;

	rapidxml::xml_document<> doc;

	string strFileNameFillist(FILENAME_FILELIST);
	if (!mzf.Open(strFileNameFillist.c_str(), pfs))
		return false;

	char *buffer = new char[mzf.GetLength() + 1];
	mzf.Read(buffer, mzf.GetLength());
	buffer[mzf.GetLength()] = 0;
	if (!doc.parse<0>(buffer)) {
		mlog("Error parsing %s", strFileNameFillist.c_str());
		return false;
	}

	rapidxml::xml_node<>* rootNode = doc.first_node();
	for (auto itor = rootNode->first_node(); itor; itor = itor->next_sibling())
	{
		if (itor->name()[0] == '#')
			continue;

		if (_stricmp(itor->name(), "FILE") == 0)
		{
			char szContents[256], szCrc32[256];
			strcpy_s(szContents, itor->first_attribute("NAME")->value());
			strcpy_s(szCrc32, itor->first_attribute("CRC32")->value());

			if (_stricmp(szContents, "config.xml") != 0)
			{
				unsigned int crc32_list = pfs->GetCRC32(szContents);
				unsigned int crc32_current;
				sscanf(szCrc32, "%x", &crc32_current);

				if (crc32_current != crc32_list)
					return false;
			}
		}
	}
	delete[] buffer;
	doc.clear();
	mzf.Close();

	return true;
}


enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//둘다없으면..

		if( _access("_Font",0) != -1) { // 이미 기록되어 있다면..
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

			if(hr==IDOK) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	// 체크해봐서 등록이 안되어있으면 등록한다. 사용자에게 물어볼수도 있겠다.
	char szValue[256];
	if(!MRegistry::Read(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = {0,};
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf(szCommand,"\"%s\" \"%%1\"",szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT,GUNZ_REPLAY_CLASS_NAME"\\shell\\open\\command",NULL,szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

HANDLE Mutex = 0;

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam);
#endif

DWORD g_dwMainThreadID;


//------------------------------------------- nhn usa -------------------------------------------------------------
bool InitReport()
{
	return true;
}

bool InitPoll()
{
	return true;
}

void InitTimeFaker() {
#ifdef _64BIT
	HMODULE hUptimeFaker = LoadLibrary("Resources/UptimeFaker/UptimeFaker64.dll");
#else
	HMODULE hUptimeFaker = LoadLibrary("Resources/UptimeFaker/UptimeFaker32.dll");
#endif
	if (!hUptimeFaker)
	{
		mlog("Failed to load UptimeFaker, continuing without..\n");
	}
	else if (hUptimeFaker) {
		mlog("Succesfully loaded UpTimeFaker!\n");
	}
}

void exitAppStuff() {
	ACExit();
	CloseConsole();
}

//------------------------------------------- nhn usa end----------------------------------------------------------
int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	/* CUSTOM Anticheat Init */
	if(ACMain() != 0)
		return 112;

	/* CUSTOM Console */
	CreateConsole();

	/* Init MLog */
	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
	mlog("GunZ: The Duel launched.\n");

	/* CUSTOM TimeFaker */
	InitTimeFaker();

	g_fpOnCrcFail = CrcFailExitApp;
	g_dwMainThreadID = GetCurrentThreadId();	

	/* Set working directory to game dir */
	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

	/* Seed RNG */
	srand( (unsigned)time( NULL ));

	/* Print date and time */
	char szDateRun[128]="";
	char szTimeRun[128]="";
	_strdate( szDateRun );
	_strtime( szTimeRun );
	mlog("Log time (%s %s)\n", szDateRun, szTimeRun);

	/* Print command line arguments, if the game ran with any */
	if(cmdline != "")
		mlog("Command Line Arguments = %s\n",cmdline);

	/* Print info about the system */
	MSysInfoLog();

	/* Gets OS Version */
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&os);

	CheckFileAssociation();

	/* Initialize MZFileSystem - MUpdate */
	MRegistry::szApplicationName=APPLICATION_NAME;
	g_App.ShiftBytesOnStart();
	g_App.InitFileSystem();

	/* Set MRS Read mode */
#if defined(_PUBLISH) && !defined(DEVELOPER_MODE)
	MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

	/* Load the config */
	ZGetConfiguration()->Load();
	
	/* Check if DXVK is enabled */
	if (Z_VIDEO_DXVK) {
		isDxvk = true;
	}

	/* Check if D3D9Ex is enabled */
	if (os.dwMajorVersion >= 6 && Z_VIDEO_D3D9EX) {
		g_isDirect3D9ExEnabled = true;
	}

	/* Init ZStringResManager and Locale */
	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Locale initialization failed\n");
		exitAppStuff();
		return false;
	}
	ZGetConfiguration()->Load_StringResDependent();

	/* I have no clue what this does im gonna be honest */
	if( !ZGetConfiguration()->LateStringConvert() )
	{
		MLog( "main.cpp - Late string convert fail.\n" );
		exitAppStuff();
		return false;
	}

	DWORD ver_major = 0;
	DWORD ver_minor = 0;
	TCHAR ver_letter = ' ';

#ifdef SUPPORT_EXCEPTIONHANDLING
	char szDumpFileName[256];
	sprintf(szDumpFileName, "Gunz.dmp");
	try{
#endif

	/* Parse command line arguments */
	if (ZApplication::GetInstance()->ParseArguments(cmdline) == false)
	{
		exitAppStuff();
		return 0;
	}

	/* Initialize notify.xml whatever that is */
	if(!InitializeNotify(ZApplication::GetFileSystem())) {
		MLog("Check notify.xml\n");
		exitAppStuff();
		return 0;
	}
	else 
	{
		mlog( "InitializeNotify ok.\n" );
	}

	/* Font stuff, and why is this comment in korean? */
	if(CheckFont()==false) {
		MLog("폰트가 없는 유저가 폰트 선택을 취소\n");
		exitAppStuff();
		return 0;
	}

	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);
	RSetFunction(RF_ACTIVATE,	OnActivate);
	RSetFunction(RF_DEACTIVATE,	OnDeActivate);
	RSetFunction(RF_ERROR,		OnError);
	RSetFunction(RF_UPDATEINPUT, OnUpdateInput);
	SetModeParams();

	/* Call RealSpace2 main */
	int nRMainReturn = RMain(APPLICATION_NAME, this_inst, prev_inst, cmdline, cmdshow, &g_ModeParams, WndProc, IDI_ICON1);
	if (0 != nRMainReturn)
	{
		mlog("error making window");
		exitAppStuff();
		return nRMainReturn;
	}

	/* Determine antialiasing from config */
	D3DMULTISAMPLE_TYPE type = D3DMULTISAMPLE_NONE;
	switch (Z_VIDEO_ANTIALIAS)
	{
	case 0:
		type = D3DMULTISAMPLE_NONE; break;
	case 1:
		type = D3DMULTISAMPLE_2_SAMPLES; break;
	case 2:
		type = D3DMULTISAMPLE_4_SAMPLES; break;
	case 3:
		type = D3DMULTISAMPLE_8_SAMPLES; break;
	}

	/* Set up stencil buffer, multisampling, update limit, and resetDevice */
	RSetStencilBuffer(Z_VIDEO_STENCILBUFFER);
	RSetMultiSampling(type);
	RSetUpdateLimitPerSecond(Z_ETC_UPDATELIMIT_PERSECOND);
	bool resetDevice = false;

	/* Initialize D3D */
	if( 0 != RInitD3D(&g_ModeParams) )
	{
		MessageBox(g_hWnd, "fail to initialize DirectX", NULL, MB_OK);
		mlog( "error init RInitD3D\n" );
		exitAppStuff();
		return 0;
	}

	/* Call RRun() */
	const int nRRunReturn = RRun();

	/* Show the window i guess */
	ShowWindow(g_hWnd, SW_MINIMIZE);

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

#ifdef _HSHIELD
	_AhnHS_StopService();
	_AhnHS_Uninitialize();		
#endif


#ifdef LOCALE_NHNUSA
	mlog( "Poll Process\n" );
	int nRetPoll = GetNHNUSAPoll().ZHanPollProcess( NULL);
#endif

	/* CLEANUP FROM HERE */

	ZStringResManager::FreeInstance();

	exitAppStuff();
	return nRRunReturn;

//	ShowCursor(TRUE);

#ifdef SUPPORT_EXCEPTIONHANDLING
	}
	catch (const std::exception& e) {
		// Handle standard exceptions
		// You can log the exception message or handle it as needed
		mlog("Standard exception: %s\n", e.what());
		exitAppStuff();
		return -1;
	}
	catch (...) {
		// Handle non-standard exceptions
		// Optionally, you can use CrashExceptionDump function here if needed
		CrashExceptionDump(nullptr, szDumpFileName, true);
		exitAppStuff();
		return -1;
	}
	//__except(MFilterException(GetExceptionInformation())){
//	__except(CrashExceptionDump(GetExceptionInformation(), szDumpFileName, true))
//	{
//		//HandleExceptionLog();
////		MessageBox(g_hWnd, "예상치 못한 오류가 발생했습니다.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
//	}
#endif

#ifdef _PUBLISH
	if (Mutex != 0) CloseHandle(Mutex);
#endif

//	CoUninitialize();

	exitAppStuff();
	return 0;
}
