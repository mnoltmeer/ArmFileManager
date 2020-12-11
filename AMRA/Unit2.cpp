/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "RecpOrganizer.h"
#include "RecpThread.h"
#include "Unit2.h"
#include "..\..\work-functions\MyFunc.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAURAForm *AURAForm;

TForm *NewForm;
TEdit *NewName, *NewHost, *NewPort;
TComboBox *NewGroupList;
TButton *NewApply, *NewCancel;
TLabel *LbNewName, *LbNewHost, *LbNewPort;

TForm *EditForm;
TEdit *EditName, *EditHost, *EditPort;
TComboBox *EditGroupList;
TButton *EditApply, *EditCancel;
TLabel *LbEditName, *LbEditHost, *LbEditPort;

TForm *PacketForm;
TLabel *LbPackName, *LbPackList;
TScrollBox *PackRecpList;
std::vector<TCheckBox*> recipients;
TComboBox *PackCmdList;
TButton *PackSend, *PackCancel;
TCheckBox *PackCheckAll;
TComboBox *PackGroupList;

TRecpientItemCollection *AddrBook;
TRecpientCollectionThread *AddrBookChecker;
String AppPath;
int col, row;
//---------------------------------------------------------------------------
__fastcall TAURAForm::TAURAForm(TComponent* Owner)
	: TForm(Owner)
{
  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::ConnectClick(TObject *Sender)
{
  Log->Clear();
  CfgList->Strings->Clear();
  ReadServerList();
  GetStatus->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AddActionLog(String status)
{
  ShowLog(status, ActionLog);
  SendMessage(ActionLog->Handle, WM_VSCROLL, SB_BOTTOM, 0);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PortClick(TObject *Sender)
{
  Port->Text = "";
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::ReadCfgClick(TObject *Sender)
{
  AddActionLog("����� ��������������� �����");

  int id = GetConnectionID(CfgKind->Items->Strings[CfgKind->ItemIndex]);
  String msg = "#send%cfg%" + IntToStr(id);

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;

	   if (AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms) == 0)
		 {
		   String recvmsg = ms->ReadString(ms->Size);
		   ReadTmpCfg(recvmsg);
		   ReadCfg->Hint = id;
		 }
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::ReadTmpCfg(String cfg)
{
  int res = 0;
  TStringList *lst = new TStringList();
  CfgList->Strings->Clear();

  try
	{
	  StrToList(lst, cfg, "#");
	  CfgList->Strings->AddStrings(lst);
	  res = 1;
	}
  __finally {delete lst;}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::DelCfgClick(TObject *Sender)
{
  if (MessageDlg("ĳ���� �������� ������ " +
				 CfgKind->Items->Strings[CfgKind->ItemIndex] + " ?",
				 mtConfirmation,
				 TMsgDlgButtons() << mbYes << mbNo,
				 0) == mrYes)
	{
      AddActionLog("��������� �������");

	  int id = GetConnectionID(CfgKind->Items->Strings[CfgKind->ItemIndex]);
	  String msg = "#delcfg_id%" + IntToStr(id);

	  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

	  try
		 {
		   ms->Position = 0;
		   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
		 }
	  __finally {delete ms;}

      Connect->Click();
	}
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::ReadServerList()
{
  String msg = "#send%srvlist";
  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);
  ms->Position = 0;

  AddActionLog("ϳ��������� �� " + Host->Text + ":" + Port->Text);

  int res = AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);

  if (res == 0)
	{
	  ms->Position = 0;
	  String recvmsg = ms->ReadString(ms->Size);

	  TStringList *servers = new TStringList();

	  try
		 {
		   StrToList(servers, recvmsg, "#");

		   CfgKind->Clear();
		   CfgKind->Items->Add("0: ��������");
		   CfgKind->ItemIndex = 0;

		   for (int i = 0; i < servers->Count; i++)
			  CfgKind->Items->Add(servers->Strings[i]);

		   ServList->Clear();
		   ServList->Items->Add("0: ��");
		   ServList->ItemIndex = 0;

		   for (int i = 0; i < servers->Count; i++)
			  ServList->Items->Add(servers->Strings[i]);

		   LogFilter->Clear();
		   LogFilter->Items->Add("0: ���� ���");
		   LogFilter->ItemIndex = 0;

		   for (int i = 0; i < servers->Count; i++)
			  LogFilter->Items->Add(servers->Strings[i]);
		 }
	  __finally {delete servers;}

	  AddActionLog("� ��'����, �������� ��� ������������ ");

	  ReadRemoteVersion();
	}

  delete ms;

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::ReadRemoteVersion()
{
  String msg = "#send%version";
  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);
  ms->Position = 0;

  int res = AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);

  if (res == 0)
	{
	  ms->Position = 0;
	  String recvmsg = ms->ReadString(ms->Size);
	  ModuleVersion->Caption = recvmsg;

	  if (recvmsg != "no_data")
		AddActionLog("��������� ����� ���������� ������");
	}

  delete ms;

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  if (EditForm)
	delete EditForm;

  if (PacketForm)
    delete PacketForm;

  AddrBookChecker->Terminate();

  while (!AddrBookChecker->Finished)
    Sleep(100);

  delete AddrBookChecker;
  delete AddrBook;
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::SendCfgClick(TObject *Sender)
{
  AddActionLog("³������� ��������������� �����");

  String cfg;

  for (int i = 1; i < CfgList->RowCount; i++)
	 {
	   cfg += CfgList->Keys[i] + "=" + CfgList->Values[CfgList->Keys[i]] + "#";
	 }

  cfg.Delete(cfg.Length(), 1);
  String msg = "#get%cfg%" + ReadCfg->Hint + "%" + cfg;

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;
	   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::HostKeyPress(TObject *Sender, System::WideChar &Key)
{
  if (Key == 13)
	Connect->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PortKeyPress(TObject *Sender, System::WideChar &Key)
{
  if (Key == 13)
	Connect->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::GetLogClick(TObject *Sender)
{
  int id = GetConnectionID(LogFilter->Items->Strings[LogFilter->ItemIndex]);
  RequestLog(id);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::GetStatusClick(TObject *Sender)
{
  AddActionLog("����� ������� ���������");

  String msg = "#send%status";

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;

	   if (AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms) == 0)
		 {
		   String recvmsg = ms->ReadString(ms->Size);
		   ReadTmpCfg(recvmsg);
		 }
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CmdRunClick(TObject *Sender)
{
  AddActionLog("���������� ������� ������� �'�������");

  int id = GetConnectionID(ServList->Items->Strings[ServList->ItemIndex]);
  String msg = "#run%" + IntToStr(id);

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;
	   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
	 }
  __finally {delete ms;}

  GetStatus->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CmdStopClick(TObject *Sender)
{
  AddActionLog("���������� ������� ������� �'�������");

  int id = GetConnectionID(ServList->Items->Strings[ServList->ItemIndex]);
  String msg = "#stop%" + IntToStr(id);

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;
	   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
	 }
  __finally {delete ms;}

  GetStatus->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::GetFileClick(TObject *Sender)
{
  AddActionLog("����� ����� � ���������� ����'�����");

  String filename = RemCompFilePath->Text;
  filename.Delete(1, filename.LastDelimiter("\\"));

  String msg = "#send%file%" + RemCompFilePath->Text;

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;

	   if (AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms) == 0)
		 {
		   if (ms->Size > 0)
			 {
			   if (!DirectoryExists("Download"))
				 CreateDir("Download");

			   try
				  {
					ms->SaveToFile("Download\\" + filename);
					GetFileStatus->Caption = "���� ��������� ��: Download\\" + filename;
				  }
			   catch (Exception &e)
				  {
		   			GetFileStatus->Caption = e.ToString();
				  }
			 }
		   else
			 {
		   	   GetFileStatus->Caption = "³������ ����, ��� ������� ��� ��������";
			 }
		 }
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::GetThreadListClick(TObject *Sender)
{
  AddActionLog("����� ������� ������");

  String msg = "#send%thlist";

  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  try
	 {
	   ms->Position = 0;

	   if (AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms) == 0)
		 {
		   String recvmsg = ms->ReadString(ms->Size);
		   ReadTmpCfg(recvmsg);
		 }
	 }
  __finally {delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AddrListClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->FindItem(AddrList->Selected);

	   if (itm)
		 {
		   Host->Text = itm->Host;
		   Port->Text = itm->Port;
		 }
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
     }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AddrListDblClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->FindItem(AddrList->Selected);

	   if (itm)
		 {
		   AddrListClick(Sender);
  		   Connect->Click();
		 }
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::FormShow(TObject *Sender)
{
  AURAForm->Caption = AURAForm->Caption + ", �����: " + GetVersionInString(Application->ExeName.c_str());
  AddActionLog("������� ������");

  try
	 {
	   if (!FileExists(AppPath + "\\address.grp"))
		 SaveToFile(AppPath + "\\address.grp", "");

	   AddrBook = new TRecpientItemCollection(AppPath + "\\address.grp");
	   AddrBook->CreateSortedTree(AddrList);

	   AddrBookChecker = new TRecpientCollectionThread(true);
	   AddrBookChecker->Collection = AddrBook;
	   AddrBookChecker->CheckInterval = 1000;
       AddrBookChecker->Resume();
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::AskToServer(const wchar_t *host, int port, TStringStream *rw_bufer)
{
  TIdTCPClient *AURAClient;
  int res = 0;

  try
	 {
	   AURAClient = CreateSender(host, port);

	   try
		  {
			AURAClient->Connect();
			AddActionLog("³������� ������ �����");
			AURAClient->IOHandler->Write(rw_bufer, rw_bufer->Size, true);

            rw_bufer->Clear();
			rw_bufer->Position = 0;

            AddActionLog("��������� ������ �����");
			AURAClient->IOHandler->ReadStream(rw_bufer);
		  }
	   catch (Exception &e)
		  {
			AddActionLog(String(host) + ":" + IntToStr(port) + " : " + e.ToString());
			res = -1;
		  }

	   rw_bufer->Position = 0;
	 }
  __finally
	 {
	   if (AURAClient)
		 FreeSender(AURAClient);
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::SendToServer(const wchar_t *host, int port, TStringStream *rw_bufer)
{
  TIdTCPClient *AURAClient;
  int res = 0;

  try
	 {
	   AURAClient = CreateSender(host, port);

	   try
		  {
			AURAClient->Connect();
			AddActionLog("³������� ������ �����");
			AURAClient->IOHandler->Write(rw_bufer, rw_bufer->Size, true);
		  }
	   catch (Exception &e)
		  {
			AddActionLog(String(host) + ":" + IntToStr(port) + " : " + e.ToString());
			res = -1;
		  }

	   rw_bufer->Clear();
	 }
  __finally
	 {
	   if (AURAClient)
		 FreeSender(AURAClient);
	 }

  return res;
}
//---------------------------------------------------------------------------

TIdTCPClient* __fastcall TAURAForm::CreateSender(const wchar_t *host, int port)
{
  TIdTCPClient *sender = new TIdTCPClient(AURAForm);

  sender->Host = host;
  sender->Port = port;
  sender->IPVersion = Id_IPv4;
  sender->OnConnected = AURAClientConnected;
  sender->OnDisconnected = AURAClientDisconnected;
  sender->ConnectTimeout = 500;
  sender->ReadTimeout = 5000;

  return sender;
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::FreeSender(TIdTCPClient *sender)
{
  if (sender)
	{
	  if (sender->Connected())
		{
		  sender->Disconnect();
		  sender->Socket->Close();
		}

      delete sender;
    }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AURAClientConnected(TObject *Sender)
{
  TIdTCPClient *sender = reinterpret_cast<TIdTCPClient*>(Sender);

  AURAForm->AddActionLog("������� ��� � " + sender->Host + ":" + IntToStr(sender->Port));
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AURAClientDisconnected(TObject *Sender)
{
  TIdTCPClient *sender = reinterpret_cast<TIdTCPClient*>(Sender);

  AURAForm->AddActionLog("ʳ���� ��� � " + sender->Host + ":" + IntToStr(sender->Port));
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::NewCfgClick(TObject *Sender)
{
  if (OpenCfgDialog->Execute())
	{
	  AddActionLog("������������ ������� � �����: " + OpenCfgDialog->FileName);

	  TStringList *cfg = new TStringList();
	  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
	  String file_name = OpenCfgDialog->FileName;
	  int pos = file_name.LastDelimiter("\\");
	  file_name.Delete(1, pos);

	  try
		 {
		   cfg->LoadFromFile(OpenCfgDialog->FileName);
		   String msg = ListToStr(cfg, "#");
		   msg = "#get_new%" + file_name + "%" + msg;
		   ms->Position = 0;
		   ms->WriteString(msg);
           ms->Position = 0;
		   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
		 }
	  __finally {delete cfg; delete ms;}

	  Connect->Click();
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::ShutdownClick(TObject *Sender)
{
  if (MessageDlg("�������� ��������� ������?",
				 mtConfirmation,
				 TMsgDlgButtons() << mbYes << mbNo,
				 0) == mrYes)
	{
	  AddActionLog("���������� ������� #shutdown");

	  TStringStream *ms = new TStringStream("#shutdown%", TEncoding::UTF8, true);

	  try
		 {
		   ms->Position = 0;
		   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
		 }
	 __finally {delete ms;}
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::RestartGuardClick(TObject *Sender)
{
  if (MessageDlg("������������� Guardian ������?",
				 mtConfirmation,
				 TMsgDlgButtons() << mbYes << mbNo,
				 0) == mrYes)
	{
	  AddActionLog("���������� ������� ����������� Guardian");

	  TStringStream *ms = new TStringStream("#restart_guard%", TEncoding::UTF8, true);

	  try
		 {
		   ms->Position = 0;
		   SendToServer(Host->Text.c_str(), Port->Text.ToInt(), ms);
		 }
	 __finally {delete ms;}
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AddToBookClick(TObject *Sender)
{
  try
	 {
	   if (AddrBook->FindGroup(AddrList->Selected))
		 CreateNewForm();
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::AddGroupBookClick(TObject *Sender)
{
  try
	 {
	   String name = InputBox("��������� ���� �����", "������ ��'�", "");

	   if (name != "")
		 {
		   AddrBook->Add(0, AddrList->Items->Add(AddrList->Selected, name), name);
		   AddrBook->CreateSortedTree(AddrList);
		   AddrBookChecker->CollectionChanged = true;
		 }
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::DeleteFromBookClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm && !itm->ParentNodeID)
		 {
		   if (MessageDlg("ĳ���� �������� ����� ������?",
						  mtConfirmation,
						  TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
			 {
			   AddrBook->DeleteRecipientsInGroup(AddrBook->FindGroup(AddrList->Selected)->ID);
			   AddrBook->Remove(AddrList->Selected);
			   AddrBook->CreateSortedTree(AddrList);
			   AddrBookChecker->CollectionChanged = true;
			 }
		 }
	   else if (MessageDlg("�������� �����?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
		 {
		   int grp_id = itm->ParentNodeID;

		   AddrBook->Remove(itm->ID);
           AddrBook->CreateSortedTree(AddrList);
		   AddrBookChecker->CollectionChanged = true;

		   AddrBook->FindGroup(grp_id)->Node->Expand(true);
		 }
	 }
  catch (Exception &e)
	 {
	   AddActionLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::EditBookClick(TObject *Sender)
{
  RecipientItem *itm = AddrBook->Find(AddrList->Selected);

  if (itm->ParentNodeID == 0)
	{
	  String name = InputBox("����������� �����", "������ ���� ��'�", itm->Name);

	  if (name != "")
		{
          itm->Name = name;
		  AddrBook->CreateSortedTree(AddrList);
		  AddrBookChecker->CollectionChanged = true;
		}
	}
  else
	{
	  CreateEditForm();
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CreateEditForm()
{
  if (!EditForm)
	{
	  EditForm = new TForm(AURAForm);
	  EditForm->Position = poOwnerFormCenter;
	  EditForm->BorderStyle = bsSingle;
	  EditForm->BorderIcons = EditForm->BorderIcons >> biMinimize >> biMaximize;
      EditForm->Caption = "����������� ������";
	  EditForm->AutoSize = false;
	  EditForm->OnShow = EditFormShow;
	  EditForm->Width = 200;
	  EditForm->Height = 260;

	  EditGroupList = new TComboBox(EditForm);
	  EditGroupList->Parent = EditForm;
	  EditGroupList->Width = 180;
	  EditGroupList->Height = 40;
	  EditGroupList->Left = 5;
	  EditGroupList->Top = 5;

	  LbEditName = new TLabel(EditForm);
	  LbEditName->Parent = EditForm;
	  LbEditName->Caption = "��'�";
	  LbEditName->Left = 5;
	  LbEditName->Top = EditGroupList->Top + EditGroupList->Height + 2;;

	  EditName = new TEdit(EditForm);
	  EditName->Parent = EditForm;
	  EditName->Width = 150;
	  EditName->Height = 30;
	  EditName->Left = 2;
	  EditName->Top = LbEditName->Top + LbEditName->Height + 2;

	  LbEditHost = new TLabel(EditForm);
	  LbEditHost->Parent = EditForm;
	  LbEditHost->Caption = "����";
	  LbEditHost->Left = 5;
	  LbEditHost->Top = EditName->Top + EditName->Height + 1;

	  EditHost = new TEdit(EditForm);
	  EditHost->Parent = EditForm;
	  EditHost->Width = 150;
	  EditHost->Height = 30;
	  EditHost->Left = 2;
	  EditHost->Top = LbEditHost->Top + LbEditHost->Height + 2;

	  LbEditPort = new TLabel(EditForm);
	  LbEditPort->Parent = EditForm;
	  LbEditPort->Caption = "����";
	  LbEditPort->Left = 5;
	  LbEditPort->Top = EditHost->Top + EditHost->Height + 1;

	  EditPort = new TEdit(EditForm);
	  EditPort->Parent = EditForm;
	  EditPort->Width = 50;
	  EditPort->Height = 30;
	  EditPort->Left = 2;
	  EditPort->Top = LbEditPort->Top + LbEditPort->Height + 2;

	  EditApply = new TButton(EditForm);
	  EditApply->Parent = EditForm;
	  EditApply->Caption = "�����";
	  EditApply->Width = 50;
	  EditApply->Height = 30;
	  EditApply->Left = 2;
	  EditApply->Top = EditPort->Top + EditPort->Height + 2;
	  EditApply->OnClick = EditApplyClick;

	  EditCancel = new TButton(EditForm);
	  EditCancel->Parent = EditForm;
	  EditCancel->Caption = "���������";
	  EditCancel->Width = 80;
	  EditCancel->Height = 30;
	  EditCancel->Left = EditApply->Left + EditApply->Width + 5;
	  EditCancel->Top = EditPort->Top + EditPort->Height + 2;
	  EditCancel->OnClick = EditCancelClick;

	  EditForm->Show();
	}
  else
	EditForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::EditFormShow(TObject *Sender)
{
  try
	 {
	   RecipientItem *grp, *itm;

	   itm = AddrBook->FindItem(AddrList->Selected);

	   if (itm)
		 {
		   EditGroupList->Clear();
		   EditName->Clear();
		   EditHost->Clear();
		   EditPort->Clear();

		   grp = AddrBook->FindGroup(itm->ParentNodeID);
		   AddrBook->SelectGroups(EditGroupList->Items);
		   EditGroupList->Items->Add("+ ������ �����");
		   EditGroupList->ItemIndex = EditGroupList->Items->IndexOf(grp->Name);

		   EditName->Text = itm->Name;
		   EditName->Tag = itm->ID; //�����'����� �� �������� ������
		   EditHost->Text = itm->Host;
		   EditPort->Text = itm->Port;
		 }
	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� �����: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::EditApplyClick(TObject *Sender)
{
  try
	 {
	   String name = EditGroupList->Items->Strings[EditGroupList->ItemIndex];

	   if (name == "+ ������ �����")
		 {
		   AddGroupBook->Click();
           EditGroupList->Clear();
		   AddrBook->SelectGroups(EditGroupList->Items);
		   name = EditGroupList->Items->Strings[EditGroupList->Items->Count - 1];
		 }

//�����'����� ������ ���������� �����, �� ��� ������� �����
	   RecipientItem *prev_grp = AddrBook->FindGroup(AddrBook->FindItem(EditName->Tag)->ParentNodeID);

	   AddrBook->Remove(EditName->Tag); //��������� �����
	   AddrBook->CreateSortedTree(AddrList);

//� ��������� ����� � ���� ����������, �� �� �������� �� ������� ������
	   RecipientItem *grp = AddrBook->FindGroup(name);

	   if (grp)
		 {
		   AddrBook->Add(grp->ID, grp->Node, EditName->Text, EditHost->Text, EditPort->Text);
		   AddrBook->CreateSortedTree(AddrList);
		   AddrBookChecker->CollectionChanged = true;
		 }

	   prev_grp->Node->Expand(true);
	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� �����: " + e.ToString());
	 }

  EditForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::EditCancelClick(TObject *Sender)
{
  EditForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CreateNewForm()
{
  if (!NewForm)
	{
	  NewForm = new TForm(AURAForm);
	  NewForm->Position = poOwnerFormCenter;
	  NewForm->BorderStyle = bsSingle;
	  NewForm->BorderIcons = NewForm->BorderIcons >> biMinimize >> biMaximize;
	  NewForm->Caption = "��������� ������";
	  NewForm->AutoSize = false;
	  NewForm->OnShow = NewFormShow;
	  NewForm->Width = 280;
	  NewForm->Height = 260;

	  NewGroupList = new TComboBox(NewForm);
	  NewGroupList->Parent = NewForm;
	  NewGroupList->Width = 240;
	  NewGroupList->Height = 40;
	  NewGroupList->Left = 5;
	  NewGroupList->Top = 5;

	  LbNewName = new TLabel(NewForm);
	  LbNewName->Parent = NewForm;
	  LbNewName->Caption = "��'�";
	  LbNewName->Left = 5;
	  LbNewName->Top = NewGroupList->Top + NewGroupList->Height + 2;

	  NewName = new TEdit(NewForm);
	  NewName->Parent = NewForm;
	  NewName->Width = 150;
	  NewName->Height = 30;
	  NewName->Left = 2;
	  NewName->Top = LbNewName->Top + LbNewName->Height + 2;

	  LbNewHost = new TLabel(NewForm);
	  LbNewHost->Parent = NewForm;
	  LbNewHost->Caption = "����";
	  LbNewHost->Left = 5;
	  LbNewHost->Top = NewName->Top + NewName->Height + 1;

	  NewHost = new TEdit(NewForm);
	  NewHost->Parent = NewForm;
	  NewHost->Width = 150;
	  NewHost->Height = 30;
	  NewHost->Left = 2;
	  NewHost->Top = LbNewHost->Top + LbNewHost->Height + 2;

	  LbNewPort = new TLabel(NewForm);
	  LbNewPort->Parent = NewForm;
	  LbNewPort->Caption = "����";
	  LbNewPort->Left = 5;
	  LbNewPort->Top = NewHost->Top + NewHost->Height + 1;

	  NewPort = new TEdit(NewForm);
	  NewPort->Parent = NewForm;
	  NewPort->Width = 50;
	  NewPort->Height = 30;
	  NewPort->Left = 2;
	  NewPort->Top = LbNewPort->Top + LbNewPort->Height + 2;

	  NewApply = new TButton(NewForm);
	  NewApply->Parent = NewForm;
	  NewApply->Caption = "�����";
	  NewApply->Width = 50;
	  NewApply->Height = 30;
	  NewApply->Left = 2;
	  NewApply->Top = NewPort->Top + NewPort->Height + 2;
	  NewApply->OnClick = NewApplyClick;

	  NewCancel = new TButton(NewForm);
	  NewCancel->Parent = NewForm;
	  NewCancel->Caption = "���������";
	  NewCancel->Width = 80;
	  NewCancel->Height = 30;
	  NewCancel->Left = NewApply->Left + NewApply->Width + 5;
	  NewCancel->Top = NewPort->Top + NewPort->Height + 2;
	  NewCancel->OnClick = NewCancelClick;

	  NewForm->Show();
	}
  else
	NewForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::NewFormShow(TObject *Sender)
{
  try
	 {
	   NewGroupList->Clear();
	   NewName->Clear();
	   NewHost->Text = AURAForm->Host->Text;
       NewPort->Text = AURAForm->Port->Text;

	   AddrBook->SelectGroups(NewGroupList->Items);
	   NewGroupList->Items->Add("+ ������ �����");

	   RecipientItem *grp = AddrBook->FindGroup(AddrList->Selected);

	   NewGroupList->ItemIndex = NewGroupList->Items->IndexOf(grp->Name);
	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� �����: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::NewApplyClick(TObject *Sender)
{
  String name = NewGroupList->Items->Strings[NewGroupList->ItemIndex];

  if (name == "+ ������ �����")
	{
	  AddGroupBook->Click();
	  NewGroupList->Clear();
	  AddrBook->SelectGroups(NewGroupList->Items);
	  name = NewGroupList->Items->Strings[NewGroupList->Items->Count - 1];
	}

  RecipientItem *grp = AddrBook->FindGroup(name);

  if (grp)
	{
	  AddrBook->Add(grp->ID,
					grp->Node,
					NewName->Text,
					NewHost->Text,
					NewPort->Text);

	  AddrBook->CreateSortedTree(AddrList);
	  AddrBook->FindGroup(name)->Node->Expand(true);
      AddrBookChecker->CollectionChanged = true;
	}

  NewForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::NewCancelClick(TObject *Sender)
{
  NewForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CreatePacketForm()
{
  if (!PacketForm)
	{
	  PacketForm = new TForm(AURAForm);
	  PacketForm->Position = poOwnerFormCenter;
	  PacketForm->BorderStyle = bsSingle;
	  PacketForm->BorderIcons = PacketForm->BorderIcons >> biMinimize >> biMaximize;
	  PacketForm->Caption = "������� ���������� ������";
	  PacketForm->AutoSize = false;
	  PacketForm->OnShow = PacketFormShow;
	  PacketForm->Width = 450;
	  PacketForm->Height = 360;

	  LbPackName = new TLabel(PacketForm);
	  LbPackName->Parent = PacketForm;
	  LbPackName->Caption = "������ ��������";
	  LbPackName->Left = 5;
	  LbPackName->Top = 5;

      PackGroupList = new TComboBox(PacketForm);
	  PackGroupList->Parent = PacketForm;
	  PackGroupList->Width = 180;
	  PackGroupList->Height = 40;
	  PackGroupList->Left = 2;
	  PackGroupList->Top = LbPackName->Top + LbPackName->Height + 5;
	  PackGroupList->OnChange = PackGroupListChange;

	  PackRecpList = new TScrollBox(PacketForm);
	  PackRecpList->Parent = PacketForm;
	  PackRecpList->Width = 180;
	  PackRecpList->Height = 240;
	  PackRecpList->Left = 2;
	  PackRecpList->Top = PackGroupList->Top + PackGroupList->Height + 2;
	  PackRecpList->OnMouseWheelDown = RecpListMouseWheelDown;
	  PackRecpList->OnMouseWheelUp = RecpListMouseWheelUp;
	  PackRecpList->OnMouseMove = RecpListMouseMove;

	  PackCheckAll = new TCheckBox(PacketForm);
	  PackCheckAll->Parent = PacketForm;
	  PackCheckAll->Caption = "������� ���";
	  PackCheckAll->Checked = false;
	  PackCheckAll->Width = 150;
	  PackCheckAll->Height = 20;
	  PackCheckAll->Left = 2;
	  PackCheckAll->Top = PackRecpList->Top + PackRecpList->Height + 5;
	  PackCheckAll->OnClick = PackAllClick;

	  LbPackList = new TLabel(PacketForm);
	  LbPackList->Parent = PacketForm;
	  LbPackList->Caption = "������ �������";
	  LbPackList->Left = PackRecpList->Left + PackRecpList->Width + 10;
	  LbPackList->Top = 5;

	  PackCmdList = new TComboBox(PacketForm);
	  PackCmdList->Parent = PacketForm;
	  PackCmdList->Width = 240;
	  PackCmdList->Height = 40;
	  PackCmdList->Left = LbPackList->Left;
	  PackCmdList->Top = LbPackName->Top + LbPackName->Height + 2;
	  PackCmdList->Items->Add("���������� �������");
      PackCmdList->Items->Add("������������� �� �'�������");
	  PackCmdList->Items->Add("�������� ������");
	  PackCmdList->Items->Add("������������� Guardian");
	  PackCmdList->Items->Add("������� �������� � ��������");
	  PackCmdList->Items->Add("������������ �� ��������� ���������");
	  PackCmdList->Items->Add("�������� ������");
	  PackCmdList->Items->Add("��������� ����");
	  PackCmdList->Items->Add("�������� ������ (��'� �����)");
	  PackCmdList->Items->Add("�������� ������ (��'� �'�������)");

	  PackCancel = new TButton(PacketForm);
	  PackCancel->Parent = PacketForm;
	  PackCancel->Caption = "���������";
	  PackCancel->Width = 80;
	  PackCancel->Height = 30;
	  PackCancel->Left = PacketForm->Width - 15 - PackCancel->Width;
	  PackCancel->Top = PacketForm->Height - 70;
	  PackCancel->OnClick = PackCancelClick;

      PackSend = new TButton(PacketForm);
	  PackSend->Parent = PacketForm;
	  PackSend->Caption = "��������";
	  PackSend->Width = 80;
	  PackSend->Height = 30;
	  PackSend->Left = PackCancel->Left - PackSend->Width - 5;
	  PackSend->Top = PackCancel->Top;
	  PackSend->OnClick = PackApplyClick;

	  PacketForm->Show();
	}
  else
	{
	  PacketForm->Show();
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CreateRecpList(std::vector<int> *recp_list)
{
  try
	 {
       delete PackRecpList;

	   PackRecpList = new TScrollBox(PacketForm);
	   PackRecpList->Parent = PacketForm;
	   PackRecpList->Width = 180;
	   PackRecpList->Height = 240;
	   PackRecpList->Left = 2;
	   PackRecpList->Top = PackGroupList->Top + PackGroupList->Height + 2;
	   PackRecpList->OnMouseWheelDown = RecpListMouseWheelDown;
	   PackRecpList->OnMouseWheelUp = RecpListMouseWheelUp;
	   PackRecpList->OnMouseMove = RecpListMouseMove;

       PackCheckAll->Top = PackRecpList->Top + PackRecpList->Height + 5;
       PackCheckAll->Checked = false;

	   int top = 2;
	   recipients.clear();

	   for (int i = 0; i < recp_list->size(); i++)
		  {
			TCheckBox *recp = new TCheckBox(PacketForm);
			recp->Parent = PackRecpList;
			recp->Checked = false;
			recp->Width = 150;
			recp->Height = 20;
			recp->Left = 2;
			recp->Top = top;
			top += recp->Height;

			try
			   {
				 RecipientItem *itm = AddrBook->FindItem(recp_list->at(i));
				 recp->Caption = itm->Name;
				 recp->Hint = itm->Host + ":" + itm->Port;
				 recp->ShowHint = true;

				 recipients.push_back(recp);
			   }
			catch (Exception &e)
			   {
				 AddActionLog("������� ������� ������ � ������� �����: " + e.ToString());
				 continue;
			   }
		  }

	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� �����: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PacketFormShow(TObject *Sender)
{
  try
	 {
       PackGroupList->Clear();
	   PackGroupList->Items->Add("�� ������");
	   AddrBook->SelectGroups(PackGroupList->Items);
	   PackGroupList->ItemIndex = 0;

	   PackCmdList->ItemIndex = 0;
	   PackCheckAll->Checked = false;

	   std::vector<int> recp_list;

	   AddrBook->SelectRecipients(&recp_list);

	   CreateRecpList(&recp_list);
	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� �����: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PackApplyClick(TObject *Sender)
{
  String name;

  if (PackCmdList->ItemIndex == 6)
	{
	  String old_mask = OpenCfgDialog->Filter;
	  OpenCfgDialog->Filter = "������� �������|*.es";

	  if (OpenCfgDialog->Execute())
		{
		  name = OpenCfgDialog->FileName;
		}

	  OpenCfgDialog->Filter = old_mask;
	  OpenCfgDialog->FileName = "";
	}
  else if (PackCmdList->ItemIndex == 7)
	{
      CfgList->Strings->Clear();
	}
  else if (PackCmdList->ItemIndex == 8)
	{
	  name = InputBox("��������� �������", "������ ��'� ����� (��� ����������)", "");
	}
  else if (PackCmdList->ItemIndex == 9)
	{
	  name = InputBox("��������� �������", "������ ��'� �'������� (Caption)", "");
	}

  for (int i = 0; i < recipients.size(); i++)
	 {
	   if (recipients[i]->Checked)
		 {
		   String rcp_host;
		   int rcp_port;

		   ExtractHostPort(recipients[i]->Hint, rcp_host, rcp_port);

		   switch (PackCmdList->ItemIndex)
			 {
			   case 0:
					  {
						TStringStream *ms = new TStringStream("#reload_cfg%", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
							 SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 1:
					  {
						TStringStream *ms = new TStringStream("#restart%0", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
							 SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 2:
					  {
						TStringStream *ms = new TStringStream("#shutdown%", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
							 SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 3:
					  {
						TStringStream *ms = new TStringStream("#restart_guard%", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
                             SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 4:
					  {
						TStringStream *ms = new TStringStream("#cpy_manager%", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
                             SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 5:
					  {
						TStringStream *ms = new TStringStream("#upd_manager%", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;
                             SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 6:
					  {
						if (name != "")
						  {
                            AddActionLog("������������ ������� � �����: " + name);

							TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

							try
							   {
								 String script = LoadTextFile(name);

								 script = "#exec_script%" + script;
								 ms->Position = 0;
								 ms->WriteString(script);
								 ms->Position = 0;
								 SendToServer(rcp_host.c_str(), rcp_port, ms);
							   }
							__finally {delete ms;}
						  }

						break;
					  }

			   case 7:
					  {
						TStringStream *ms = new TStringStream("#send%version", TEncoding::UTF8, true);

						try
						   {
							 ms->Position = 0;

							 int res = AskToServer(rcp_host.c_str(), rcp_port, ms);

							 if (res == 0)
							   {
								 ms->Position = 0;
								 String recvmsg = ms->ReadString(ms->Size);
								 ModuleVersion->Caption = recvmsg;

								 CfgList->Strings->Add(recipients[i]->Hint +
													   "=" +
													   recvmsg);
							   }
						   }
						__finally {delete ms;}

						break;
					  }

			   case 8:
					  {
						if (name == "")
						  {
                            AddActionLog("��������� �������: ������ ��'� �����!");
							break;
						  }

						TStringStream *ms = new TStringStream("#delcfg_file%" + name,
															  TEncoding::UTF8,
															  true);

						try
						   {
							 ms->Position = 0;
							 SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

			   case 9:
					  {
                        if (name == "")
						  {
							AddActionLog("��������� �������: ������ ��'� �'�������!");
							break;
						  }

						TStringStream *ms = new TStringStream("#delcfg_name%" + name,
															  TEncoding::UTF8,
															  true);

						try
						   {
							 ms->Position = 0;
							 SendToServer(rcp_host.c_str(), rcp_port, ms);
						   }
						__finally {delete ms;}

						break;
					  }

               default: AddActionLog("������ ������: ������� ������"); break;
			 }
         }
     }

  PacketForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PackCancelClick(TObject *Sender)
{
  recipients.clear();
  PacketForm->Close();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PacketCmdSendClick(TObject *Sender)
{
  try
	 {
	   CreatePacketForm();
	 }
  catch (Exception &e)
	 {
	   AddActionLog("������� ��������� �����: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::RecpListMouseWheelDown(TObject *Sender, TShiftState Shift,
												  const TPoint &MousePos, bool &Handled)
{
  SendMessage(PackRecpList->Handle, WM_VSCROLL, SB_LINEDOWN, 0);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::RecpListMouseWheelUp(TObject *Sender, TShiftState Shift,
												const TPoint &MousePos, bool &Handled)
{
  SendMessage(PackRecpList->Handle, WM_VSCROLL, SB_LINEUP, 0);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PackAllClick(TObject *Sender)
{
  if (PackCheckAll->Checked)
	{
	  for (int i = 0; i < recipients.size(); i++)
		 {
		   recipients[i]->Checked = true;
		 }
	}
  else
	{
      for (int i = 0; i < recipients.size(); i++)
		 {
		   recipients[i]->Checked = false;
		 }
    }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::RecpListMouseMove(TObject *Sender,
											 TShiftState Shift,
											 int X,
											 int Y)
{
  PackRecpList->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PackGroupListChange(TObject *Sender)
{
  std::vector<int> grp_list;

  if (PackGroupList->ItemIndex == 0) //�������� ���
	{
	  AddrBook->SelectRecipients(&grp_list);

	  CreateRecpList(&grp_list);
	}
  else //�������� � ��������� �����
	{
	  try
		 {

		   AddrBook->SelectRecipientsInGroup(&grp_list,
											 PackGroupList->Items->Strings[PackGroupList->ItemIndex]);

		   CreateRecpList(&grp_list);
		 }
	  catch (Exception &e)
		 {
		   AddActionLog("PackGroupListChange(): " + e.ToString());
		 }
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::ImportInAddrBookClick(TObject *Sender)
{
  String old_mask = OpenCfgDialog->Filter;
  OpenCfgDialog->Filter = "������ ����� (����)|*.book|������ ����� (���)|*.grp";

  if (OpenCfgDialog->Execute())
	{
	  try
		 {
		   String file_ext = OpenCfgDialog->FileName;
		   int pos = file_ext.LastDelimiter("\\");
		   file_ext.Delete(1, pos);
		   pos = file_ext.LastDelimiter(".");
		   file_ext.Delete(1, pos);

		   if (file_ext == "book")
			 {
			   RecipientItem *grp = AddrBook->FindGroup("�����������");

			   if (!grp)
				 {
				   AddrBook->Add(0, NULL, "�����������"); //������ ����� � ���� �����
				   grp = AddrBook->FindGroup("�����������");
				 }

			   TStringList *old_book = new TStringList();
			   TStringList *lst = new TStringList();

			   old_book->LoadFromFile(OpenCfgDialog->FileName, TEncoding::UTF8);

			   try
				  {
					for (int i = 0; i < old_book->Count; i++)
					   {
						 lst->Clear();
						 StrToList(lst, old_book->Strings[i], ";");
                         grp = AddrBook->FindGroup("�����������");
						 AddrBook->Add(grp->ID,
									   grp->Node,
									   lst->Strings[0],
									   lst->Strings[1],
									   lst->Strings[2]);

						 AddActionLog("����������� �����: " + lst->Strings[0]);
					   }

                    AddActionLog("������ ����� ���������");
				  }
			   __finally {delete lst; delete old_book;}

			   AddrBook->Save();

               AddrBook->CreateSortedTree(AddrList);
			   grp = AddrBook->FindGroup("�����������");
			   grp->Node->Expand(true);
			 }
		   else if (file_ext == "grp")
			 {
			   TRecpientItemCollection *ImportBook = new TRecpientItemCollection(OpenCfgDialog->FileName);

			   try
				  {
					AddrBook->ImportData(ImportBook);
					AddrBook->CreateSortedTree(AddrList);
					AddrBook->Save();
					AddActionLog("������ ����� ���������");
				  }
			   __finally{delete ImportBook;}
			 }
		 }
	  catch (Exception &e)
		 {
		   AddActionLog("������ �����: " + e.ToString());
		 }
	}

  OpenCfgDialog->Filter = old_mask;
  OpenCfgDialog->FileName = "";
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::ExportFromAddrBookClick(TObject *Sender)
{
  String old_mask = SaveCfgDialog->Filter;
  SaveCfgDialog->Filter = "������ ����� (����)|*.book|������ ����� (���)|*.grp";

  if (SaveCfgDialog->Execute())
	{
	  try
		 {
		   String file_ext = SaveCfgDialog->FileName;
		   int pos = file_ext.LastDelimiter("\\");
		   file_ext.Delete(1, pos);
		   pos = file_ext.LastDelimiter(".");

		   if (!pos)
			 {
			   if (SaveCfgDialog->FilterIndex == 1)
				 file_ext = "book";
			   else if (SaveCfgDialog->FilterIndex == 2)
				 file_ext = "grp";

			   SaveCfgDialog->FileName = SaveCfgDialog->FileName + "." + file_ext;
			 }
		   else
		     file_ext.Delete(1, pos);

		   if (file_ext == "book")
			 {
			   DeleteFile(SaveCfgDialog->FileName);

			   for (int i = 0; i < AddrBook->Count; i++)
				  {
					if (AddrBook->Items[i]->ParentNodeID > 0)
					  {
						AddToFile(SaveCfgDialog->FileName,
								  AddrBook->Items[i]->Name + ";" +
								  AddrBook->Items[i]->Host + ";" +
							  	  AddrBook->Items[i]->Port + "\r\n");
					  }
				  }
			 }
		   else if (file_ext == "grp")
			 AddrBook->SaveToFile(SaveCfgDialog->FileName);

           AddActionLog("������� ����� ���������");
		 }
	  catch (Exception &e)
		 {
		   AddActionLog("������� �����: " + e.ToString());
		 }
	}

  SaveCfgDialog->Filter = old_mask;
  SaveCfgDialog->FileName = "";
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::SaveCfgListClick(TObject *Sender)
{
  if (SaveCfgDialog->Execute())
	{
	  try
		 {
		   int len = SaveCfgDialog->FileName.Length();

		   if (SaveCfgDialog->FileName.SubString(len - 3, 4) != ".cfg")
			 SaveCfgDialog->FileName = SaveCfgDialog->FileName + ".cfg";

		   for (int i = 1; i < CfgList->RowCount; i++)
			  {
				if (CfgList->Cells[1][i] == "")
				  AddToFile(SaveCfgDialog->FileName, CfgList->Cells[0][i] + "\r\n");
				else
				  AddToFile(SaveCfgDialog->FileName,
							CfgList->Cells[0][i] + "=" + CfgList->Cells[1][i] + "\r\n");
			  }

		   AddActionLog("������� ����� ������� ���������");
		 }
	  catch (Exception &e)
		 {
		   AddActionLog("������� ����� �������: " + e.ToString());
		 }
	}
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::SaveLogClick(TObject *Sender)
{
  String old_mask = SaveCfgDialog->Filter;
  SaveCfgDialog->Filter = "����� ����|*.log";

  if (SaveCfgDialog->Execute())
	{
	  try
		 {
		   int len = SaveCfgDialog->FileName.Length();

		   if (SaveCfgDialog->FileName.SubString(len - 3, 4) != ".log")
			 SaveCfgDialog->FileName = SaveCfgDialog->FileName + ".log";

		   ActionLog->Lines->SaveToFile(SaveCfgDialog->FileName, TEncoding::UTF8);

		   AddActionLog("������� ���� ���������");
		 }
	  catch (Exception &e)
		 {
		   AddActionLog("������� ����: " + e.ToString());
		 }
	}

  SaveCfgDialog->Filter = old_mask;
  SaveCfgDialog->FileName = "";
}
//---------------------------------------------------------------------------

int __fastcall TAURAForm::GetConnectionID(const String &str_with_id)
{
  int res;

  try
	 {
	   String operstr = str_with_id;
	   int pos = operstr.Pos(":");

	   operstr = operstr.SubString(1, pos - 1);

	   if (operstr == "")
		 operstr = "0";

       res = operstr.ToInt();
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   AddActionLog("GetConnectionID: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::RequestLog(int conn_id)
{
  try
	 {
       Log->Clear();
	   String msg = "#send%log";

	   TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

	   try
		  {
			ms->Position = 0;

			if (AskToServer(Host->Text.c_str(), Port->Text.ToInt(), ms) == 0)
			  {
				String recvmsg = ms->ReadString(ms->Size);

				TStringList *unfiltered = new TStringList();

				try
				   {
					 StrToList(unfiltered, recvmsg, "&");

					 if (conn_id > 0)
					   {
						 TStringList *filtered = new TStringList();

						 try
							{
							  String filter = IntToStr(conn_id);

							  for (int i = 0; i < unfiltered->Count; i++)
								 {
								   if (unfiltered->Strings[i].Pos("{id: " + filter + "}"))
									 filtered->Add(unfiltered->Strings[i]);
								 }

                              Log->Lines->AddStrings(filtered);
							}
						 __finally {delete filtered;}
					   }
					 else
					   Log->Lines->AddStrings(unfiltered);

					 SendMessage(Log->Handle, WM_VSCROLL, SB_BOTTOM, 0);
				   }
		  		__finally {delete unfiltered;}
			  }
		  }
	   __finally {delete ms;}
	 }
  catch (Exception &e)
	 {
	   AddActionLog("RequestLog: " + e.ToString());
     }
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::LogFilterChange(TObject *Sender)
{
  int id = GetConnectionID(LogFilter->Items->Strings[LogFilter->ItemIndex]);
  RequestLog(id);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PPConfigShowClick(TObject *Sender)
{
  CfgKind->ItemIndex = CfgKind->Items->IndexOf(CfgList->Cells[0][row]);
  ReadCfg->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PPConfigRemoveClick(TObject *Sender)
{
  CfgKind->ItemIndex = CfgKind->Items->IndexOf(CfgList->Cells[0][row]);
  DelCfg->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PPConnectionStartClick(TObject *Sender)
{
  ServList->ItemIndex = ServList->Items->IndexOf(CfgList->Cells[0][row]);
  CmdRun->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PPConnectionStopClick(TObject *Sender)
{
  ServList->ItemIndex = ServList->Items->IndexOf(CfgList->Cells[0][row]);
  CmdStop->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::PPConfigAddClick(TObject *Sender)
{
  NewCfg->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CfgListMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
  CfgList->MouseToCell(X, Y, col, row);
}
//---------------------------------------------------------------------------

void __fastcall TAURAForm::CfgListMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
  if (Button == mbRight)
	{
	  if ((row > 0) && (GetConnectionID(CfgList->Cells[0][row]) > 0))
		{
          TPoint cursor;
		  GetCursorPos(&cursor);
		  ConnPopupMenu->Popup(cursor.X, cursor.Y);
        }
	}
}
//---------------------------------------------------------------------------


