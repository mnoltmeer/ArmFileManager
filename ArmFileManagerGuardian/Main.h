/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdExplicitTLSClientServerBase.hpp>
#include <IdFTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdAllFTPListParsers.hpp>
#include <Tlhelp32.h>
#include <IdMessageClient.hpp>
#include <IdSMTP.hpp>
#include <IdSMTPBase.hpp>
#include <IdIntercept.hpp>
#include <IdLogBase.hpp>
#include <IdLogFile.hpp>
#include <IdIOHandler.hpp>
#include <IdIOHandlerSocket.hpp>
#include <IdIOHandlerStack.hpp>
#include <IdIcmpClient.hpp>
#include <IdRawBase.hpp>
#include <IdRawClient.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Menus.hpp>
#include <IdSimpleServer.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <Vcl.Imaging.pngimage.hpp>

//---------------------------------------------------------------------------

String AppPath;

TDate DateStart;

const int MainPrmCnt = 3;

const wchar_t *MainParams[MainPrmCnt] = {L"StationID",
										 L"IndexVZ",
										 L"RegionID"};

const int GuardPrmCnt = 8;

const wchar_t *GuardParams[GuardPrmCnt] = {L"HideWnd",
										   L"FirewallRule",
										   L"RemAdmPort",
										   L"UseCollector",
										   L"CollectorHost",
										   L"CollectorPort",
										   L"UpdatesPath",
										   L"ForceStopManager"};

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TTrayIcon *TrayIcon1;
	TPopupMenu *PopupMenu1;
	TMenuItem *IconPP1;
	TMenuItem *IconPP2;
	TIdTCPServer *AURAServer;
	TMenuItem *IconPP3;
	TMenuItem *IconPP4;
	TIdTCPClient *AMColClient;
	TImageList *ImageList1;
	TImage *Image1;
	TImage *Image2;
	TImage *SwitchOn;
	TImage *SwitchOff;
	TLabel *LbStatus;
	TLabel *LbUpload;
	TLabel *LbDownload;
	TTimer *FileCountTimer;
	TTimer *SaveLogTimer;
	TMenuItem *IconPP5;
	TLabel *Label1;
	TLabel *ModuleVersion;
	void __fastcall IconPP1Click(TObject *Sender);
	void __fastcall IconPP2Click(TObject *Sender);
	void __fastcall AURAServerExecute(TIdContext *AContext);
	void __fastcall IconPP3Click(TObject *Sender);
	void __fastcall IconPP4Click(TObject *Sender);
	void __fastcall SwitchOnClick(TObject *Sender);
	void __fastcall SwitchOffClick(TObject *Sender);
	void __fastcall FileCountTimerTimer(TObject *Sender);
	void __fastcall Image1Click(TObject *Sender);
	void __fastcall Image2Click(TObject *Sender);
	void __fastcall SaveLogTimerTimer(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall IconPPConnClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);

private:	// User declarations
	TList *SrvList;
	TList *ThreadList;
	TList *MenuItemList;

	int __fastcall SendToCollector(String text);
	void __fastcall ShowInfoMsg(String textr);
	void __fastcall CheckConfig(String cfg_file, bool read_main_cfg);
	int __fastcall ReadConfig();
	void __fastcall CreateServers();
	TExchangeConnect* __fastcall FindServer(int id);
	TAMGThread* __fastcall FindServerThread(unsigned int thread_id);
	void __fastcall DeleteServerThread(unsigned int id);
	void __fastcall DeleteServerThreads();

	void __fastcall RunWork(TExchangeConnect *server);
	void __fastcall ResumeWork(TExchangeConnect *server);
	void __fastcall EndWork(TExchangeConnect *server);

	void __fastcall ShowDlLog();
	void __fastcall ShowUlLog();

	void __fastcall StartWork();
	void __fastcall StopWork();

	int __fastcall GetConnectionID(String caption);

	int __fastcall AAnswerToClient(TIdContext *AContext, TStringStream *ms);

	int __fastcall ASendStatus(TIdContext *AContext);
	int __fastcall ASendConfig(TStringList *list, TIdContext *AContext);
	int __fastcall ASendLog(TIdContext *AContext);
	int __fastcall ASendConnList(TIdContext *AContext);
	int __fastcall ASendThreadList(TIdContext *AContext);
	int __fastcall ASendFile(TStringList *list, TIdContext *AContext);
	int __fastcall ASendVersion(TIdContext *AContext);

	void __fastcall InitInstance();
    void __fastcall StopInstance();

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
