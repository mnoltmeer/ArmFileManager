/*!
Copyright 2019 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "..\..\MyFunc\MyFunc.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

int ListenPort, RequestPort, DBPort, PoolSize, PoolMonitoring;
String DBHost, DBPath, FormCaption, LogName, AppPath;
TStringList *ParamsPool;
int TimerCnt;
bool Processing;

const wchar_t * sqlheader = L"SELECT * FROM LOGS lg WHERE ";
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  ParamsPool = new TStringList();

  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));
}
//---------------------------------------------------------------------------

void TMainForm::ConnectToDB()
{
  if (!StatConnection->Connected)
	{
	  DBHost = GetConfigLine(AppPath + "\\server.cfg", "DBHost");
	  DBPath = GetConfigLine(AppPath + "\\server.cfg", "DBPath");

	  ActTransCnt->Caption = "0";
	  SucsTransCnt->Caption = "0";
	  RollTransCnt->Caption = "0";

	  StatConnection->Params->Clear();
	  StatConnection->Params->Add("User_Name=sysdba");
	  StatConnection->Params->Add("Password=masterkey");
	  StatConnection->Params->Add("Port=3050");
	  StatConnection->Params->Add("DriverID=FB");
	  StatConnection->Params->Add("Protocol=TCPIP");
	  StatConnection->Params->Add("Database=" + DBPath);
	  StatConnection->Params->Add("Server=" + DBHost);

	  try
		 {
           ProcessingTimer->Interval = PoolMonitoring;
		   StatConnection->Connected = true;
		   ProcessingTimer->Enabled = true;
		   ShowDBConn();
         }
	  catch (Exception &e)
		 {
		   LastRecLog->Lines->Add(e.ToString());
		   ShowDBNoConn();
		 }
	}
}
//---------------------------------------------------------------------------

void TMainForm::DisconnectFromDB()
{
  if (StatConnection->Connected)
	{
	  try
		 {
           ShowDBNoConn();
		   WriteQuery->Active = false;
		   StatConnection->Connected = false;
		   ProcessingTimer->Enabled = false;
         }
	  catch (Exception &e)
		 {
		   Processing = false;
		   LastRecLog->Lines->Add(e.ToString());
		   ProcessingTimer->Enabled = false;
		 }
	}
}
//---------------------------------------------------------------------------

void TMainForm::ShowServerCon()
{
  SwServerOn->Hide();
  SwServerOff->Show();
}
//---------------------------------------------------------------------------

void TMainForm::ShowServerNoCon()
{
  SwServerOn->Show();
  SwServerOff->Hide();
}
//---------------------------------------------------------------------------

void TMainForm::ShowDBConn()
{
  SwDBOn->Hide();
  SwDBOff->Show();
}
//---------------------------------------------------------------------------

void TMainForm::ShowDBNoConn()
{
  SwDBOn->Show();
  SwDBOff->Hide();
}
//---------------------------------------------------------------------------

void TMainForm::ShowHttpCon()
{
  SwHttpOn->Hide();
  SwHttpOff->Show();
}
//---------------------------------------------------------------------------

void TMainForm::ShowHttpNoCon()
{
  SwHttpOn->Show();
  SwHttpOff->Hide();
}
//---------------------------------------------------------------------------

void TMainForm::StartServer()
{
  try
	 {
	   AMColServer->DefaultPort = ListenPort;
	   AMColServer->Active = true;
       ShowServerCon();
     }
  catch (Exception &e)
	 {
	   LastRecLog->Lines->Add(e.ToString());
	   ShowServerNoCon();
     }
}
//---------------------------------------------------------------------------

void TMainForm::StopServer()
{
  try
	 {
	   AMColServer->Active = false;
	   ShowServerNoCon();
     }
  catch (Exception &e)
	 {
	   LastRecLog->Lines->Add(e.ToString());
	   ShowServerNoCon();
	 }
}
//---------------------------------------------------------------------------

void TMainForm::StartHttpServer()
{
  try
	 {
	   HttpServer->DefaultPort = RequestPort;
	   HttpServer->Active = true;
	   ShowHttpCon();
     }
  catch (Exception &e)
	 {
	   LastRecLog->Lines->Add(e.ToString());
	   ShowHttpNoCon();
     }
}
//---------------------------------------------------------------------------

void TMainForm::StopHttpServer()
{
  try
	 {
	   HttpServer->Active = false;
	   ShowHttpNoCon();
     }
  catch (Exception &e)
	 {
	   LastRecLog->Lines->Add(e.ToString());
	   ShowHttpNoCon();
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AMColServerExecute(TIdContext *AContext)
{
  String msg;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

  try
	 {
       AContext->Connection->IOHandler->ReadStream(ms);
	   ms->Position = 0;
	   msg = ms->ReadString(ms->Size);
	   ParamsPool->Add(msg);
       PoolCount->Caption = ParamsPool->Count;

       msg = ParseString(msg, "&", " | ");
	   LastRecLog->Lines->Add(msg);

	   if (LastRecLog->Lines->Count > 30)
		 LastRecLog->Lines->Delete(0);

	   SendMessage(LastRecLog->Handle, WM_VSCROLL, SB_BOTTOM, 0);
	 }
  catch (Exception &e)
	 {
	   LastRecLog->Lines->Add(e.ToString());
	 }

  delete ms;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwServerOnClick(TObject *Sender)
{
  StartServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwDBOnClick(TObject *Sender)
{
  ConnectToDB();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwServerOffClick(TObject *Sender)
{
  StopServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwDBOffClick(TObject *Sender)
{
  DisconnectFromDB();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwHttpOnClick(TObject *Sender)
{
  StartHttpServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwHttpOffClick(TObject *Sender)
{
  StopHttpServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  DisconnectFromDB();
  StopHttpServer();
  StopServer();
  delete ParamsPool;
}
//---------------------------------------------------------------------------

void TMainForm::ProcessingMessages()
{
  while (Processing)
	{
	  if (TimerCnt >= 10)
		{
          if (ParamsPool->Count == 0)
			continue;
		  else if (ParamsPool->Count < 10)
			{
			  for (int i = 0; i < ParamsPool->Count; i++)
				 {
				   WorkWithDB(ParamsPool->Strings[i]);
				   Sleep(100);
				 }

			  for (int i = 0; i < ParamsPool->Count; i++)
				 {
				   ParamsPool->Delete(i);
				 }

			  TimerCnt = 0;
			}
		  else
			{
			  for (int i = 0; i < 10; i++)
				 {
				   WorkWithDB(ParamsPool->Strings[i]);
				   Sleep(100);
				 }

			  for (int i = 0; i < 10; i++)
				 {
				   ParamsPool->Delete(i);
				 }

			  TimerCnt = 0;
			}
        }
	}
}
//---------------------------------------------------------------------------

int TMainForm::WorkWithDB(String args)
{
  TStringList *list = new TStringList();
  StrToList(list, args, "&");

  try
	 {
	   if (list->Count == 6)
		 {
		   String query = "INSERT INTO LOGS (ID, REGION, DATE_ADDED, TIME_ADDED, INDEXVZ, EVENT_TYPE, EVENT_TEXT) ";
		   query += "VALUES (gen_id(gen_logs_id,  1), :regionid, :dateadd, :timeadd, :indexvz, :evtype, :evtxt)";

		   try
			  {
				WriteQuery->SQL->Clear();
				WriteQuery->SQL->Add(query);

				WriteQuery->ParamByName("regionid")->AsString = list->Strings[0];
				WriteQuery->ParamByName("dateadd")->AsDateTime = list->Strings[3];
				WriteQuery->ParamByName("timeadd")->AsDateTime = list->Strings[4];
				WriteQuery->ParamByName("indexvz")->AsString = list->Strings[1];
				WriteQuery->ParamByName("evtype")->AsString = list->Strings[2];
				WriteQuery->ParamByName("evtxt")->AsString = list->Strings[5];
			  }
		   catch (Exception &e)
			  {
				ShowLog(e.ToString(), MainForm->LastRecLog);
				SaveLog(LogName, "Помилка вставки запису з параметрами: " +
								 list->Strings[0] + ", " +
								 list->Strings[3] + ", " +
								 list->Strings[4] + ", " +
								 list->Strings[1] + ", " +
								 list->Strings[2] + ", " +
								 list->Strings[5]);
				SaveLog(LogName, e.ToString());
			  }

		   try
			  {
				WriteTransaction->StartTransaction();
				WriteQuery->Prepare();
				WriteQuery->ExecSQL(query);
				WriteTransaction->Commit();
			  }
		   catch (Exception &e)
			  {
				ShowLog(e.ToString(), MainForm->LastRecLog);
				SaveLog(LogName, "Помилка вставки запису з параметрами: " +
								 list->Strings[0] + ", " +
								 list->Strings[3] + ", " +
								 list->Strings[4] + ", " +
								 list->Strings[1] + ", " +
								 list->Strings[2] + ", " +
								 list->Strings[5]);
				SaveLog(LogName, e.ToString());
				WriteTransaction->Rollback();
		 	  }
		 }
	   else
		 {
		   LastRecLog->Lines->Add("Невірні дані!");
         }
	 }
  __finally
	 {
	   delete list;
	 }

  return 0;
}
//---------------------------------------------------------------------------

TFDTransaction *CreateNewTransactionObj()
{
  TFDTransaction *t = new TFDTransaction(NULL);

  t->Connection = MainForm->StatConnection;
  t->Options->AutoCommit = false;
  t->Options->AutoStart = false;
  t->Options->AutoStop = false;
  t->Options->DisconnectAction = xdRollback;
  t->Options->Isolation = xiSnapshot;

  return t;
}
//---------------------------------------------------------------------------

TFDQuery *CreateNewQueryObj(TFDTransaction *t)
{
  TFDQuery * q = new TFDQuery(NULL);

  q->Connection = MainForm->StatConnection;
  q->Transaction = t;

  return q;
}
//---------------------------------------------------------------------------

void DeleteTransactionObj(TFDTransaction *t)
{
  if (t)
	delete t;
}
//---------------------------------------------------------------------------

void DeleteQueryObj(TFDQuery *q)
{
  if (q)
	delete q;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::StatConnectionAfterDisconnect(TObject *Sender)
{
  if (!SwDBOn->Visible)
  	ConnectToDB();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteTransactionAfterCommit(TObject *Sender)
{
  SucsTransCnt->Caption = SucsTransCnt->Caption.ToInt() + 1;
  ActTransCnt->Caption = ActTransCnt->Caption.ToInt() - 1;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteTransactionAfterRollback(TObject *Sender)
{
  RollTransCnt->Caption = RollTransCnt->Caption.ToInt() + 1;
  ActTransCnt->Caption = ActTransCnt->Caption.ToInt() - 1;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteTransactionAfterStartTransaction(TObject *Sender)
{
  ActTransCnt->Caption = ActTransCnt->Caption.ToInt() + 1;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ProcessingTimerTimer(TObject *Sender)
{
  if (ParamsPool->Count > 0)
	{
	  WorkWithDB(ParamsPool->Strings[0]);
	  ParamsPool->Delete(0);
      PoolCount->Caption = ParamsPool->Count;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::HttpServerCommandGet(TIdContext *AContext, TIdHTTPRequestInfo *ARequestInfo,
          TIdHTTPResponseInfo *AResponseInfo)
{
  AResponseInfo->ContentType = "text/html; charset=utf-8";

  if (ARequestInfo->Document == "/")
	{
	  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

	  try
		 {
		   ms->LoadFromFile("index.html");
		   ms->Position = 0;

		   AResponseInfo->ContentText = ms->ReadString(ms->Size);

		   AResponseInfo->ContentText = ParseString(AResponseInfo->ContentText,
													"$ServerName",
													FormCaption);

           AResponseInfo->ContentText = ParseString(AResponseInfo->ContentText,
													"$ServerVersion",
													ModuleVersion->Caption);
		 }
	  __finally {delete ms;}
    }
  else if (ARequestInfo->Document == "/select.html")
	{
	  try
		 {
		   String p_region = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("region")];
		   String p_indexvz = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("indexvz")];
		   String p_evtype = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("evtype")];
		   p_evtype = TIdURI::URLDecode(p_evtype, IndyTextEncoding_UTF8());
		   String p_evtxt = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("evtxt")];
		   p_evtxt = TIdURI::URLDecode(p_evtxt, IndyTextEncoding_UTF8());
		   String p_datefrom = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("datefrom")];
		   String p_dateto = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("dateto")];
		   String p_sortby = ARequestInfo->Params->Strings[ARequestInfo->Params->IndexOfName("sortby")];
		   p_sortby = TIdURI::URLDecode(p_sortby, IndyTextEncoding_UTF8());

		   p_region.Delete(1, p_region.Pos("="));

		   p_indexvz.Delete(1, p_indexvz.Pos("="));
		   p_indexvz = "%" + p_indexvz + "%";

		   p_evtype.Delete(1, p_evtype.Pos("="));
		   p_evtype = "%" + p_evtype + "%";

		   p_evtxt.Delete(1, p_evtxt.Pos("="));
		   p_evtxt = "%" + p_evtxt + "%";

		   p_datefrom.Delete(1, p_datefrom.Pos("="));
		   p_dateto.Delete(1, p_dateto.Pos("="));
		   p_sortby.Delete(1, p_sortby.Pos("="));

		   TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

		   try
			  {
				RequestToDB(ms, p_region, p_indexvz, p_evtype, p_evtxt, p_datefrom, p_dateto, p_sortby);
				ms->Position = 0;
				AResponseInfo->ContentText = ms->ReadString(ms->Size);
			  }
		   __finally {delete ms;}
		 }
	  catch (Exception &e)
		 {
		   LastRecLog->Lines->Add(e.ToString());
		   SaveLog(LogName, e.ToString());
		 }
	}
  else if (ARequestInfo->Document == "/select_all_today.html")
	{
	  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

	  try
		 {
		   RequestToDB(ms, vt_today);
		   ms->Position = 0;
		   AResponseInfo->ContentText = ms->ReadString(ms->Size);
		 }
	  __finally {delete ms;}
	}
  else if (ARequestInfo->Document == "/select_all_files.html")
	{
	  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

	  try
		 {
		   RequestToDB(ms, vt_files);
		   ms->Position = 0;
		   AResponseInfo->ContentText = ms->ReadString(ms->Size);
		 }
	  __finally {delete ms;}
	}
  else if (ARequestInfo->Document == "/select_all_events.html")
	{
	  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

	  try
		 {
		   RequestToDB(ms, vt_events);
		   ms->Position = 0;
		   AResponseInfo->ContentText = ms->ReadString(ms->Size);
		 }
	  __finally {delete ms;}
	}
  else if (ARequestInfo->Document == "/select_all_guardian.html")
	{
      TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

	  try
		 {
		   RequestToDB(ms, vt_guard);
		   ms->Position = 0;
		   AResponseInfo->ContentText = ms->ReadString(ms->Size);
		 }
	  __finally {delete ms;}
	}
}
//---------------------------------------------------------------------------

TStringStream *TMainForm::RequestToDB(TStringStream *ms,
									  String p_region,
									  String p_indexvz,
									  String p_evtype,
                                      String p_evtxt,
									  String p_datefrom,
									  String p_dateto,
									  String p_sortby)
{
  String sqltext = sqlheader;
  bool openfield = true;
  bool use_region = false, use_index = false, use_evtype = false,
	   use_evtxt = false, use_datefr = false, use_dateto = false;

  TFDTransaction *tmp_tr = CreateNewTransactionObj();
  TFDQuery *tmp_query = CreateNewQueryObj(tmp_tr);

  if ((p_region != "") && (openfield))
	{
	  sqltext += "lg.REGION = :region";
	  use_region = true;
	  openfield = false;
	}

  if ((p_indexvz != "") && (openfield))
	{
	  sqltext += "lg.INDEXVZ LIKE :indexvz";
	  use_index = true;
	  openfield = false;
	}
  else if ((p_indexvz != "") && (!openfield))
	{
	  sqltext += " AND lg.INDEXVZ LIKE :indexvz";
	  use_index = true;
	}

  if ((p_evtype != "") && (openfield))
	{
	  sqltext += "lg.EVENT_TYPE LIKE :evtype";
	  use_evtype = true;
	  openfield = false;
	}
  else if ((p_evtype != "") && (!openfield))
	{
	  sqltext += " AND lg.EVENT_TYPE LIKE :evtype";
	  use_evtype = true;
	}

  if ((p_evtxt != "") && (openfield))
	{
	  sqltext += "lg.EVENT_TEXT LIKE :evtxt";
	  use_evtxt = true;
	  openfield = false;
	}
  else if ((p_evtxt != "") && (!openfield))
	{
	  sqltext += " AND lg.EVENT_TEXT LIKE :evtxt";
	  use_evtxt = true;
	}

  if ((p_datefrom != "") && (openfield))
	{
	  if (p_dateto != "")
		{
		  sqltext += "lg.DATE_ADDED BETWEEN :datefrom and :dateto";
		  use_dateto = true;
		  use_datefr = true;
        }
	  else
		{
		  sqltext += "lg.DATE_ADDED = :datefrom";
		  use_datefr = true;
		}
	}
  else if ((p_datefrom != "") && (!openfield))
	{
	  if (p_dateto != "")
		{
		  sqltext += " AND lg.DATE_ADDED BETWEEN :datefrom and :dateto";
		  use_dateto = true;
		  use_datefr = true;
        }
	  else
		{
		  sqltext += " AND lg.DATE_ADDED = :datefrom";
		  use_datefr = true;
		}
	}

  if (!use_region && !use_index && !use_evtype && !use_datefr && !use_dateto)
    sqltext = "SELECT * FROM LOGS lg";

  if (p_sortby == "region")
	sqltext += " ORDER BY lg.REGION";
  else if (p_sortby == "indexvz")
	sqltext += " ORDER BY lg.INDEXVZ";
  else if (p_sortby == "evtype")
	sqltext += " ORDER BY lg.EVENT_TYPE";
  else if (p_sortby == "dateadd")
	sqltext += " ORDER BY lg.DATE_ADDED";
  else if (p_sortby == "timeadd")
	sqltext += " ORDER BY lg.TIME_ADDED";

  try
	 {
	   try
		  {
			tmp_tr->StartTransaction();
			tmp_query->SQL->Add(sqltext);

			if (use_region)
			  tmp_query->ParamByName("region")->AsString = p_region;

			if (use_index)
			  tmp_query->ParamByName("indexvz")->AsString = p_indexvz;

			if (use_evtype)
			  tmp_query->ParamByName("evtype")->AsString = p_evtype;

			if (use_evtxt)
			  tmp_query->ParamByName("evtxt")->AsString = p_evtxt;

			if (use_datefr)
			  {
				short d, m, y;
				y = p_datefrom.SubString(1, 4).ToInt();
				m = p_datefrom.SubString(6, 2).ToInt();
				d = p_datefrom.SubString(9, 2).ToInt();

				TDate *dt = new TDate(y, m, d);
				tmp_query->ParamByName("datefrom")->AsDateTime = dt->FormatString("dd.mm.yyyy");
				delete dt;
			  }

			if (use_dateto)
			  {
                short d, m, y;
				y = p_dateto.SubString(1, 4).ToInt();
				m = p_dateto.SubString(6, 2).ToInt();
				d = p_dateto.SubString(9, 2).ToInt();

				TDate *dt = new TDate(y, m, d);
				tmp_query->ParamByName("dateto")->AsDateTime = dt->FormatString("dd.mm.yyyy");
				delete dt;
			  }

			tmp_query->Prepare();
			tmp_query->Open();
			tmp_tr->Commit();

			try
			   {
                 ms->Clear();
				 ms->Position = 0;
				 ms->WriteString("<html>\n");
				 ms->WriteString("<head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\">");
				 ms->WriteString("<title>Результат виборки</title></head>\n");
				 ms->WriteString("<br>\n<body>\n");
				 ms->WriteString("<form method=\"post\" action=\"/\">\
								 <input type=\"submit\" value=\"На головну\">\
								 </form>");
				 ms->WriteString("<br>\n<table border=\"1\">\n");
				 ms->WriteString("<caption><h2>Результат виборки</h2></caption>\n");
				 ms->WriteString("<tr><th>ID</th><th>Регіон</th><th>Дата події</th>");
				 ms->WriteString("<th>Час події</th><th>ID роб. місця</th><th>Тип події</th><th>Текст</th></tr>");

				 tmp_query->First();

				 while(!tmp_query->Eof)
					{
					  ms->WriteString("<tr>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("ID")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("REGION")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("DATE_ADDED")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("TIME_ADDED")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("INDEXVZ")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("EVENT_TYPE")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("EVENT_TEXT")->AsString + "</td>");
					  ms->WriteString("</tr>\n");

					  tmp_query->Next();
					}

				 ms->WriteString("<br>\n</table>\n");
				 ms->WriteString("<br>\n</body>\n</html>");
			   }
			catch (Exception &e)
			   {
				 SaveLog(LogName, e.ToString());
				 ms->Clear();
				 ms->WriteString("<html>\n");
				 ms->WriteString("<head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\"></head>\n");
				 ms->WriteString("<br><body>\n");
				 ms->WriteString("INVALID REQUEST\n");
				 ms->WriteString("<br></body>\n</html>");
               }
		  }
		catch (Exception &e)
		  {
			tmp_tr->Rollback();
			SaveLog(LogName, e.ToString());
          }
	 }
  __finally {DeleteTransactionObj(tmp_tr); DeleteQueryObj(tmp_query);}

  return ms;
}
//---------------------------------------------------------------------------

TStringStream *TMainForm::RequestToDB(TStringStream *ms, unsigned int view_type)
{
  String sqltext;
  TFDTransaction *tmp_tr = CreateNewTransactionObj();
  TFDQuery *tmp_query = CreateNewQueryObj(tmp_tr);

  switch (view_type)
	{
	  case 0: sqltext = "SELECT * FROM SEL_REC_EVENT_VIEW"; break;
	  case 1: sqltext = "SELECT * FROM SEL_REC_FILES_VIEW"; break;
	  case 2: sqltext = "SELECT * FROM SEL_REC_TODAY_VIEW"; break;
	  case 3: sqltext = "SELECT * FROM SEL_REC_GUARD_VIEW"; break;
	}

  try
	 {
	   try
		  {
			tmp_tr->StartTransaction();
			tmp_query->SQL->Add(sqltext);
			tmp_query->Open();
			tmp_tr->Commit();

			try
			   {
                 ms->Clear();
				 ms->Position = 0;
				 ms->WriteString("<html>\n");
				 ms->WriteString("<head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\">");
				 ms->WriteString("<title>Результат виборки</title></head>\n");
				 ms->WriteString("<br>\n<body>\n");
				 ms->WriteString("<form method=\"post\" action=\"/\">\
								 <input type=\"submit\" value=\"На головну\">\
								 </form>");
				 ms->WriteString("<br>\n<table border=\"1\">\n");
				 ms->WriteString("<caption><h2>Результат виборки</h2></caption>\n");
				 ms->WriteString("<tr><th>ID</th><th>Регіон</th><th>Дата події</th>");
				 ms->WriteString("<th>Час події</th><th>ID роб. місця</th><th>Тип події</th><th>Текст</th></tr>");

				 tmp_query->First();

				 while(!tmp_query->Eof)
					{
					  ms->WriteString("<tr>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("ID")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("REGION")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("DATE_ADDED")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("TIME_ADDED")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("INDEXVZ")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("EVENT_TYPE")->AsString + "</td>");
					  ms->WriteString("<td>" + tmp_query->FieldByName("EVENT_TEXT")->AsString + "</td>");
					  ms->WriteString("</tr>\n");

					  tmp_query->Next();
					}

				 ms->WriteString("<br>\n</table>\n");
				 ms->WriteString("<br>\n</body>\n</html>");
			   }
			catch (Exception &e)
			   {
				 SaveLog(LogName, e.ToString());
				 ms->Clear();
				 ms->WriteString("<html>\n");
				 ms->WriteString("<head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\"></head>\n");
				 ms->WriteString("<br><body>\n");
				 ms->WriteString("INVALID REQUEST\n");
				 ms->WriteString("<br></body>\n</html>");
               }
		  }
		catch (Exception &e)
		  {
			tmp_tr->Rollback();
			SaveLog(LogName, e.ToString());
          }
	 }
  __finally {DeleteTransactionObj(tmp_tr); DeleteQueryObj(tmp_query);}

  return ms;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::IconPP5Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP4Click(TObject *Sender)
{
  if (!Visible)
	{
	  WindowState = wsNormal;
	  Show();
	}
  else
	{
      WindowState = wsMinimized;
	  Hide();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP3Click(TObject *Sender)
{
  if (PopupMenu->Items->Items[2]->ImageIndex == 2)
	StopHttpServer();
  else
	StartHttpServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP2Click(TObject *Sender)
{
  if (PopupMenu->Items->Items[1]->ImageIndex == 2)
	ConnectToDB();
  else
	DisconnectFromDB();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP1Click(TObject *Sender)
{
  if (PopupMenu->Items->Items[0]->ImageIndex == 2)
	StopServer();
  else
	StartServer();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopupMenuPopup(TObject *Sender)
{
  if (!SwServerOn->Visible)
	PopupMenu->Items->Items[0]->ImageIndex = 2;
  else
	PopupMenu->Items->Items[0]->ImageIndex = 3;

  if (!SwDBOn->Visible)
	PopupMenu->Items->Items[1]->ImageIndex = 2;
  else
	PopupMenu->Items->Items[1]->ImageIndex = 3;

  if (!SwHttpOn->Visible)
	PopupMenu->Items->Items[2]->ImageIndex = 2;
  else
	PopupMenu->Items->Items[2]->ImageIndex = 3;

  if (!Visible)
	PopupMenu->Items->Items[4]->Caption = "Показати головне вікно";
  else
	PopupMenu->Items->Items[4]->Caption = "Сховати головне вікно";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  int HideWindow;

  ModuleVersion->Caption = GetVersionInString(Application->ExeName.c_str());

  StatFBDriverLink->VendorLib = "fbclient.dll";

  try
	 {
	   int AutoStart;

	   if (GetConfigLine(AppPath + "\\server.cfg", "AutoStart") == "^no_line")
		 AutoStart = 0;
	   else
		 AutoStart = GetConfigLine(AppPath + "\\server.cfg", "AutoStart").ToInt();

	   if (AutoStart == 1)
		 {
		   if (!AddAppAutoStart("ArmStatServer", Application->ExeName, FOR_CURRENT_USER))
			 ShowMessage("Автоматичний запуск Серверу статистики не створено!");
		   else
			 ShowMessage("Автоматичний запуск Серверу статистики створено!");

		   SetConfigLine(AppPath + "\\server.cfg", "AutoStart", "0");
		 }
	   else if (AutoStart == 2)
		 {
		   if (!RemoveAppAutoStart("ArmStatServer", FOR_CURRENT_USER))
			 ShowMessage("Автоматичний запуск Серверу статистики не видалено!");
		   else
			 ShowMessage("Автоматичний запуск Серверу статистики видалено!");

           SetConfigLine(AppPath + "\\server.cfg", "AutoStart", "0");
		 }

	   HideWindow = GetConfigLine(AppPath + "\\server.cfg", "HideWindow").ToInt();
	   int FirewallRule = GetConfigLine(AppPath + "\\server.cfg", "FirewallRule").ToInt();
	   ListenPort = GetConfigLine(AppPath + "\\server.cfg", "ListenPort").ToInt();
	   RequestPort = GetConfigLine(AppPath + "\\server.cfg", "RequestPort").ToInt();
	   PoolMonitoring = GetConfigLine(AppPath + "\\server.cfg", "PoolMonitoring").ToInt();
	   DBHost = GetConfigLine(AppPath + "\\server.cfg", "DBHost");
	   DBPath = GetConfigLine(AppPath + "\\server.cfg", "DBPath");
	   FormCaption = GetConfigLine(AppPath + "\\server.cfg", "Caption");
	   this->Caption = this->Caption + ": " + FormCaption;
       LogName = DateToStr(Date()) + ".log";

	   if (FirewallRule == 0)
		 {
		   if (system("netsh advfirewall firewall show rule name=\"AMCOL\"") != 0)
			 {
			   AnsiString cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=in action=allow protocol=TCP localport=" + IntToStr(ListenPort) + " enable=yes";
			   int add_in = system(cmd.c_str());
			   Sleep(200);
			   cmd = "netsh advfirewall firewall add rule name=\"AMCOL\" dir=out action=allow protocol=TCP localport=" + IntToStr(ListenPort) + " enable=yes";
			   int add_out = system(cmd.c_str());

			   if ((add_in == 0) && (add_out == 0))
				 SetConfigLine(AppPath + "\\server.cfg", "FirewallRule", "1");
			   else
				 SetConfigLine(AppPath + "\\server.cfg", "FirewallRule", "0");
			}
		  else
			SetConfigLine(AppPath + "\\server.cfg", "FirewallRule", "1");
		 }

	   StartServer();
	   ConnectToDB();
	   StartHttpServer();

	   if (HideWindow == 1)
		 WindowState = wsMinimized;
	 }
  catch (Exception &e)
	 {
	   Caption = e.ToString();

	   if (HideWindow == 1)
		 WindowState = wsMinimized;
	 }
}
//---------------------------------------------------------------------------



