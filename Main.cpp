/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TExchangeConnect.h"
#include "Main.h"
#include "..\work-functions\MyFunc.h"
#include "..\work-functions\ThreadSafeLog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

String StationID, IndexVZ, RegionID, MailFrom, MailTo,
	   MailSubjectErr, MailSubjectOK, MailCodePage,
	   SmtpHost, LogName, CollectorHost, ConnPath, LogPath,
	   GuardianUpdatesPath, AppVersion, ControlScript;

int SmtpPort, RemAdmPort, CollectorPort, ScriptInterval, RemAdmGuardPort;
bool SendReportToMail, UseCollector, UseGuardian,
	 FirewallRule, EnableAutoStart, HideWnd, ScriptLog, AutoStartForAllUsers;

bool Initialised; //флаг активності інстансу, стає істиною після вдалого виконання InitInstance()

TThreadSafeLog *Log;

TForm *DlLog;
TForm *UlLog;
TMemo *DList;
TMemo *UList;

HINSTANCE dllhandle;

GETELIINTERFACE GetELI;
FREEELIINTERFACE FreeELI;

ELI_INTERFACE *eIface;

TAMEliThread *EliThread;
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));

  ConnPath = AppPath + "\\Connections";
  LogPath = AppPath + "\\Log";

  SrvList = new TList();
  ThreadList = new TList();
  MenuItemList = new TList();
  Log = new TThreadSafeLog();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckConfig(String cfg_file)
{
  try
	 {
	   for (int i = 0; i < MainPrmCnt; i++)
		  {
			if (GetConfigLine(cfg_file, MainParams[i]) == "^no_line")
			  {
				if (wcscmpi(MainParams[i], L"ScriptInterval") == 0)
				  AddConfigLine(cfg_file, MainParams[i], "10");
				else
				  AddConfigLine(cfg_file, MainParams[i], "0");
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
  Log->Add("Завантаження конфігу з " + AppPath + "\\main.cfg");

  int result = 0;

//Основний конфіг
  try
	 {
	   CheckConfig(AppPath + "\\main.cfg");

	   int rp;

	   rp = ReadParameter(AppPath + "\\main.cfg", "HideWnd", &HideWnd, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру HideWnd: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру HideWnd: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "FirewallRule", &FirewallRule, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру FirewallRule: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру FirewallRule: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "StationID", &StationID, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру StationID: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру StationID: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "IndexVZ", &IndexVZ, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру IndexVZ: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру IndexVZ: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

		rp = ReadParameter(AppPath + "\\main.cfg", "MailFrom", &MailFrom, TT_TO_STR);

		if (rp != RP_OK)
		  {
			Log->Add("Помилка створення параметру MailFrom: " + String(GetLastReadParamResult()));
			SendToCollector("подія",
						   "Помилка створення параметру MailFrom: " + String(GetLastReadParamResult()));
			result = -1;
		  }

	   rp = ReadParameter(AppPath + "\\main.cfg", "MailTo", &MailTo, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailTo: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру MailTo: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "MailCodePage", &MailCodePage, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailCodePage: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру MailCodePage: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "SmtpHost", &SmtpHost, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpHost: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру SmtpHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "SmtpPort", &SmtpPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpPort: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру SmtpPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "RemAdmPort", &RemAdmPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "SendReportToMail", &SendReportToMail, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SendReportToMail: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру SendReportToMail: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "UseCollector", &UseCollector, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру UseCollector: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру UseCollector: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "CollectorHost", &CollectorHost, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру CollectorHost: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру CollectorHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "CollectorPort", &CollectorPort, TT_TO_INT) ;

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру CollectorPort: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру CollectorPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "RegionID", &RegionID, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру RegionID: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру RegionID: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "UseGuardian", &UseGuardian, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру UseGuardian: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру UseGuardian: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (ReadParameter(AppPath + "\\main.cfg", "GuardianUpdatesPath", &GuardianUpdatesPath, TT_TO_STR) != RP_OK)
		 {
		   Log->Add("Помилка створення параметру GuardianUpdatesPath: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
							   "Помилка створення параметру GuardianUpdatesPath: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (GuardianUpdatesPath.Pos("$AppPath") > 0)
		 GuardianUpdatesPath = ParseString(GuardianUpdatesPath, "$AppPath", AppPath);

       if (UseGuardian)
		 {
		   rp = ReadParameter(AppPath + "\\guardian.cfg", "RemAdmPort", &RemAdmGuardPort, TT_TO_INT);

		   if (rp != RP_OK)
			 {
			   Log->Add("Помилка створення параметру RemAdmGuardPort: " + String(GetLastReadParamResult()));
			   SendToCollector("подія",
							   "Помилка створення параметру RemAdmGuardPort: " + String(GetLastReadParamResult()));
			   result = -1;
			 }
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "EnableAutoStart", &EnableAutoStart, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру EnableAutoStart: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру EnableAutoStart: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 {
		   rp = ReadParameter(AppPath + "\\main.cfg", "AutoStartForAllUsers", &AutoStartForAllUsers, TT_TO_BOOL);

		   if (rp != RP_OK)
			 {
			   Log->Add("Помилка створення параметру AutoStartForAllUsers: " + String(GetLastReadParamResult()));
			   SendToCollector("подія",
							   "Помилка створення параметру AutoStartForAllUsers: " + String(GetLastReadParamResult()));
			   result = -1;
			 }
		 }

	   String str;

	   rp = ReadParameter(AppPath + "\\main.cfg", "MailSubjectErr", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectErr: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру MailSubjectErr: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(AppPath + "\\main.cfg", "MailSubjectOK", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectOK: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру MailSubjectOK: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(AppPath + "\\main.cfg", "ControlScriptName", &ControlScript, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ControlScriptName: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру ControlScriptName: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "ScriptLog", &ScriptLog, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ScriptLog: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру ScriptLog: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(AppPath + "\\main.cfg", "ScriptInterval", &ScriptInterval, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ScriptInterval: " + String(GetLastReadParamResult()));
		   SendToCollector("подія",
						   "Помилка створення параметру ScriptInterval: " + String(GetLastReadParamResult()));
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
	   GetFileList(servers, ConnPath, "*.cfg", false, true);

	   for (int i = 0; i < servers->Count; i++)
		  CreateConnection(servers->Strings[i]);
	 }
  __finally {delete servers;}
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::ConnectToSMTP()
{
  MailSender->Username = "noname@ukrposhta.com";
  MailSender->Password = "noname";
  MailSender->Host = SmtpHost;
  MailSender->Port = SmtpPort;

  try
	{
	  MailSender->Connect();
	}
  catch (Exception &e)
	{
	  Log->Add("SMTP помилка: " + e.ToString());
	  SendToCollector("подія", "SMTP помилка: " + e.ToString());

	  return false;
	}

  return MailSender->Connected();
}
//-------------------------------------------------------------------------

void __fastcall TMainForm::SendMsg(String mail_addr, String subject, String from, String log)
{
  if (MailSender->Connected())
	{
	  TIdMessage* msg = new TIdMessage(MainForm);

	  msg->CharSet = MailCodePage;
	  msg->Body->Text = log;
	  msg->From->Text = from;
	  msg->Recipients->EMailAddresses = mail_addr;
	  msg->Subject = subject;
	  msg->Priority = TIdMessagePriority(mpHighest);

	  MailSender->Send(msg);
	  MailSender->Disconnect();

	  delete msg;
	}
}
//-------------------------------------------------------------------------

int __fastcall TMainForm::SendToCollector(String cathegory, String text)
{
  if (!UseCollector)
	return 0;

  int res;
  String msg = RegionID + "&" +
			   IndexVZ + ":" + StationID + "&" +
			   cathegory + "&" +
			   DateToStr(Date()) + "&" +
			   TimeToStr(Time()) + "&" +
			   text;
  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  ms->Position = 0;

  try
	 {
	   try
		  {
            if (!AMColClient->Connected() && (CollectorHost != ""))
		 	  AMColClient->Connect(CollectorHost, CollectorPort);

			AMColClient->IOHandler->Write(ms, ms->Size, true);
			res = 1;
		  }
	   catch (Exception &e)
		  {
			Log->Add("Відправка до Коллектору: " + e.ToString());
			res = -1;
		  }
	 }
  __finally
	 {
       delete ms;

	   if (AMColClient->Connected())
		 AMColClient->Disconnect();
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SendToGuardian(String text)
{
  if (!UseGuardian)
    return;

  TStringStream *ms = new TStringStream(text, TEncoding::UTF8, true);

  try
	 {
	   try
		  {
			TIdTCPClient *sender = CreateSimpleTCPSender(L"127.0.0.1", RemAdmGuardPort);

            sender->Connect();

			Log->Add("Надсилання Guardian команди: " + text);
			SendToCollector("подія", "Надсилання Guardian команди: " + text);

			ms->Position = 0;
			sender->IOHandler->Write(ms, ms->Size, true);

			FreeSimpleTCPSender(sender);
		  }
       catch (Exception &e)
		  {
			Log->Add("Надсилання команд Guardian: " + e.ToString());
			SendToCollector("подія", "Надсилання команд Guardian: " + e.ToString());
		  }
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShowInfoMsg(String text)
{
  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = text;
  TrayIcon1->ShowBalloonHint();
}
//-------------------------------------------------------------------------

int __fastcall TMainForm::SendLog(String mail_addr, String subject, String from, String log)
{
  int try_cnt = 0;

  while (!ConnectToSMTP())
	{
	  try_cnt++;

	  if (try_cnt >= 10)
		{
		  return 0;
		}

	  Sleep(30000);
	}

  SendMsg(mail_addr, subject, from, log);

  return 1;
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
						 ls->SaveToFile(AppPath + "\\main.cfg", TEncoding::UTF8);
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
                     ReadConfig();
				   }
				__finally {delete ls;}
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
				  {
					DeleteFile(srv->ServerCfgPath);
					DestroyConnection(srv->ServerID);
				  }
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
				  {
					DeleteFile(srv->ServerCfgPath);
                    DestroyConnection(srv->ServerID);
				  }
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
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
						TAMThread *th = FindServerThread(srv->ServerThreadID);

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
			else if (list->Strings[0] == "#shutdown")
			  {
				Log->Add("AMRA: отримано команду shutdown");
				SendToCollector("AMRA", "Отримано команду shutdown");
				PostMessage(Application->Handle, WM_QUIT, 0, 0);
			  }
			else if (list->Strings[0] == "#restart_guard")
			  {
				Log->Add("AMRA: отримано команду перезапуску Guardian");
				SendToCollector("AMRA", "Отримано команду перезапуску Guardian");

				PostMessage(FindHandleByName(L"Guardian Менеджера обміну файлами АРМ ВЗ"), WM_QUIT, 0, 0);
				Sleep(3000);
				RunGuardian();
			  }
			else if (list->Strings[0] == "#reload_cfg")
			  {
				Log->Add("AMRA: отримано команду перечитування конфігів");
				SendToCollector("AMRA", "Отримано команду перечитування конфігів");
  				ReadConfig();
			  }
			else if (list->Strings[0] == "#exec_script")
			  {
				Log->Add("AMRA: надійшов керуючий скрипт");
				SendToCollector("AMRA", "Надійшов керуючий скрипт");

				try
				   {
					 eIface->RunScript(list->Strings[1].c_str(),
									   L"",
									   ScriptLog);

                     TStringList *lst = new TStringList();

					 try
						{
						  StrToList(lst, eIface->ShowInfoMessages(), "\r\n");

						  for (int i = 0; i < lst->Count; i++)
							Log->Add(lst->Strings[i]);
						}
					 __finally {delete lst;}
				   }
				catch (Exception &e)
				   {
					 Log->Add("ELI: помилка виконання скрипту " + e.ToString());
					 SendToCollector("ELI", "Помилка виконання скрипту " + e.ToString());
				   }
			  }
			else if (list->Strings[0] == "#init")
			  {
				Log->Add("AMRA: надійшла команда ініціалізації");
				SendToCollector("AMRA", "надійшла команда ініціалізації");

				try
				   {
					 InitInstance();
				   }
				catch (Exception &e)
				   {
					 Log->Add("Ініціалізація: " + e.ToString());
					 SendToCollector("подія", "Ініціалізація: " + e.ToString());
				   }
			  }
			else if (list->Strings[0] == "#instance_request")
			  {
				Log->Add("AMRA: надійшов запит статусу ініціалізації");
				SendToCollector("AMRA", "надійшов запит статусу ініціалізації");

				if (Initialised)
				  SendToGuardian("#instance_online");
				else
                  SendToGuardian("#instance_offline");
			  }
			else
			  throw Exception("невідомі дані: " + list->Strings[0]);
		  }
	   catch (Exception &e)
		  {
			Log->Add("AMRA: " + e.ToString());
			SendToCollector("AMRA", e.ToString());
		  }
	 }
  __finally {delete list; delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopWork()
{
  try
	 {
	   SwitchOff->Visible = false;
	   SwitchOn->Visible = true;
	   LbStatus->Caption = "Зупинено";
	   LbStatus->Font->Color = clRed;
	   LbStatus->Tag = 0;

	   TExchangeConnect *srv;

	   for (int i = 0; i < SrvList->Count; i++)
		  EndWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));

	   SendToGuardian("#manager_stopped");
	 }
  catch (Exception &e)
	 {
	   Log->Add("Зупинка роботи: " + e.ToString());
	   SendToCollector("подія", "Зупинка роботи: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartWork()
{
  try
	 {
	   SwitchOn->Visible = false;
	   SwitchOff->Visible = true;
	   LbStatus->Caption = "Робота";
	   LbStatus->Font->Color = clLime;
	   LbStatus->Tag = 1;

	   TExchangeConnect *srv;

	   for (int i = 0; i < SrvList->Count; i++)
		  RunWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));
	 }
  catch (Exception &e)
	 {
	   Log->Add("Початок роботи: " + e.ToString());
	   SendToCollector("подія", "Початок роботи: " + e.ToString());

	   throw new Exception("Помилка запуску!");
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

TAMThread* __fastcall TMainForm::FindServerThread(unsigned int thread_id)
{
  for (int i = 0; i < ThreadList->Count; i++)
	 {
	   TAMThread *th = (TAMThread*)ThreadList->Items[i];

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
	   TAMThread *th = (TAMThread*)ThreadList->Items[ind];

	   if (th->ThreadID == id)
		 {
		   ThreadList->Delete(ind);
		   delete th;

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
	   TAMThread *th = (TAMThread*)ThreadList->Items[ind];

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
			  TAMThread *serv_thread = new TAMThread(true);
			  serv_thread->InfoIcon = TrayIcon1;
			  serv_thread->Connection = server;
			  server->ServerThreadID = serv_thread->ThreadID;
			  ThreadList->Add(serv_thread);
			  serv_thread->Resume();
			  server->Start();
			}
		  else
			{
			  Log->Add("З'єднання з ID " + IntToStr(server->ServerID) + " не ініціалізоване!");
			  SendToCollector("подія", "З'єднання з ID " + IntToStr(server->ServerID) + " не ініціалізоване!");
			  Log->SaveToFile(LogPath + "\\" + LogName);
            }
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("подія", "Помилковий вказівник TExchangeConnect*");
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
		  SendToCollector("подія", "Помилковий вказівник TExchangeConnect*");
        }
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("подія", "Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EndWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (server->Working())
		server->Stop();

	  TAMThread *th = FindServerThread(server->ServerThreadID);

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
		  SendToCollector("подія", "Помилковий ID потоку " + server->ServerCaption +
						  ", поток: " + IntToStr((int)server->ServerThreadID));
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	  SendToCollector("подія", "Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::UpdateGuardian()
{
  Log->Add("Спроба оновити Guardian");
  SendToCollector("подія", "Спроба оновити Guardian");

  if (!DirectoryExists(GuardianUpdatesPath))
	{
	  Log->Add("Шлях до файлів оновлень не існує, відмова");
	  SendToCollector("подія", "Шлях до файлів оновлень не існує, відмова");

	  return -1;
    }

  if (GuardianRunning())
	{
      Log->Add("Guardian в роботі, відмова");
	  SendToCollector("подія", "Guardian в роботі, відмова");

	  return -1;
	}

  int result = 0;

  TStringList *files = new TStringList();
  TStringList *upd_files = new TStringList();

  try
	 {
	   try
		  {
			String file_name;

			GetFileList(files, GuardianUpdatesPath, "*.*", WITHOUT_DIRS, WITH_FULL_PATH);

            for (int i = 0; i < files->Count; i++)
			   {
				 file_name = files->Strings[i];
				 file_name.Delete(1, file_name.LastDelimiter("\\"));

				 upd_files->Add(AppPath + "\\" + file_name);

				 if (FileExists(AppPath + "\\" + file_name))
				   {
//порівняємо версії обох файлів
					 int comp_ver = CompareVersions(files->Strings[i].c_str(),
													String(AppPath + "\\" + file_name).c_str());

					 TDateTime date1 = GetFileDateTime(files->Strings[i]); //завантажений
					 TDateTime date2 = GetFileDateTime(AppPath + "\\" + file_name); //існуючий

//якщо версія або дата завантаженого файлу більша за дату існуючого - оновимо існуючий файл
					 if (comp_ver == VER_1_HIGHER)
					   {
                         RenameFile(AppPath + "\\" + file_name, AppPath + "\\" + file_name + ".bckp");
						 DeleteFile(AppPath + "\\" + file_name);

						 if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
						   {
//сповіщення про невдале копіювання
							 Log->Add("Помилка при копіювання файлу " + files->Strings[i] +
									 " до " + AppPath + "\\" + file_name);
							 SendToCollector("файли", "Помилка при копіювання файлу " + files->Strings[i] +
											 " до " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
					   }
					 else if ((comp_ver == -1) || (date1 > date2))
					   {
						 RenameFile(AppPath + "\\" + file_name, AppPath + "\\" + file_name + ".bckp");
						 DeleteFile(AppPath + "\\" + file_name);

						 if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
						   {
//сповіщення про невдале копіювання
							 Log->Add("Помилка при копіювання файлу " + files->Strings[i] +
									 " до " + AppPath + "\\" + file_name);
							 SendToCollector("файли", "Помилка при копіювання файлу " + files->Strings[i] +
											 " до " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
					   }
					 else
					   {
						 Log->Add("Файл " + AppPath + "\\" + file_name + " не потребує оновлення");
						 SendToCollector("файли", "Файл " + AppPath + "\\" + file_name + " не потребує оновлення");
					   }
				   }
				 else if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
				   {
					 //сповіщення про невдале копіювання
					 Log->Add("Помилка при копіювання файлу " + files->Strings[i] +
							 " до " + AppPath + "\\" + file_name);
					 SendToCollector("подія", "Помилка при копіювання файлу " + files->Strings[i] +
									  " до " + AppPath + "\\" + file_name);
					 result = -2;
                     break;
				   }
			   }

			if (result < 0) //якщо були помилки - відновимо попередні файли
			  {
				Log->Add("Виявлені помилки при оновленні, відновлення попередніх версій файлів...");
				SendToCollector("подія", "Виявлені помилки при оновленні, відновлення попередніх версій файлів...");

				for (int i = 0; i < upd_files->Count; i++) //видалимо скопійовані нові файли
				   {
					 if (FileExists(upd_files->Strings[i]))
					   DeleteFile(upd_files->Strings[i]);
                   }

				files->Clear();

				GetFileList(files, AppPath, "*.bckp", WITHOUT_DIRS, WITH_FULL_PATH);

				for (int i = 0; i < files->Count; i++) //відновимо старі
				   {
					 file_name = files->Strings[i];
					 file_name.Delete(file_name.LastDelimiter("."), 5);
					 RenameFile(files->Strings[i], file_name);
					 DeleteFile(files->Strings[i]);
				   }
			  }
			else
			  {
				files->Clear();
				GetFileList(files, AppPath, "*.bckp", WITHOUT_DIRS, WITH_FULL_PATH);

				for (int i = 0; i < files->Count; i++) //видалимо бекапи файлів
				   DeleteFile(files->Strings[i]);

				/*files->Clear();
				GetFileList(files, GuardianUpdatesPath, "*.*", WITHOUT_DIRS, WITH_FULL_PATH);

                for (int i = 0; i < files->Count; i++) //видалимо завантажені модулі
				   DeleteFile(files->Strings[i]);*/
			  }
		  }
	   catch(Exception &e)
		  {
			Log->Add("Оновлення Guardian: помилка " + e.ToString());
			SendToCollector("подія", "Оновлення Guardian: помилка " + e.ToString());
			result = -3;
          }
	 }
  __finally { delete files; delete upd_files;}

  return result;
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::GuardianRunning()
{
  DWORD MngrPID = GetProcessByExeName(L"ArmMngrGuard.exe");

  if (MngrPID == 0)
	return false;
  else
   return true;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RunGuardian()
{
  if (FileExists(AppPath + "\\ArmMngrGuard.exe"))
	{
	  Log->Add("Запуск Guardian");
	  SendToCollector("подія", "Запуск Guardian");

	  ShellExecute(NULL,
				   L"open",
				   String(AppPath + "\\ArmMngrGuard.exe").c_str(),
				   L"",
				   NULL,
				   SW_SHOWNORMAL);

	  if (GuardianRunning())
		return 0;
	  else
		return -1;
	}
  else
	{
	  Log->Add("Не вдалося запустити Guardian: відсутній файл");
	  SendToCollector("подія", "Не вдалося запустити Guardian: відсутній файл");

      return -1;
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
	CreateDir(LogPath);

  if (!DirectoryExists(ConnPath))
	CreateDir(ConnPath);

  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + ".log";

  if (FileExists(LogPath + "\\" + LogName))
	Log->LoadFromFile(LogPath + "\\" + LogName);

  if (ReadConfig() == 0)
	{
	  ShowDlLog();
	  DlLog->Hide();

	  ShowUlLog();
	  UlLog->Hide();

	  if (!FirewallRule)
		AddFirewallRule();

	  AURAServer->DefaultPort = RemAdmPort;
	  AURAServer->Active = true;

	  AMColClient->Host = CollectorHost;
	  AMColClient->Port = CollectorPort;

	  if (EnableAutoStart)
		{
		  if (!AddAppAutoStart("ArmFileManager", Application->ExeName, AutoStartForAllUsers))
			{
			  Log->Add("Не вдалося створити запис у реєстрі для автозапуску");
			  SendToCollector("подія", "Не вдалося створити запис у реєстрі для автозапуску");
			}
		}
	  else
		{
		  if (!RemoveAppAutoStart("ArmFileManager", AutoStartForAllUsers))
			{
			  Log->Add("Не вдалося видалити запис автозапуску з реєстру");
			  SendToCollector("подія", "Не вдалося видалити запис автозапуску з реєстру");
			}
		}

	  if (AutoStartForAllUsers && CheckAppAutoStart("ArmFileManager", FOR_CURRENT_USER))
		{
		  if (!RemoveAppAutoStart("ArmFileManager", FOR_CURRENT_USER))
			{
			  Log->Add("Не вдалося видалити запис автозапуску з реєстру HKCU");
			  SendToCollector("подія", "Не вдалося видалити запис автозапуску з реєстру HKCU");
			}
		}

	  SaveLogTimer->Enabled = true;
//підписуємось на отримання повідомлень WM_WTSSESSION_CHANGE
//щоб зупиняти Менеджер, коли користувач виходить з обліковки
	  if (!WTSRegisterSessionNotification(this->Handle, NOTIFY_FOR_THIS_SESSION))
		{
		  throw new Exception("WTSRegisterSessionNotification() fail: " + IntToStr((int)GetLastError()));
        }

	  if (HideWnd == 1)
		this->WindowState = wsMinimized;

	  if (UseGuardian)
		{
		  if (UpdateGuardian() == 0)
			{
			  Log->Add("Guardian оновлено");
			  SendToCollector("подія", "Guardian оновлено");
			}

		  if (!GuardianRunning())
			{
			  if (RunGuardian() < 0)
				{
				  Log->Add("Помилка під час запуску Guardian");
				  SendToCollector("подія", "Помилка під час запуску Guardian");
				}
			}

          Log->Add("Запит до Guardian на дозвіл запуску");
		  SendToCollector("подія", "Запит до Guardian на дозвіл запуску");

		  SendToGuardian("#init_request");
		}
	  else
		{
		  try
			 {
			   InitInstance();
			 }
		  catch (Exception &e)
			 {
			   Log->Add("Ініціалізація: " + e.ToString());
			   SendToCollector("подія", "Ініціалізація: " + e.ToString());
			 }
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

	  if (SendReportToMail)
		SendLog(MailTo, MailSubjectErr, MailFrom, Log->GetText());
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AddFirewallRule()
{
  try
	 {
	   AnsiString cmd;

	   if (system("netsh advfirewall firewall show rule name=\"ArmMngr\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

           cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(200);
		 }

       if (system("netsh advfirewall firewall show rule name=\"ArmMngr\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

           cmd = "netsh advfirewall firewall add rule name=\"ArmMngr\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(200);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"AMRA\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(200);
		 }

       if (system("netsh advfirewall firewall show rule name=\"AMRA\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(200);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"AMCOL\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=in action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=in action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=in action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(200);
		 }

       if (system("netsh advfirewall firewall show rule name=\"AMCOL\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=out action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=out action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(200);

		   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=out action=allow protocol=TCP localport=" + IntToStr(CollectorPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		 }

	   if ((system("netsh advfirewall firewall show rule name=\"ArmMngr\" dir=in") == 0) &&
		   (system("netsh advfirewall firewall show rule name=\"ArmMngr\" dir=out") == 0))
		 {
		   FirewallRule = true;
		   SetConfigLine(AppPath + "\\main.cfg", "FirewallRule", "1");
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Створення правила для файрволу: " + e.ToString());
	   SendToCollector("Створення правила для файрволу: ", e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::InitInstance()
{
  try
	 {
	   if (ConnectELI() == 0)
		 {
		   ExecuteScript("admin.es");

		   EliThread = new TAMEliThread(true);

		   EliThread->ScriptPath = AppPath + "\\" + ControlScript;
		   EliThread->Logging = ScriptLog;
		   EliThread->SetRunInterval(ScriptInterval);
		   EliThread->ELIInterface = eIface;
		   EliThread->Resume();
		   		   
		   Log->Add("ELI: очікування на командний скрипт");
		   SendToCollector("ELI", "Очікування на командний скрипт");
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ініціалізація та запуск: " + e.ToString());
	   SendToCollector("ELI", "Ініціалізація та запуск: " + e.ToString());
	 }

  try
	 {
       FileCountTimer->Enabled = true;
	   CreateServers();

	   Log->Add("Початок роботи");
       SendToCollector("подія", "Початок роботи");
	   Log->Add("Версія модулю: " + AppVersion);

	   StartWork();

	   TrayIcon1->BalloonFlags = bfInfo;
	   TrayIcon1->BalloonHint = "Менеджер обміну файлами АРМ ВЗ запущено";
	   TrayIcon1->ShowBalloonHint();

	   Initialised = true;
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ініціалізація: " + e.ToString());
	   SendToCollector("подія", "Ініціалізація: " + e.ToString());

       Initialised = false;

	   throw new Exception("Помилка ініціалізації!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopInstance()
{
  try
	 {
	   if (EliThread)
		 {
		   EliThread->Terminate();

		   while (!EliThread->Finished)
			 Sleep(100);

		   delete EliThread;
		 }

	   ReleaseELI();

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
	   SendToCollector("подія", "Кінець роботи");

	   if (SendReportToMail)
		 SendLog(MailTo, MailSubjectOK, MailFrom, Log->GetText());
	 }
  catch (Exception &e)
	 {
	   Log->Add("Завершення роботи: " + e.ToString());
	   SendToCollector("подія", "Завершення роботи: " + e.ToString());

	   throw new Exception("Помилка завершення роботи!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPPConnClick(TObject *Sender)
{
  TMenuItem *menu = reinterpret_cast<TMenuItem*>(Sender);
  TExchangeConnect *srv;
  TAMThread *th;

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
	   SendToCollector("подія", "Ручний запуск обміну: " + e.ToString());
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
	   Log->Add(e.ToString());
	   SendToCollector("подія", e.ToString());
	 }

//відписуємось від розсилки повідомлень WM_WTSSESSION_CHANGE
  WTSUnRegisterSessionNotification(this->Handle);

  AURAServer->Active = false;
  SaveLogTimer->Enabled = false;

  Log->SaveToFile(LogPath + "\\" + LogName);

  delete Log;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendStatus(TIdContext *AContext)
{
  Log->Add("AMRA: запит статусу");
  SendToCollector("AMRA", "Запит статусу");

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
  SendToCollector("AMRA", "Запит конфігурації");

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
			  ls->LoadFromFile(AppPath + "\\main.cfg");
			else
			  {
				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  ls->LoadFromFile(srv->ServerCfgPath);
				else
				  throw Exception("Невідомий ID з'єднання: " + list->Strings[2]);
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
			SendToCollector("AMRA", e.ToString());
		  }
     }
  __finally {delete ls; delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendLog(TIdContext *AContext)
{
  Log->Add("AMRA: запит логу");
  SendToCollector("AMRA", "Запит логу");

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
  SendToCollector("AMRA", "Запит списку з'єднань");

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
  SendToCollector("AMRA", "Запит списку потоків");

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
  SendToCollector("AMRA", "Запит файлу " + list->Strings[2]);

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
  SendToCollector("AMRA", "Запит версії");

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
	   SendToCollector("AMRA", e.ToString());

	   return -1;
	 }

  return 0;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ConnectELI()
{
  int res;
  dllhandle = LoadLibrary(String(AppPath + "\\ELI.dll").c_str());

  if (!dllhandle)
	{
	  Log->Add("Помилка ініціалізації інтерфейсу ELI_INTERFACE");
	  SendToCollector("ELI", "Помилка ініціалізації інтерфейсу ELI_INTERFACE");
	  res = -1;
	}
  else
	{
	  GetELI = (GETELIINTERFACE) GetProcAddress(dllhandle, "GetELIInterface");
	  FreeELI = (FREEELIINTERFACE) GetProcAddress(dllhandle, "FreeELIInterface");

	  if (!GetELI)
		{
		  Log->Add("Помилка ініціалізації GetELI");
		  SendToCollector("ELI", "Помилка ініціалізації GetELI");
		  res = -2;
		}
	  else if (!FreeELI)
		{
		  Log->Add("Помилка ініціалізації FreeELI");
		  SendToCollector("ELI", "Помилка ініціалізації FreeELI");
		  res = -3;
		}
	  else if (!GetELI(&eIface))
		{
		  Log->Add("Помилка ініціалізації інтерфейсу ELI_INTERFACE");
		  SendToCollector("ELI", "Помилка ініціалізації інтерфейсу ELI_INTERFACE");
		  res = -4;
		}
	  else
		{
		  Log->Add("Інтерфейс ELI_INTERFACE ініціалізований");
		  SendToCollector("ELI", "Інтерфейс ELI_INTERFACE ініціалізований");
          LoadFunctionsToELI();
		  res = 0;
		}
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReleaseELI()
{
  int res;

  if (eIface)
	{
      if (!FreeELI(&eIface))
		{
		  Log->Add("Помилка звільнення інтерфейсу ELI_INTERFACE");
		  SendToCollector("ELI", "Помилка звільнення інтерфейсу ELI_INTERFACE");

		  res = -1;
		}
	  else
		{
		  Log->Add("Інтерфейс ELI_INTERFACE вивільнений");
		  SendToCollector("ELI", "Інтерфейс ELI_INTERFACE вивільнений");
		  res = 0;
		}
	}

  FreeLibrary(dllhandle);

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ExecuteScript(String ctrl_script_name)
{
  if (FileExists(AppPath + "\\" + ctrl_script_name))
	{
	  try
		 {
		   Log->Add("ELI: запуск скрипту " + AppPath + "\\" + ctrl_script_name);
		   SendToCollector("ELI", "Запуск скрипту " + AppPath + "\\" + ctrl_script_name);

		   eIface->RunScriptFromFile(String(AppPath + "\\" + ctrl_script_name).c_str(),
									 L"",
									 ScriptLog);

		   DeleteFile(AppPath + "\\" + ctrl_script_name);

		   TStringList *lst = new TStringList();

		   try
			  {
				StrToList(lst, eIface->ShowInfoMessages(), "\r\n");

				for (int i = 0; i < lst->Count; i++)
				   Log->Add(lst->Strings[i]);
			  }
		   __finally {delete lst;}
		 }
	  catch (Exception &e)
		 {
		   Log->Add("ELI: помилка виконання скрипту " + e.ToString());
		   SendToCollector("ELI", "Помилка виконання скрипту " + e.ToString());
		 }
	}
  else
	{
	  Log->Add("ELI: відсутній файл керуючого скрипта " + AppPath + "\\" + ctrl_script_name);
	  SendToCollector("ELI", "Відсутній файл керуючого скрипта " + AppPath + "\\" + ctrl_script_name);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LoadFunctionsToELI()
{
  eIface->AddFunction(L"_ConnCount", L"", &eConnectionsCount);
  eIface->AddFunction(L"_CreateConn", L"sym pFile,num pAddMenu", &eCreateConnection);
  eIface->AddFunction(L"_DestroyConn", L"num pID", &eDestroyConnection);
  eIface->AddFunction(L"_RemoveConn", L"num pID", &eRemoveConnection);
  eIface->AddFunction(L"_StartConn", L"num pID", &eStartConnection);
  eIface->AddFunction(L"_StopConn", L"num pID", &eStopConnection);
  eIface->AddFunction(L"_ConnID", L"sym pCap", &eConnectionID);
  eIface->AddFunction(L"_ConnIDInd", L"num pIndex", &eConnectionIDInd);
  eIface->AddFunction(L"_ConnStatus", L"num pID", &eConnectionStatus);
  eIface->AddFunction(L"_ConnCfgPath", L"num pID", &eConnectionCfgPath);
  eIface->AddFunction(L"_ReloadCfg", L"", &eReloadConfig);
  eIface->AddFunction(L"_SendCount", L"", &eSendFileCount);
  eIface->AddFunction(L"_RecvCount", L"", &eRecvFileCount);
  eIface->AddFunction(L"_ReadCfg", L"sym pFile,sym pPrm", &eReadFromCfg);
  eIface->AddFunction(L"_RemCfg", L"sym pFile,sym pPrm", &eRemoveFromCfg);
  eIface->AddFunction(L"_WriteCfg", L"sym pFile,sym pPrm,sym pVal", &eWriteToCfg);
  eIface->AddFunction(L"_WriteConnCfg", L"num pID,sym pPrm,sym pVal", &eWriteToCfgByID);
  eIface->AddFunction(L"_SendToCollector", L"sym pMsg", &eSendMsgToCollector);
  eIface->AddFunction(L"_WriteToLog", L"sym pMsg", &eWriteMsgToLog);
  eIface->AddFunction(L"_MngrAppPath", L"", &eGetAppPath);
  eIface->AddFunction(L"_ShutdownMngr", L"", &eShutdownManager);
  eIface->AddFunction(L"_ShutdownGuard", L"", &eShutdownGuardian);
  eIface->AddFunction(L"_StartGuard", L"", &eStartGuardian);
  eIface->AddFunction(L"_RestartGuard", L"", &eRestartGuardian);
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GenConnectionID()
{
  TExchangeConnect *srv;
  int id = 0;

  for (int i = 0; i < SrvList->Count; i++)
	 {
	   srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);
	   
	   if (srv->ServerID > id)
		 id = srv->ServerID;
	 }

  return id + 1;
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TMainForm::CreateConnection(String file)
{
  try
	 {
	   TExchangeConnect *srv = new TExchangeConnect(AMColClient,
													UseCollector,
													TrayIcon1,
													Log,
													file,
													GenConnectionID());

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
	   srv_menu->OnClick = IconPPConnClick;

	   MenuItemList->Add(srv_menu);
	   SrvList->Add(srv);

	   RunWork(srv);
	 }
  catch (Exception &e)
	 {
	   Log->Add("CreateConnection: " + e.ToString());
	   SendToCollector("подія", "CreateConnection: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::CreateConnection(String file, bool create_menu)
{
  Log->Add("ELI: Створення з'єднання");
  SendToCollector("ELI", "Створення з'єднання");

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = AppPath + file;
	}

  if (!FileExists(file))
    return -1;

  TExchangeConnect *srv = new TExchangeConnect(AMColClient,
											   UseCollector,
											   TrayIcon1,
											   Log,
											   file,
											   GenConnectionID());

  srv->UsedCollectorHost = CollectorHost;
  srv->UsedCollectorPort = CollectorPort;
  srv->DownloadFileList = DList;
  srv->UploadfileList = UList;

  if (create_menu)
	{
      TMenuItem *srv_menu = new TMenuItem(PopupMenu1);

	  srv_menu->Caption = "Запустити " + srv->ConnectionConfig->Caption;
	  srv_menu->Hint = IntToStr(srv->ServerID);
	  IconPP5->Add(srv_menu);
	  IconPP5->SubMenuImages = ImageList1;
	  srv_menu->ImageIndex = 4;
	  srv_menu->OnClick = IconPPConnClick;

	  MenuItemList->Add(srv_menu);
    }

  SrvList->Add(srv);

  return srv->ServerID;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DestroyConnection(int id)
{
  Log->Add("ELI: Знищення з'єднання");
  SendToCollector("ELI", "Знищення з'єднання");

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  EndWork(srv);
      SrvList->Delete(SrvList->IndexOf(srv));

	  delete srv;
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RemoveConnection(int id)
{
  Log->Add("ELI: Знищення з'єднання та видалення конфігу");
  SendToCollector("ELI", "Знищення з'єднання та видалення конфігу");

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  EndWork(srv);
	  SrvList->Delete(SrvList->IndexOf(srv));
	  DeleteFile(srv->ServerCfgPath);

	  delete srv;
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartConnection(int id)
{
  Log->Add("ELI: Запуск з'єднання");
  SendToCollector("ELI", "Запуск з'єднання");

  RunWork(FindServer(id));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopConnection(int id)
{
  Log->Add("ELI: Зупинка з'єднання");
  SendToCollector("ELI", "Зупинка з'єднання");

  EndWork(FindServer(id));
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

int __fastcall TMainForm::GetConnectionID(int index)
{
  int res = 0;

  if ((index >= 0) && (index < SrvList->Count))
	{
	  TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[index]);
	  res = srv->ServerID;
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ConnectionStatus(int id)
{
  int res;
  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  if (srv->Initialized() && srv->Working())
		res = 1;
	  else if (srv->Initialized() && !srv->Working())
		res = 0;
	  else if (!srv->Initialized())
        res = -1;
	}

  return res;
}
//---------------------------------------------------------------------------

String __fastcall TMainForm::ConnectionCfgPath(int id)
{
  String res = "";
  TExchangeConnect *srv = FindServer(id);

  if (srv)
	res = srv->ServerCfgPath;

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ReloadConfig()
{
  Log->Add("ELI: Команда оновлення конфігів");
  SendToCollector("ELI", "Команда оновлення конфігів");

  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = "Дані з конфігу оновлені";
  TrayIcon1->ShowBalloonHint();
  ReadConfig();
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::SendFileCount()
{
  Log->Add("ELI: Запит кількості переданих файлів");
  SendToCollector("ELI", "Запит кількості переданих файлів");

  return UList->Lines->Count;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RecvFileCount()
{
  Log->Add("ELI: Запит кількості отриманих файлів");
  SendToCollector("ELI", "Запит кількості отриманих файлів");

  return DList->Lines->Count;
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::RemoveFromCfg(String file, String param)
{
  Log->Add("ELI: Читання параметру " + param + " з файлу " + file);
  SendToCollector("ELI", "Читання параметру " + param + " з файлу " + file);

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = AppPath + file;
	}

  if (!FileExists(file))
	return false;
  else
	return RemConfigLine(file, param);
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::WriteToCfg(String file, String param, String val)
{
  Log->Add("ELI: Запис параметру " + param + " у файл " + file);
  SendToCollector("ELI", "Запис параметру " + param + " у файл " + file);

  bool res;

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = AppPath + file;
	}

  if (!FileExists(file))
	res = false;
  else
	{
      if (GetConfigLineInd(file, param) > -1)
		res = SetConfigLine(file, param, val);
	  else
		res = AddConfigLine(file, param, val);
    }

  return res;
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::WriteToCfg(int id, String param, String val)
{
  Log->Add("ELI: Запис параметру " + param + " у конфіг підключення з ID = " + id);
  SendToCollector("ELI", "Запис параметру " + param + " у конфіг підключення з ID = " + id);

  bool res;

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  if (GetConfigLineInd(srv->ServerCfgPath, param) > -1)
		res = SetConfigLine(srv->ServerCfgPath, param, val);
	  else
		res = AddConfigLine(srv->ServerCfgPath, param, val);
	}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SendMsgToCollector(String msg)
{
  SendToCollector("ELI", msg);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteToMngrLog(String msg)
{
  Log->Add("ELI: " + msg);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownManager()
{
  Log->Add("ELI: Отримано команду shutdown");
  SendToCollector("ELI", "Отримано команду shutdown");
  PostMessage(Application->Handle, WM_QUIT, 0, 0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownGuardian()
{
  Log->Add("ELI: Отримано команду зупинки Guardian");
  SendToCollector("ELI", "Отримано команду зупинки Guardian");
  PostMessage(FindHandleByName(L"Guardian Менеджера обміну файлами АРМ ВЗ"), WM_QUIT, 0, 0);
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::StartGuardian()
{
  Log->Add("ELI: Отримано команду запуску Guardian");
  SendToCollector("ELI", "Отримано команду запуску Guardian");

  return RunGuardian();
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RestartGuardian()
{
  Log->Add("ELI: Отримано команду перезапуску Guardian");
  SendToCollector("ELI", "Отримано команду перезапуску Guardian");

  PostMessage(FindHandleByName(L"Guardian Менеджера обміну файлами АРМ ВЗ"), WM_QUIT, 0, 0);

  Sleep(1000);

  return RunGuardian();
}
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[10];
  swprintf(res, L"%d", MainForm->Connections->Count);

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eCreateConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  bool menu = (bool)ep->GetParamToInt(L"pAddMenu");
  wchar_t res[3];

  try
	 {
	   int id = MainForm->CreateConnection(ep->GetParamToStr(L"pFile"), menu);

	   wcscpy(res, IntToStr(id).c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eCreateConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eDestroyConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->DestroyConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eDestroyConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eRemoveConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->RemoveConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eRemoveConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eStartConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->StartConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStartConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eStopConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->StopConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStopConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionID(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToStr(L"pCap")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionID(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionIDInd(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pIndex")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionIDInd(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionStatus(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pID")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionStatus(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionCfgPath(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  String res;

  try
	 {
	   res = MainForm->ConnectionCfgPath(ep->GetParamToInt(L"pID"));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionCfgPath(): " + e.ToString());
	   res = "";
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
}
//---------------------------------------------------------------------------

void __stdcall eReloadConfig(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ReloadConfig();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eSendFileCount(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  try
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(),
							 IntToStr(MainForm->SendFileCount()).c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eSendFileCount(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-1");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eRecvFileCount(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  try
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(),
							 IntToStr(MainForm->RecvFileCount()).c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eRecvFileCount(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-1");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eReadFromCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  String res;

  if (ReadParameter(ep->GetParamToStr(L"pFile"),
					ep->GetParamToStr(L"pPrm"),
					&res,
					TT_TO_STR) == RP_OK)
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
	}
  else
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
}
//---------------------------------------------------------------------------

void __stdcall eRemoveFromCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (MainForm->RemoveFromCfg(ep->GetParamToStr(L"pFile"),
							  ep->GetParamToStr(L"pPrm")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	}
  else
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (!MainForm->WriteToCfg(ep->GetParamToStr(L"pFile"),
							ep->GetParamToStr(L"pPrm"),
							ep->GetParamToStr(L"pVal")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
  else
	ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfgByID(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (!MainForm->WriteToCfg(ep->GetParamToInt(L"pID"),
							ep->GetParamToStr(L"pPrm"),
							ep->GetParamToStr(L"pVal")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
  else
	ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
}
//---------------------------------------------------------------------------

void __stdcall eSendMsgToCollector(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  try
	 {
	   MainForm->SendMsgToCollector(ep->GetParamToStr(L"pMsg"));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eSendMsgToCollector(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eWriteMsgToLog(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  try
	 {
	   MainForm->WriteToMngrLog(ep->GetParamToStr(L"pMsg"));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (...)
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eGetAppPath(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  ep->SetFunctionResult(ep->GetCurrentFuncName(), AppPath.c_str());
}
//---------------------------------------------------------------------------

void __stdcall eShutdownManager(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ShutdownManager();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eShutdownGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ShutdownGuardian();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eStartGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->StartGuardian());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStartGuardian(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eRestartGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->RestartGuardian());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eRestartGuardian(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WndProc(Messages::TMessage& Msg)
{
  if (Msg.Msg == WM_WTSSESSION_CHANGE)
	{
	  if (Msg.WParam == WTS_CONSOLE_DISCONNECT )
		{
		  Log->Add("WTS_CONSOLE_DISCONNECT");
		  SendToCollector("подія", "WTS_CONSOLE_DISCONNECT");

		  try
			 {
			   ShutdownGuardian();
			   StopWork();
			   AURAServer->Active = false;
			 }
		  catch (Exception &e)
			 {
			   Log->Add("WTS_CONSOLE_CONNECT: " + e.ToString());
			   SendToCollector("подія", "WTS_CONSOLE_CONNECT: " + e.ToString());
			 }
		}
	  else if (Msg.WParam == WTS_CONSOLE_CONNECT)
		{
		  Log->Add("WTS_CONSOLE_CONNECT");
		  SendToCollector("подія", "WTS_CONSOLE_CONNECT");

		  try
			 {
               StartGuardian();
			   StartWork();
			   AURAServer->Active = true;
			 }
		  catch (Exception &e)
			 {
			   Log->Add("WTS_CONSOLE_CONNECT: " + e.ToString());
			   SendToCollector("подія", "WTS_CONSOLE_CONNECT: " + e.ToString());
			 }
		}
	}

  TForm::WndProc(Msg);
}
//---------------------------------------------------------------------------


