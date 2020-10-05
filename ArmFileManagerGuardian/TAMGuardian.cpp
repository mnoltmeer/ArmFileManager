/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TAMGuardian.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall TAMGuardian::TAMGuardian(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TAMGuardian::Execute()
{
  try
	 {
	   ForceUpdateManager(); //��������� ��������� ���� ������� Guardian
	   SendToManager("#instance_request"); //��������� ����� ������� �������� ���������

	   int passed = 0;

	   while (!Terminated)
		 {
		   if (!Update) //���� �� �������� ��������� ���������
			 {
			   if (!ManagerRunning())
				 {
				   if (FileExists(AppPath + "\\ArmMngr.exe"))
					 {
					   RunManager();
					 }
				   else
					 {
					   Log->Add("³������ ������ ���������");
					   SendToCollector("³������ ������ ���������");

					   if (CopyFile(String(UpdatesDirectory + "\\ArmMngr.exe").c_str(),
									String(AppPath + "\\ArmMngr.exe").c_str(), false) > 0)
						 {
						   Log->Add("³�������� � �������� ��������");
						   SendToCollector("³�������� � �������� ��������");

						   RunManager();
						 }
					 }
				 }
			   else if (passed > 600000) //������� 10 ��.
				 {
				   SendToManager("#instance_request"); //��������� ����� ������� �������� ���������
				   passed = 0;
				 }
			 }
		   else
			 {
			   Log->Add("������ ��������� ���������");
			   SendToCollector("������ ��������� ���������");

			   if (UpdateManager() < 0)
				 {
				   Update = false;
                   SendToManager("#instance_request");
//��������� ��� �������
				   Log->Add("�� ������� ������� ����� ���������");
				   SendToCollector("�� ������� ������� ����� ���������");
				 }
			   else //��������� ��������
				 {
			  	   Update = false;

				   Log->Add("��������� ��������� ���������");
				   SendToCollector("��������� ��������� ���������");

				   if (!ManagerRunning())
					 RunManager();
				   else
                     SendToManager("#instance_request");
				 }
			 }

		   Sleep(100);
		   passed += 100;
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("TAMGuardian::Execute: " + e.ToString());
	   SendToCollector("TAMGuardian::Execute: "  + e.ToString());
	   Terminate();
	 }
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::UpdateManager()
{
  if (NeedToStopManager())
	{
	  if (StopManager() < 0)
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

			GetFileList(files, UpdDir, "*.*", WITHOUT_DIRS, WITH_FULL_PATH);

            for (int i = 0; i < files->Count; i++)
			   {
				 file_name = files->Strings[i];
				 file_name.Delete(1, file_name.LastDelimiter("\\"));

				 upd_files->Add(AppPath + "\\" + file_name);

                 if (FileExists(AppPath + "\\" + file_name))
				   {
//��������� ���� ���� �����
					 int comp_ver = CompareVersions(files->Strings[i].c_str(),
													String(AppPath + "\\" + file_name).c_str());

					 TDateTime date1 = GetFileDateTime(files->Strings[i]); //������������
					 TDateTime date2 = GetFileDateTime(AppPath + "\\" + file_name); //��������

//���� ����� ��� ���� ������������� ����� ����� �� ���� ��������� - ������� �������� ����
					 if (comp_ver == VER_1_HIGHER)
					   {
                         RenameFile(AppPath + "\\" + file_name, AppPath + "\\" + file_name + ".bckp");
						 DeleteFile(AppPath + "\\" + file_name);

						 if (CopyFile(files->Strings[i].c_str(),
							 String(AppPath + "\\" + file_name).c_str(), 0) == 0)
						   {
//��������� ��� ������� ���������
							 Log->Add("������� ��� �������� ����� " + files->Strings[i] +
									 " �� " + AppPath + "\\" + file_name);
							 SendToCollector("������� ��� �������� ����� " + files->Strings[i] +
											 " �� " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
                       }
//���� ����� ��� ���� ������������� ����� ����� �� ���� ��������� - ������� �������� ����
					 else if ((comp_ver == -1) || (date1 > date2))
					   {
						 RenameFile(AppPath + "\\" + file_name, AppPath + "\\" + file_name + ".bckp");
						 DeleteFile(AppPath + "\\" + file_name);

						 if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
						   {
//��������� ��� ������� ���������
							 Log->Add("������� ��� �������� ����� " + files->Strings[i] +
									 " �� " + AppPath + "\\" + file_name);
							 SendToCollector("������� ��� �������� ����� " + files->Strings[i] +
											 " �� " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
					   }
					 else
					   {
						 Log->Add("���� " + AppPath + "\\" + file_name + " �� ������� ���������");
						 SendToCollector("���� " + AppPath + "\\" + file_name + " �� ������� ���������");
					   }
				   }
				 else if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
				   {
					 //��������� ��� ������� ���������
					 Log->Add("������� ��� �������� ����� " + files->Strings[i] +
							 " �� " + AppPath + "\\" + file_name);
					 SendToCollector("������� ��� �������� ����� " + files->Strings[i] +
									 " �� " + AppPath + "\\" + file_name);
					 result = -2;
					 break;
				   }
			   }

			if (result < 0) //���� ���� ������� - �������� �������� �����
			  {
				Log->Add("������� ������� ��� ��������, ���������� ��������� ����� �����...");
				SendToCollector("������� ������� ��� ��������, ���������� ��������� ����� �����...");

				for (int i = 0; i < upd_files->Count; i++) //�������� ��������� ��� �����
				   {
					 if (FileExists(upd_files->Strings[i]))
					   DeleteFile(upd_files->Strings[i]);
                   }

				files->Clear();

				GetFileList(files, AppPath, "*.bckp", WITHOUT_DIRS, WITH_FULL_PATH);

				for (int i = 0; i < files->Count; i++) //�������� ����
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

				for (int i = 0; i < files->Count; i++) //�������� ������ �����
				   DeleteFile(files->Strings[i]);

				files->Clear();
				GetFileList(files, UpdDir, "*.es", WITHOUT_DIRS, WITH_FULL_PATH);

				for (int i = 0; i < files->Count; i++) //�������� ���������� �������
				   DeleteFile(files->Strings[i]);
			  }
		  }
	   catch(Exception &e)
		  {
			Log->Add("��������� ���������: ������� " + e.ToString());
			SendToCollector("��������� ���������: ������� " + e.ToString());
            result = -3;
          }
	 }
  __finally {delete files; delete upd_files;}

  return result;
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::ForceUpdateManager()
{
//��������� �������� ���������� ����� ������� ���������

  bool tmp_force_prm = UseForceStop;

  UseForceStop = true;

  int res = UpdateManager();

  UseForceStop = tmp_force_prm;

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TAMGuardian::RunManager()
{
  try
	 {
	   Log->Add("������ ���������");
	   SendToCollector("������ ���������");

	   ShellExecute(NULL,
					L"open",
					String(AppPath + "\\ArmMngr.exe").c_str(),
					L"",
					NULL,
					SW_SHOWNORMAL);
	 }
  catch (Exception &e)
	 {
	   Log->Add("������ ���������: " + e.ToString());
	   SendToCollector("������ ���������: "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::StopManager()
{
  Log->Add("������� ���������");
  SendToCollector("������� ���������");

  int res;

  if (!ManagerRunning())
	{
	  Log->Add("�������� �� ��������");
	  SendToCollector("�������� �� ��������");

	  return 0;
	}
  else
	{
	  try
		 {
		   Log->Add("�������� �������� ������, �������...");
		   SendToCollector("�������� �������� ������, �������...");

		   SendToManager("#stop%0"); //�������� ������� ������� ��� �'������

		   int wait_cnt = 0;

		   while (ManagerActiveConnections) //������ ���� �������� ��������� ��� �������
			 {
			   Sleep(100);
			   wait_cnt++;

			   if (wait_cnt > 150) //���� ������ 15 ������ - ���������� �� ������� ��������
				 {
				   Log->Add("���� ������ 15 ������ - ������� ��������");
				   SendToCollector("���� ������ 15 ������ - ������� ��������");

				   break;
				 }
			 }

		   wait_cnt = 0;

		   PostMessage(FindHandleByName(L"�������� ����� ������� ��� ��"), WM_QUIT, 0, 0);

		   while (FindHandleByName(L"�������� ����� ������� ��� ��"))
			 {
			   Sleep(100);
			   wait_cnt++;

			   if (wait_cnt > 100) //�������� �� �������, ���� ������� �������� ������
				 {
				   Log->Add("�������� �� ������� 10 ������, ������� ������� �������");
				   SendToCollector("�������� �� ������� 10 ������, ������� ������� �������");

				   DWORD MngrPID = GetProcessByExeName(L"ArmMngr.exe");
				   TerminateManager(MngrPID);
				   break;
				 }
			 }

		   DWORD MngrPID = GetProcessByExeName(L"ArmMngr.exe");

		   if (MngrPID == 0)
			 {
			   Log->Add("�������� �������� �����");
			   SendToCollector("�������� �������� �����");

			   res = 1;
			 }
		   else if (ForceStop) //��������� ��������� ��������
			 {
			   Log->Add("�� ������� �������� �������� ��������, ��������� �������...");
			   SendToCollector("�� ������� �������� �������� ��������, ��������� �������...");

			   if (TerminateManager(MngrPID) > 0)
				 {
				   Log->Add("������ ��������� ���� ��������� ��������");
				   SendToCollector("������ ��������� ���� ��������� ��������");

				   res = 2;
				 }
			   else
				 {
				   Log->Add("�� ������� ��������� �������� ��������");
				   SendToCollector("�� ������� ��������� �������� ��������");

			  	   res = -2; //�� ������� �������� ������ �� ��������
				 }
			 }
		   else
			 {
			   Log->Add("�� ������� �������� �������� ��������");
			   SendToCollector("�� ������� �������� �������� ��������");

			   res = -1; //�� ������� �������� �������� ��������
			 }
		 }
	   catch (Exception &e)
		 {
		   Log->Add("������� ���������: " + e.ToString());
		   SendToCollector("������� ���������: "  + e.ToString());
		   res = -1;
		 }
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::TerminateManager(DWORD mngr_pid)
{
  int res;
  HANDLE proc;

  Log->Add("������� ������� ������� ���������");
  SendToCollector("������� ������� ������� ���������");

  try
	 {
	   proc = OpenProcess(PROCESS_TERMINATE, 0, mngr_pid);
	 }
  catch (Exception &e)
	 {
	   Log->Add("���������� ������� ���������: " + e.ToString());
	   SendToCollector("���������� ������� ���������: "  + e.ToString());
	   res = -1;
	 }

  if (proc)
	{
	  try
		 {
		   try
			  {
				TerminateProcess(proc, 0);
				res = 1;
				Log->Add("�������� ������� ���������: ��������");
				SendToCollector("�������� ������� ���������: ��������");
			  }
			catch (Exception &e)
			  {
				Log->Add("�������� ������� ���������: " + e.ToString());
				SendToCollector("�������� ������� ���������: "  + e.ToString());
				res = -1;
			  }
		 }
	  __finally {CloseHandle(proc);}
	}
  else
	{
	  Log->Add("�� ������� �������� ������ �� �������");
	  SendToCollector("�� ������� �������� ������ �� �������");

	  res = 0; //�� ������� �������� ������ �� ��������
	}

  return res;
}
//---------------------------------------------------------------------------

bool __fastcall TAMGuardian::NeedToStopManager()
{
  bool result = false;
  TStringList *upd_files = new TStringList();

  try
	 {
	   try
		  {
			String file_name;

			GetFileList(upd_files, UpdDir, "*.*", WITHOUT_DIRS, WITH_FULL_PATH);

			for (int i = 0; i < upd_files->Count; i++)
			   {
				 file_name = upd_files->Strings[i];
				 file_name.Delete(1, file_name.LastDelimiter("\\"));

				 if (IsImportantFile(file_name.c_str()))
				   {
					 if (GetFileDateTime(upd_files->Strings[i]) > GetFileDateTime(AppPath + "\\" + file_name))
					   {
						 result = true;
						 Log->Add("�������� ��������� �������� ����, ������� ������� ���������");
						 SendToCollector("�������� ��������� �������� ����, ������� ������� ���������");
					 	 break;
					   }
				   }
			   }
		  }
       catch(Exception &e)
		  {
			Log->Add("�������� ����� ��������: ������� " + e.ToString());
			SendToCollector("�������� ����� ��������: ������� " + e.ToString());
		  }
	 }
  __finally {delete upd_files;}

  return result;
}
//---------------------------------------------------------------------------

bool __fastcall TAMGuardian::ManagerRunning()
{
  DWORD MngrPID;

  try
	 {
	   MngrPID = GetProcessByExeName(L"ArmMngr.exe");
	 }
  catch (Exception &e)
	 {
	   Log->Add("�������� �� ��������� ������� ���������: " + e.ToString());
	   SendToCollector("�������� �� ��������� ������� ���������: "  + e.ToString());
	 }

  if (MngrPID == 0)
	return false;
  else
   return true;
}
//-------------------------------------------------------------------------

int __fastcall TAMGuardian::SendToCollector(String text)
{
  if (!UseCollector)
	return 0;

  int res;
  String msg = RegionID + "&" +
			   IndexVZ + ":" + StationID + "&" +
			   "Guardian" + "&" +
			   DateToStr(Date()) + "&" +
			   TimeToStr(Time()) + "&" +
			   text;
  TStringStream *ms = new TStringStream(msg, TEncoding::UTF8, true);

  ms->Position = 0;

  try
	 {
	   try
		  {
            if (!Collector->Connected() && (CollectorHost != ""))
		 	  Collector->Connect(CollectorHost, CollectorPort);

			Collector->IOHandler->Write(ms, ms->Size, true);
			res = 1;
		  }
	   catch (Exception &e)
		  {
			Log->Add("³������� �� ����������: " + e.ToString());
            res = -1;
		  }
	 }
  __finally
	 {
	   delete ms;

	   if (Collector->Connected())
		 Collector->Disconnect();
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TAMGuardian::SendToManager(String text)
{
  try
	 {
	   TStringStream *ms = new TStringStream(text, TEncoding::UTF8, true);

	   try
		  {
			TIdTCPClient *sender = CreateSimpleTCPSender(L"127.0.0.1", UsedMngrAdmPort);

            sender->Connect();

			Log->Add("���������� ��������� �������: " + text);
			SendToCollector("���������� ��������� �������: " + text);

			ms->Position = 0;
			sender->IOHandler->Write(ms, ms->Size, true);

            FreeSimpleTCPSender(sender);
		  }
	   __finally {delete ms;}
	 }
  catch (Exception &e)
	 {
	   Log->Add("���������� ������ ���������: " + e.ToString());
	   SendToCollector("���������� ������ ���������: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

bool IsImportantFile(const wchar_t *filename)
{
  for (int i = 0; i < IFsz; i++)
	 if (StrCmpICW(filename, ImportantFiles[i]) == 0)
	   return true;

  return false;
}
//---------------------------------------------------------------------------
