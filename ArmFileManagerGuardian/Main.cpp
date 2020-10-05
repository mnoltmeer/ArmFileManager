/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TAMGuardian.h"
#include "TExchangeConnect.h"
#include "TAMThread.h"
#include "TAMGThread.h"
#include "Main.h"
#include "..\..\MyFunc\MyFunc.h"
#include "..\..\MyFunc\ThreadSafeLog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

String StationID, IndexVZ, RegionID, LogName, CollectorHost,
	   ConnPath, LogPath, UpdatesPath, AppVersion;

int SmtpPort, RemAdmPort, MngrRemAdmPort, CollectorPort;
bool HideWnd, UseCollector, FirewallRule, ForceStopManager;

TThreadSafeLog *Log;

TAMGuardian *GuardThread;
TExchangeConnect *ManagerConnect;

TForm *DlLog;
TForm *UlLog;
TMemo *DList;
TMemo *UList;
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));

  ConnPath = AppPath + "\\Connections\\Guardian";
  LogPath = AppPath + "\\Log\\Guardian";

  SrvList = new TList();
  ThreadList = new TList();
  MenuItemList = new TList();
  Log = new TThreadSafeLog();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckConfig(String cfg_file, bool read_main_cfg)
{
  try
	 {
	   if (read_main_cfg)
		 {
           for (int i = 0; i < MainPrmCnt; i++)
			  {
				if (GetConfigLine(cfg_file, MainParams[i]) == "^no_line")
			  	  AddConfigLine(cfg_file, MainParams[i], "0");
			  }
         }
	   else
		 {
           for (int i = 0; i < GuardPrmCnt; i++)
			  {
				if (GetConfigLine(cfg_file, GuardParams[i]) == "^no_line")
			  	  AddConfigLine(cfg_file, GuardParams[i], "0");
			  }
         }
	 }
  catch (Exception &e)
	 {
	   throw new Exception("Помилка перевірки конфігу з " + cfg_file +
			   			   " помилка: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReadConfig()
{
  Log->Add("Завантаження конфігу");

  int result = 0;

//Основний конфіг
  try
	 {
       CheckConfig(AppPath + "\\main.cfg", true);
	   CheckConfig(AppPath + "\\guardian.cfg", false);

	   if (ReadParameter(AppPath + "\\main.cfg", "StationID", &StationID, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру StationID: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру StationID: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\main.cfg", "IndexVZ", &IndexVZ, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру IndexVZ: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру IndexVZ: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\main.cfg", "RegionID", &RegionID, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру RegionID: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру RegionID: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\main.cfg", "RemAdmPort", &MngrRemAdmPort, TT_TO_INT) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MngrRemAdmPort: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру MngrRemAdmPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "HideWnd", &HideWnd, TT_TO_BOOL) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру HideWnd: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру HideWnd: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "FirewallRule", &FirewallRule, TT_TO_BOOL) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру FirewallRule: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру FirewallRule: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "RemAdmPort", &RemAdmPort, TT_TO_INT) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "UseCollector", &UseCollector, TT_TO_BOOL) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру UseCollector: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру UseCollector: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "CollectorHost", &CollectorHost, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру CollectorHost: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру CollectorHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "CollectorPort", &CollectorPort, TT_TO_INT) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру CollectorPort: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру CollectorPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(AppPath + "\\guardian.cfg", "UpdatesPath", &UpdatesPath, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру UpdatesPath: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру UpdatesPath: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (UpdatesPath.Pos("$AppPath") > 0)
		 UpdatesPath = ParseString(UpdatesPath, "$AppPath", AppPath);

	   if (ReadParameter(AppPath + "\\guardian.cfg", "ForceStopManager", &ForceStopManager, TT_TO_BOOL) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ForceStopManager: " + String(GetLastReadParamResult()));
		   SendToCollector("Помилка створення параметру ForceStopManager: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

			srv->UsedCollectorHost = CollectorHost;
			srv->UsedCollectorPort = CollectorPort;
			srv->ServerUseCollector = UseCollector;
            srv->ReInitialize();
          }
	 }
  catch(Exception &e)
	 {
	   Log->Add("Помилка читання конфігу: " + e.ToString());
	   result = -1;
	 }

  return result;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CreateServers()
{
  TStringList *servers = new TStringList();
  String str;

  try
	 {
	   GetFileList(servers, ConnPath, "*.cfg", WITHOUT_DIRS, WITH_FULL_PATH);
	   TExchangeConnect *srv;

	   for (int i = 0; i < servers->Count; i++)
		  {
			srv = new TExchangeConnect(AMColClient,
									   UseCollector,
									   TrayIcon1,
									   Log,
									   servers->Strings[i],
									   i + 1);

			srv->UsedCollectorHost = CollectorHost;
			srv->UsedCollectorPort = CollectorPort;
			srv->DownloadFileList = DList;
			srv->UploadfileList = UList;

			TMenuItem *srv_menu = new TMenuItem(PopupMenu1);

			srv_menu->Caption = "Запустити " + srv->ConnectionConfig->Caption;
			srv_menu->Hint = IntToStr(srv->ServerID);
			IconPP5->Add(srv_menu);
			IconPP5->SubMenuImages = ImageList1;
			srv_menu->ImageIndex = 4;
			srv_menu->Name = "srv_menu_" + IntToStr(i + 1);
            srv_menu->OnClick = IconPPConnClick;

            MenuItemList->Add(srv_menu);

			SrvList->Add(srv);

//якщо каталог для завантаження збігається з каталогом для оновлень
//це з'єднання, що оновлює Менеджер
			if (srv->ConnectionConfig->LocDirDl == UpdatesPath)
              ManagerConnect = srv;
		  }
	 }
  __finally {delete servers;}
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::SendToCollector(String text)
{
  if (!UseCollector)
	return 0;

  try
	 {
	   if (!AMColClient->Connected() && (CollectorHost != ""))
		 AMColClient->Connect(CollectorHost, CollectorPort);

	   String msg = RegionID + "&" +
					IndexVZ + ":" + StationID + "&" +
					"Guardian&" +
					DateToStr(Date()) + "&" +
					TimeToStr(Time()) + "&" +
					text;
	   TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);
	   ms->Position = 0;

	   try
		  {
			AMColClient->IOHandler->Write(ms, ms->Size, true);
			AMColClient->Disconnect();
		  }
	   catch (Exception &e)
		  {
			Log->Add("Відправка до Коллектору: " + e.ToString());
		  }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Відправка до Коллектору: " + e.ToString());
     }

  return 0;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowInfoMsg(String text)
{
  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = text;
  TrayIcon1->ShowBalloonHint();
}
//-------------------------------------------------------------------------

void __fastcall TMainForm::AURAServerExecute(TIdContext *AContext)
{
  String msg, cfg;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  TStringList *list = new TStringList();

  AContext->Connection->IOHandler->ReadStream(ms);

  try
	 {
	   try
		  {
            ms->Position = 0;
			msg = ms->ReadString(ms->Size);

			StrToList(list, msg, "%");

			if (list->Strings[0] == "#send")
			  {
				if (list->Strings[1] == "status")
				  ASendStatus(AContext);
				else if (list->Strings[1] == "cfg")
				  ASendConfig(list, AContext);
				else if (list->Strings[1] == "log")
				  ASendLog(AContext);
				else if (list->Strings[1] == "srvlist")
				  ASendConnList(AContext);
				else if (list->Strings[1] == "thlist")
				  ASendThreadList(AContext);
				else if (list->Strings[1] == "file")
				  ASendFile(list, AContext);
				else if (list->Strings[1] == "version")
				  ASendVersion(AContext);
			  }
            else if (list->Strings[0] == "#get")
			  {
				TStringList *ls = new TStringList();

				try
				   {
					 int ind = list->Strings[2].ToInt();

					 StrToList(ls, list->Strings[3], "#");

					 if (ind == 0)
					   {
						 ls->SaveToFile(AppPath + "\\guardian.cfg", TEncoding::UTF8);
						 ReadConfig();
					   }
					 else
					   {
						 TExchangeConnect *srv = FindServer(ind);

						 if (srv)
						   {
							 ls->SaveToFile(srv->ServerCfgPath, TEncoding::UTF8);
							 srv->ReInitialize();
						   }
						 else
					  	   throw Exception("невідомий ID з'єднання: " + list->Strings[2]);
					   }

					 TrayIcon1->BalloonFlags = bfInfo;
					 TrayIcon1->BalloonHint = "Дані з конфігу оновлені";
					 TrayIcon1->ShowBalloonHint();
				   }
				__finally {delete ls;}
			  }
			else if (list->Strings[0] == "#get_new")
			  {
				//#get_new%<filename>%<cfg_data>
				TStringList *ls = new TStringList();

                try
				   {
					 StrToList(ls, list->Strings[2], "#");

					 ls->SaveToFile(ConnPath + "\\" + list->Strings[1], TEncoding::UTF8);

					 TrayIcon1->BalloonFlags = bfInfo;
					 TrayIcon1->BalloonHint = "Отримано новий конфіг";
					 TrayIcon1->ShowBalloonHint();
				   }
				__finally {delete ls;}
			  }
			else if (list->Strings[0] == "#run")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  StartWork();
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  {
						TAMGThread *th = FindServerThread(srv->ServerThreadID);

						if (th)
						  th->PassedTime = srv->ConnectionConfig->MonitoringInterval;

						if (!srv->Working())
			 			  RunWork(srv);
					  }
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#stop")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  StopWork();
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  EndWork(srv);
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
            else if (list->Strings[0] == "#restart")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  {
					StopWork();
					Sleep(1000);
					StartWork();
				  }
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  {
						EndWork(srv);
						Sleep(1000);
                        RunWork(srv);
					  }
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
            else if (list->Strings[0] == "#delcfg_file")
			  {
				//#delcfg_file%<filename>
				Log->Add("AMRA: отримано команду видалення конфігу, ім'я файлу: " + list->Strings[1]);
				DeleteFile(ConnPath + "\\" + list->Strings[1] + ".cfg");
			  }
			else if (list->Strings[0] == "#delcfg_name")
			  {
				//#delcfg_name%<caption>
				Log->Add("AMRA: отримано команду видалення конфігу, ім'я з'єднання: " + list->Strings[1]);

				int ind = GetConnectionID(list->Strings[1]);

				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  DeleteFile(srv->ServerCfgPath);
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
			  }
			else if (list->Strings[0] == "#delcfg_id")
			  {
				//#delcfg_id%<id>
				Log->Add("AMRA: отримано команду видалення конфігу, ID з'єднання: " + list->Strings[1]);

				int ind = list->Strings[1].ToInt();

				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  DeleteFile(srv->ServerCfgPath);
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
              }
			else if (list->Strings[0] == "#shutdown")
			  {
				Log->Add("AMRA: отримано команду #shutdown");
				SendToCollector("AMRA: отримано команду #shutdown");
				PostMessage(Application->Handle, WM_QUIT, 0, 0);
			  }
			else if (list->Strings[0] == "#cpy_manager")
			  {
				Log->Add("AMRA: отримано команду копіювання Менеджеру з каталогу оновлень");
				SendToCollector("AMRA: Отримано команду копіювання Менеджеру з каталогу оновлень");
				GuardThread->UpdateStarted = true;
			  }
			else if (list->Strings[0] == "#upd_manager")
			  {
				Log->Add("AMRA: отримано команду завантаження та оновлення Менеджеру");
				SendToCollector("AMRA: Отримано команду завантаження та оновлення Менеджеру");

				if (ManagerConnect)
				  {
					TAMGThread *th = FindServerThread(ManagerConnect->ServerThreadID);

					if (th)
					  th->PassedTime = ManagerConnect->ConnectionConfig->MonitoringInterval;

					if (!ManagerConnect->Working())
					  RunWork(ManagerConnect);
				  }
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
			  }
			else if (list->Strings[0] == "#manager_stopped")
			  {
				GuardThread->ManagerActiveConnections = false;

				Log->Add("AMRA: усі з'єднання Менеджеру закриті");
				SendToCollector("AMRA: усі з'єднання Менеджеру закриті");
			  }
			else if (list->Strings[0] == "#init_request")
			  {
				Log->Add("AMRA: отримано запит від Менеджеру на ініціалізацію");
				SendToCollector("AMRA: отримано запит від Менеджеру на ініціалізацію");

				if (!GuardThread->UpdateStarted)
				  {
					GuardThread->SendToManager("#init");

					Log->Add("AMRA: дозвіл на ініціалізацію надано");
					SendToCollector("AMRA: дозвіл на ініціалізацію надано");
				  }
				else
				  {
					Log->Add("AMRA: запущене оновлення. В ініціалізації відмовлено");
					SendToCollector("AMRA: запущене оновлення. В ініціалізації відмовлено");
                  }
			  }
			else if (list->Strings[0] == "#instance_offline")
			  {
				Log->Add("AMRA: Менеджер не ініціалізований");
				SendToCollector("AMRA: Менеджер не ініціалізований");

				GuardThread->SendToManager("#init");

				Log->Add("AMRA: надсилання команди ініціалізації");
				SendToCollector("AMRA: надсилання команди ініціалізації");
			  }
			else if (list->Strings[0] == "#instance_online")
			  {
				Log->Add("AMRA: Менеджер ініціалізований");
				SendToCollector("AMRA: Менеджер ініціалізований");
			  }
			else
			  throw Exception("невідомі дані: " + list->Strings[0]);
		  }
	   catch (Exception &e)
		  {
			Log->Add("AMRA: " + e.ToString());
			SendToCollector("AMRA: " + e.ToString());
		  }
	 }
  __finally {delete list; delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopWork()
{
  SwitchOff->Visible = false;
  SwitchOn->Visible = true;
  LbStatus->Caption = "Зупинено";
  LbStatus->Font->Color = clRed;
  LbStatus->Tag = 0;

  for (int i = 0; i < SrvList->Count; i++)
	 {
	   EndWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartWork()
{
  SwitchOn->Visible = false;
  SwitchOff->Visible = true;
  LbStatus->Caption = "Робота";
  LbStatus->Font->Color = clLime;
  LbStatus->Tag = 1;

  for (int i = 0; i < SrvList->Count; i++)
	 {
	   RunWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));
	 }
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TMainForm::FindServer(int id)
{
  for (int i = 0; i < SrvList->Count; i++)
	 {
	   TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

	   if (srv->ServerID == id)
         return srv;
	 }

  return NULL;
}
//---------------------------------------------------------------------------

TAMGThread* __fastcall TMainForm::FindServerThread(unsigned int thread_id)
{
  for (int i = 0; i < ThreadList->Count; i++)
	 {
	   TAMGThread *th = (TAMGThread*)ThreadList->Items[i];

	   if (th->ThreadID == thread_id)
		 return th;
	 }

  return NULL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DeleteServerThread(unsigned int id)
{
  int ind = 0;

  while (ind < ThreadList->Count)
	 {
	   TAMGThread *th = (TAMGThread*)ThreadList->Items[ind];

	   if (th->ThreadID == id)
		 {
		   delete th;
		   ThreadList->Delete(ind);

		   return;
		 }
	   else
         ind++;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DeleteServerThreads()
{
  int ind = 0;

  while (ind < ThreadList->Count)
	 {
	   TAMGThread *th = (TAMGThread*)ThreadList->Items[ind];

	   ThreadList->Delete(ind);
       delete th;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RunWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (!server->Working())
		{
		  if (server->ServerThreadID > 0)
			ResumeWork(server);
		  else if (server->Initialized())
			{
			  TAMGThread *serv_thread = new TAMGThread(true);
			  serv_thread->InfoIcon = TrayIcon1;
			  serv_thread->Connection = server;
			  server->ServerThreadID = serv_thread->ThreadID;
              serv_thread->GuardianThread = GuardThread;
			  ThreadList->Add(serv_thread);
			  serv_thread->Resume();
			  server->Start();
			}
          else
			{
			  Log->Add("З'єднання з ID " + IntToStr(server->ServerID) + " не ініціалізоване!");
			  SendToCollector("З'єднання з ID " + IntToStr(server->ServerID) + " не ініціалізоване!");
			}
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ResumeWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (!server->Working())
		server->Start();

	  if (!FindServerThread(server->ServerThreadID))
		{
		  Log->Add("Зі з'єднанням " + IntToStr(server->ServerID) + ":" +
				   server->ServerCaption +
				   " не пов'язано жодного потоку!");
		  SendToCollector("Помилковий вказівник TExchangeConnect*");
        }
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EndWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (server->Working())
	  	server->Stop();

	  TAMGThread *th = FindServerThread(server->ServerThreadID);

	  if (th)
		{
		  th->Terminate();

		  while (!th->Finished)
			Sleep(100);

		  th->Connection = NULL;
		  DeleteServerThread(th->ThreadID);
          server->ServerThreadID = 0;
		}
      else
		{
		  Log->Add("Помилковий ID потоку " + server->ServerCaption +
				   ", поток: " + IntToStr((int)server->ServerThreadID));
		  SendToCollector("Помилковий ID потоку " + server->ServerCaption +
						  ", поток: " + IntToStr((int)server->ServerThreadID));
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowDlLog()
{
  try
	 {
	   if (DlLog)
		 DlLog->Show();
	   else
		 {
		   DlLog = new TForm(this);

		   DlLog->Parent = DlLog;
		   DlLog->Name = "DlLog";
		   DlLog->AutoSize = false;
		   DlLog->AutoScroll = true;
		   DlLog->BorderStyle = bsDialog;
		   DlLog->FormStyle = fsStayOnTop;
		   DlLog->ClientWidth = 180;
		   DlLog->ClientHeight = 180;
		   DlLog->Left = Screen->Width - DlLog->ClientWidth - 10;
		   DlLog->Top = Screen->Height - DlLog->ClientHeight - 100;
		   DlLog->Caption = "ЗАВАНТАЖЕНО";
		   DlLog->TransparentColor = true;
		   DlLog->TransparentColorValue = clBtnFace;

		   DList = new TMemo(this);

		   DList->Parent = DlLog;
		   DList->Width = DlLog->ClientWidth;
		   DList->Height = DlLog->ClientHeight;
		   DList->Font->Size = 7;
		   DList->Font->Color = clGreen;
		   DList->ParentColor = true;
		   DList->ScrollBars = ssBoth;

		   DlLog->Show();
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("ShowDlLog(): " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowUlLog()
{
  try
	 {
	   if (UlLog)
		 UlLog->Show();
	   else
		 {
		   UlLog = new TForm(this);

		   UlLog->Parent = UlLog;
		   UlLog->Name = "UlLog";
		   UlLog->AutoSize = false;
		   UlLog->AutoScroll = true;
		   UlLog->BorderStyle = bsDialog;
		   UlLog->FormStyle = fsStayOnTop;
		   UlLog->ClientWidth = 180;
		   UlLog->ClientHeight = 180;
		   UlLog->Left = DlLog->Left;
		   UlLog->Top = DlLog->Top - UlLog->ClientHeight - 40;
		   UlLog->Caption = "ВИВАНТАЖЕНО";
		   UlLog->TransparentColor = true;
		   UlLog->TransparentColorValue = clBtnFace;

		   UList = new TMemo(this);

		   UList->Parent = UlLog;
		   UList->Width = UlLog->ClientWidth;
		   UList->Height = UlLog->ClientHeight;
		   UList->Font->Size = 7;
		   UList->Font->Color = clBlue;
		   UList->ParentColor = true;
		   UList->ScrollBars = ssBoth;

		   UlLog->Show();
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("ShowUlLog(): " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::InitInstance()
{
  try
	 {
	   GuardThread = new TAMGuardian(true);

	   GuardThread->CollectorServer = AMColClient;
	   GuardThread->ServiceLog = Log;
	   GuardThread->UsingCollector = UseCollector;
	   GuardThread->UsedCollectorHost = CollectorHost;
	   GuardThread->UsedCollectorPort = CollectorPort;
	   GuardThread->UpdatesDirectory = UpdatesPath;
	   GuardThread->UseForceStop = ForceStopManager;
	   GuardThread->UsedMngrAdmPort = MngrRemAdmPort;
	   GuardThread->UpdateStarted = false;
	   GuardThread->Resume();
	 }
  catch (Exception &e)
	 {
	   Log->Add("Старт потоку оновлення: " + e.ToString());
	   SendToCollector("Старт потоку оновлення: " + e.ToString());

	   throw new Exception("Помилка ініціалізації потоку оновлення!");
	 }

  try
	 {
	   FileCountTimer->Enabled = true;
	   CreateServers();

	   Log->Add("Початок роботи");
	   SendToCollector("Початок роботи");
	   Log->Add("Версія модулю: " + AppVersion);

       StartWork();

       TrayIcon1->BalloonFlags = bfInfo;
	   TrayIcon1->BalloonHint = "Guardian запущено";
	   TrayIcon1->ShowBalloonHint();
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ініціалізація: " + e.ToString());
	   SendToCollector("Ініціалізація: " + e.ToString());

	   throw new Exception("Помилка ініціалізації!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopInstance()
{
  try
	 {
	   if (LbStatus->Tag == 1)
		 StopWork();

	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

			if (srv->Working())
			  EndWork(srv);

			delete srv;
		  }

	   delete SrvList;
	   delete ThreadList;

	   GuardThread->Terminate();

	   while (!GuardThread->Finished)
		 Sleep(100);

	   delete GuardThread;

	   for (int i = 0; i < MenuItemList->Count; i++)
		  {
			TMenuItem *m = reinterpret_cast<TMenuItem*>(MenuItemList->Items[i]);
			delete m;
		  }

	   delete MenuItemList;

	   if (DList)
		 delete DList;

	   if (UList)
		 delete UList;

	   if (UlLog)
		 {
		   UlLog->Close();
		   delete UlLog;
		 }

	   if (DlLog)
		 {
		   DlLog->Close();
		   delete DlLog;
		 }

	   TrayIcon1->Visible = false;
	   Log->Add("Кінець роботи");
	   SendToCollector("Кінець роботи");
	 }
  catch (Exception &e)
	 {
	   Log->Add("Завершення роботи: " + e.ToString());
	   SendToCollector("Завершення роботи: " + e.ToString());

	   throw new Exception("Помилка завершення роботи!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP1Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP2Click(TObject *Sender)
{
  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = "Дані з конфігу оновлені";
  TrayIcon1->ShowBalloonHint();
  ReadConfig();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP3Click(TObject *Sender)
{
  if (IconPP3->Tag == 0)
	{
	  ShowDlLog();
	  ShowUlLog();
	  IconPP3->Caption = "Приховати списки файлів";
	  IconPP3->Tag = 1;
	}
  else
	{
	  if (DlLog)
		DlLog->Close();

	  if (UlLog)
		UlLog->Close();

	  IconPP3->Caption = "Показати списки файлів";
	  IconPP3->Tag = 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP4Click(TObject *Sender)
{
  if (IconPP4->Tag == 0)
	{
      WindowState = wsNormal;
	  Show();
	  IconPP4->Caption = "Приховати головне вікно";
	  IconPP4->Tag = 1;
	}
  else
	{
	  Hide();
	  IconPP4->Caption = "Показати головне вікно";
	  IconPP4->Tag = 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwitchOnClick(TObject *Sender)
{
  StartWork();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwitchOffClick(TObject *Sender)
{
  StopWork();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileCountTimerTimer(TObject *Sender)
{
  LbUpload->Caption = IntToStr(UList->Lines->Count);
  LbDownload->Caption = IntToStr(DList->Lines->Count);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SaveLogTimerTimer(TObject *Sender)
{
  TStringStream *ss = new TStringStream("", TEncoding::UTF8, true);

  if (!FileExists(LogPath + "\\" + LogName))
	SaveToFile(LogPath + "\\" + LogName, "");

  TFileStream *fs = new TFileStream(LogPath + "\\" + LogName, fmOpenReadWrite);

  try
	 {
	   Log->SaveToStream(ss);

	   if (ss->Size > fs->Size)
		 {
		   fs->Position = fs->Size;
		   ss->Position = fs->Size;
		   fs->Write(ss->Bytes, ss->Position, ss->Size - ss->Position);
		 }
	 }
  __finally {delete ss; delete fs;}

  if (Date().CurrentDate() > DateStart)
	{
	  Log->Clear();
	  LogName = DateToStr(Date()) + ".log";
	  DateStart = Date().CurrentDate();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Image1Click(TObject *Sender)
{
  ShowUlLog();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Image2Click(TObject *Sender)
{
  ShowDlLog();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  AppVersion = GetVersionInString(Application->ExeName.c_str());
  ModuleVersion->Caption = AppVersion;
  TrayIcon1->Hint = Caption;

  if (!DirectoryExists(LogPath))
	ForceDirectories(LogPath);

  if (!DirectoryExists(ConnPath))
	ForceDirectories(ConnPath);

  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + ".log";

  if (FileExists(LogPath + "\\" + LogName))
	Log->LoadFromFile(LogPath + "\\" + LogName);

  ShowDlLog();
  DlLog->Hide();

  ShowUlLog();
  UlLog->Hide();

  if (ReadConfig() == 0)
	{
	  if (!FirewallRule)
		{
		  if (system("netsh advfirewall firewall show rule name=\"ArmMngrGuardian\"") != 0)
			{
			  AnsiString cmd = "netsh advfirewall firewall add rule name=\"ArmMngrGuardian\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=all";
			  system(cmd.c_str());
			  Sleep(200);
			  cmd = "netsh advfirewall firewall add rule name=\"ArmMngrGuardian\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=all";
			  system(cmd.c_str());
			  Sleep(200);

			  if (system("netsh advfirewall firewall show rule name=\"AMGRA\"") != 0)
				{
				  cmd = "netsh advfirewall firewall add rule name=\"AMGRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=all";
				  system(cmd.c_str());
				  Sleep(200);
				  cmd = "netsh advfirewall firewall add rule name=\"AMGRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=all";
				  system(cmd.c_str());
				}

			  if (system("netsh advfirewall firewall show rule name=\"AMCOL\"") != 0)
				{
				  cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=in action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=all";
				  system(cmd.c_str());
				  Sleep(200);
				  cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=out action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=all";
				  system(cmd.c_str());
				}

			  FirewallRule = true;
			  SetConfigLine(AppPath + "\\guardian.cfg", "FirewallRule", "1");
			}
		  else
			{
              FirewallRule = true;
			  SetConfigLine(AppPath + "\\guardian.cfg", "FirewallRule", "1");
            }
		}

	  SaveLogTimer->Enabled = true;

      if (HideWnd == 1)
		WindowState = wsMinimized;

	  AURAServer->DefaultPort = RemAdmPort;
	  AURAServer->Active = true;

	  AMColClient->Host = CollectorHost;
	  AMColClient->Port = CollectorPort;

	  try
		 {
		   InitInstance();
		 }
	  catch (Exception &e)
		 {
		   Log->Add(e.ToString());
		   SendToCollector(e.ToString());
		 }
	}
  else
	{
	  Log->SaveToFile(LogPath + "\\" + LogName);
      LbStatus->Caption = "Помилка!";
	  SwitchOn->Enabled = false;
	  SwitchOff->Enabled = false;
	  Image1->Enabled = false;
      Image2->Enabled = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPPConnClick(TObject *Sender)
{
  TMenuItem *menu = reinterpret_cast<TMenuItem*>(Sender);
  TExchangeConnect *srv;
  TAMGThread *th;

  try
	 {
	   srv = FindServer(menu->Hint.ToInt());

	   if (srv)
		 {
		   th = FindServerThread(srv->ServerThreadID);

		   if (th)
			 th->PassedTime = srv->ConnectionConfig->MonitoringInterval;

           if (!srv->Working())
		   	 RunWork(srv);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ручний запуск обміну: " + e.ToString());
	   SendToCollector("Ручний запуск обміну: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
  try
	 {
	   StopInstance();
	 }
  catch (Exception &e)
	 {
	   Log->Add("FormDestroy(): " + e.ToString());
	   SendToCollector(e.ToString());
	 }

  AURAServer->Active = false;
  SaveLogTimer->Enabled = false;

  Log->SaveToFile(LogPath + "\\" + LogName);

  delete Log;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendStatus(TIdContext *AContext)
{
  Log->Add("AMRA: запит статусу");
  SendToCollector("AMRA: запит статусу");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);
			msg += IntToStr(srv->ServerID) + ": " + srv->ServerCaption + "=" + srv->ServerStatus + "#";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendConfig(TStringList *list, TIdContext *AContext)
{
  Log->Add("AMRA: запит конфігурації");
  SendToCollector("AMRA: запит конфігурації");

  int ind, res = -1;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  TStringList *ls = new TStringList();

  try
	 {
       String msg = "";

	   try
		  {
			ind = list->Strings[2].ToInt();

            if (ind == 0)
			  ls->LoadFromFile(AppPath + "\\guardian.cfg");
			else
			  {
				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  ls->LoadFromFile(srv->ServerCfgPath);
				else
				  throw Exception("AMRA: невідомий ID з'єднання: " + list->Strings[2]);
			  }

            for (int i = 0; i < ls->Count; i++)
			   {
				 msg += ls->Strings[i] + "#";
			   }

			if (msg == "")
			  msg = "error ";

			msg.Delete(msg.Length(), 1);
			ms->Clear();
			ms->WriteString(msg);
			ms->Position = 0;
            res = AAnswerToClient(AContext, ms);
		  }
	   catch (Exception &e)
		  {
			Log->Add("AMRA: " + e.ToString());
			SendToCollector("AMRA: " + e.ToString());
		  }
     }
  __finally {delete ls; delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendLog(TIdContext *AContext)
{
  Log->Add("AMRA: запит логу");
  SendToCollector("AMRA: запит логу");

  String msg = "";
  int res = -1;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

  try
	 {
	   for (int i = 0; i < Log->Count; i++)
		  {
			msg += Log->Get(i) + "&";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendConnList(TIdContext *AContext)
{
  Log->Add("AMRA: запит списку з'єднань");
  SendToCollector("AMRA: запит списку з'єднань");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);
			msg += IntToStr(srv->ServerID) + ": " + srv->ServerCaption + "#";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendThreadList(TIdContext *AContext)
{
  Log->Add("AMRA: запит списку потоків");
  SendToCollector("AMRA: запит списку потоків");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < ThreadList->Count; i++)
		  {
			TAMThread *th = (TAMThread*)ThreadList->Items[i];
			msg += "Thread: " + IntToStr((int)th->ThreadID) + "=";

			if (th->Connection)
			  {
				msg += "Connection: id=" + IntToStr((int)th->Connection->ServerID) + ", " +
					   th->Connection->ServerCaption + ", ";

				if (th->Connection->Working())
				  msg += "Runnig#";
				else
				  msg += "Stoped#";
			  }
			else
			  {
				msg += "No connection#";
			  }
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendFile(TStringList *list, TIdContext *AContext)
{
  Log->Add("AMRA: запит файлу " + list->Strings[2]);
  SendToCollector("AMRA: запит файлу " + list->Strings[2]);

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   if (FileExists(list->Strings[2]))
		 ms->LoadFromFile(list->Strings[2]);

	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendVersion(TIdContext *AContext)
{
  Log->Add("AMRA: запит версії");
  SendToCollector("AMRA: запит версії");

  String msg = GetVersionInString(Application->ExeName.c_str());
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::AAnswerToClient(TIdContext *AContext, TStringStream *ms)
{
  try
	 {
	   AContext->Connection->IOHandler->Write(ms, ms->Size, true);
	 }
  catch (Exception &e)
	 {
	   Log->Add("AMRA: " + e.ToString());
	   SendToCollector("AMRA: " + e.ToString());

	   return -1;
	 }

  return 0;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GetConnectionID(String caption)
{
  int res = 0;

  for (int i = 0; i < SrvList->Count; i++)
	 {
	   TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

	   if (srv->ConnectionConfig->Caption == caption)
		 {
		   res = srv->ServerID;
           break;
		 }
	 }

  return res;
}
//---------------------------------------------------------------------------
