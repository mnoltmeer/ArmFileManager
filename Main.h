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
#include <Registry.hpp>

#define BUILD_APP

#include "eli_interface.h"
#include "TAMThread.h"
#include "TAMEliThread.h"
//---------------------------------------------------------------------------

String AppPath;

TDate DateStart;

const int MainPrmCnt = 24;

const wchar_t *MainParams[MainPrmCnt] = {L"HideWnd",
										 L"FirewallRule",
										 L"StationID",
										 L"IndexVZ",
										 L"MailFrom",
										 L"MailTo",
										 L"MailCodePage",
										 L"SmtpHost",
										 L"SmtpPort",
										 L"RemAdmPort",
										 L"SendReportToMail",
										 L"UseCollector",
										 L"CollectorHost",
										 L"CollectorPort",
										 L"RegionID",
										 L"UseGuardian",
										 L"GuardianUpdatesPath",
										 L"EnableAutoStart",
										 L"MailSubjectErr",
										 L"MailSubjectOK",
										 L"ControlScriptName",
										 L"ScriptLog",
										 L"ScriptInterval",
										 L"AutoStartForAllUsers"};

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TIdSMTP *MailSender;
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

	TList *GetSrvList(){return SrvList;}
    void SetSrvList(TList *val){if (val){SrvList = val;}}
	TList *GetThList(){return ThreadList;}
	void SetThList(TList *val){if (val){ThreadList = val;}}

    bool __fastcall ConnectToSMTP();
	void __fastcall SendMsg(String mail_addr, String subject, String from, String log);
	void __fastcall ShowInfoMsg(String textr);
	int __fastcall SendLog(String mail_addr, String subject, String from, String log);
	void __fastcall CheckConfig(String cfg_file);
	int __fastcall ReadConfig();

	void __fastcall CreateServers();
	TExchangeConnect* __fastcall FindServer(int id);

	TAMThread* __fastcall FindServerThread(unsigned int thread_id);
	void __fastcall DeleteServerThread(unsigned int id);
	void __fastcall DeleteServerThreads();

	void __fastcall RunWork(TExchangeConnect *server);
	void __fastcall ResumeWork(TExchangeConnect *server);
	void __fastcall EndWork(TExchangeConnect *server);

	void __fastcall ShowDlLog();
	void __fastcall ShowUlLog();

	void __fastcall StartWork();
	void __fastcall StopWork();

	int __fastcall UpdateGuardian();
	bool __fastcall GuardianRunning();
	int __fastcall RunGuardian();

	int __fastcall AAnswerToClient(TIdContext *AContext, TStringStream *ms);

	int __fastcall ASendStatus(TIdContext *AContext);
	int __fastcall ASendConfig(TStringList *list, TIdContext *AContext);
	int __fastcall ASendLog(TIdContext *AContext);
	int __fastcall ASendConnList(TIdContext *AContext);
	int __fastcall ASendThreadList(TIdContext *AContext);
	int __fastcall ASendFile(TStringList *list, TIdContext *AContext);
	int __fastcall ASendVersion(TIdContext *AContext);

	int __fastcall ConnectELI();
	int __fastcall ReleaseELI();
	void __fastcall ExecuteScript(String ctrl_script_name);
	void __fastcall LoadFunctionsToELI();
	void __fastcall InitInstance();
	void __fastcall StopInstance();
	void __fastcall AddFirewallRule();
    TExchangeConnect* __fastcall CreateConnection(String file);

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

	int __fastcall SendToCollector(String cathegory, String text);
	void __fastcall SendToGuardian(String text);
	int __fastcall GenConnectionID();
	int __fastcall CreateConnection(SERVCFG cfg, bool create_menu);
	int __fastcall CreateConnection(String file, bool create_menu);
	void __fastcall DestroyConnection(int id);
    void __fastcall RemoveConnection(int id);
	void __fastcall StartConnection(int id);
	void __fastcall StopConnection(int id);
	int __fastcall GetConnectionID(String caption);
	int __fastcall GetConnectionID(int index);
	int __fastcall ConnectionStatus(int id);
	String __fastcall ConnectionCfgPath(int id);
	void __fastcall ReloadConfig();
	int __fastcall SendFileCount();
	int __fastcall RecvFileCount();
	bool __fastcall WriteToCfg(String file, String param, String val);
	bool __fastcall WriteToCfg(int id, String param, String val);
	bool __fastcall RemoveFromCfg(String file, String param);
	void __fastcall SendMsgToCollector(String msg);
	void __fastcall WriteToMngrLog(String msg);
	void __fastcall ShutdownManager();
	void __fastcall ShutdownGuardian();
	int __fastcall StartGuardian();
	int __fastcall RestartGuardian();

	__property TList *Connections = {read = GetSrvList, write = SetSrvList};
	__property TList *Threads = {read = GetThList, write = SetThList};

    void __fastcall WndProc(Messages::TMessage& Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p);

//створення з'єднання через передачу імені файлу, що описує конфіг
//повертає ІД з'єднання
//pFile - шлях до файлу, що описує конфіг
//pAddMenu - вказує, чи треба створювати елемент контекстного меню
void __stdcall eCreateConnection(void *p);

//знищення з'єднання
//pID - ІД з'єднання
void __stdcall eDestroyConnection(void *p);

//знищення з'єднання та видалення файлу конфігу
//pID - ІД з'єднання
void __stdcall eRemoveConnection(void *p);

//запуск з'єднання
//pID - ІД з'єднання
void __stdcall eStartConnection(void *p);

//зупинка з'єднання
//pID - ІД з'єднання
void __stdcall eStopConnection(void *p);

//шукає ІД з'єднання за описом
//повертає ІД, або 0, якщо з'єднання не знайдене
//pCap - Caption з'єднання
void __stdcall eConnectionID(void *p);

//шукає ІД з'єднання за індексом у переліку з'єднань
//повертає ІД, або 0, якщо з'єднання не знайдене
//pIndex - індекс у списку з'єднання
void __stdcall eConnectionIDInd(void *p);

//повертає статус з'єднання
//1 - в роботі,
//0 - зупинено,
//-1 - помилка, не ініціалізоване
//pID - ІД з'єднання
void __stdcall eConnectionStatus(void *p);

//повертає шлях до конфігу з'єднання
//pID - ІД з'єднання
void __stdcall eConnectionCfgPath(void *p);

//перечитування параметрів з'єднаннь
void __stdcall eReloadConfig(void *p);

//запит кількості вивантажених файлів
void __stdcall eSendFileCount(void *p);

//запит кількості завантажених файлів
void __stdcall eRecvFileCount(void *p);

//читання певного параметру з конфігу
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
void __stdcall eReadFromCfg(void *p);

//видалення певного параметру з конфігу
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
void __stdcall eRemoveFromCfg(void *p);

//запис певного параметру у конфіг
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
//pVal - значення параметру
void __stdcall eWriteToCfg(void *p);
//теж саме, тільки конфіг визначається по ІД з'єднання
void __stdcall eWriteToCfgByID(void *p);

void __stdcall eSendMsgToCollector(void *p);
void __stdcall eWriteMsgToLog(void *p);
void __stdcall eGetAppPath(void *p);
void __stdcall eShutdownManager(void *p);
void __stdcall eShutdownGuardian(void *p);
void __stdcall eStartGuardian(void *p);
void __stdcall eRestartGuardian(void *p);
#endif
