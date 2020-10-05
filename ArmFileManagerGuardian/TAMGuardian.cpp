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
	   ForceUpdateManager(); //примусове оновлення після запуску Guardian
	   SendToManager("#instance_request"); //надсилаємо запит статусу інстансу Менеджера

	   int passed = 0;

	   while (!Terminated)
		 {
		   if (!Update) //якщо не запущено оновлення Менеджеру
			 {
			   if (!ManagerRunning())
				 {
				   if (FileExists(AppPath + "\\ArmMngr.exe"))
					 {
					   RunManager();
					 }
				   else
					 {
					   Log->Add("Відсутній модуль Менеджера");
					   SendToCollector("Відсутній модуль Менеджера");

					   if (CopyFile(String(UpdatesDirectory + "\\ArmMngr.exe").c_str(),
									String(AppPath + "\\ArmMngr.exe").c_str(), false) > 0)
						 {
						   Log->Add("Відновлено з каталогу оновлень");
						   SendToCollector("Відновлено з каталогу оновлень");

						   RunManager();
						 }
					 }
				 }
			   else if (passed > 600000) //пройшло 10 хв.
				 {
				   SendToManager("#instance_request"); //надсилаємо запит статусу інстансу Менеджера
				   passed = 0;
				 }
			 }
		   else
			 {
			   Log->Add("Запуск оновлення Менеджера");
			   SendToCollector("Запуск оновлення Менеджера");

			   if (UpdateManager() < 0)
				 {
				   Update = false;
                   SendToManager("#instance_request");
//сповіщення про помилку
				   Log->Add("Не вдалось оновити модулі Менеджера");
				   SendToCollector("Не вдалось оновити модулі Менеджера");
				 }
			   else //запускаємо Менеджер
				 {
			  	   Update = false;

				   Log->Add("Оновлення Менеджера завершено");
				   SendToCollector("Оновлення Менеджера завершено");

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
							 Log->Add("Помилка при копіюванні файлу " + files->Strings[i] +
									 " до " + AppPath + "\\" + file_name);
							 SendToCollector("Помилка при копіюванні файлу " + files->Strings[i] +
											 " до " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
                       }
//якщо версія або дата завантаженого файлу більша за дату існуючого - оновимо існуючий файл
					 else if ((comp_ver == -1) || (date1 > date2))
					   {
						 RenameFile(AppPath + "\\" + file_name, AppPath + "\\" + file_name + ".bckp");
						 DeleteFile(AppPath + "\\" + file_name);

						 if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
						   {
//сповіщення про невдале копіювання
							 Log->Add("Помилка при копіюванні файлу " + files->Strings[i] +
									 " до " + AppPath + "\\" + file_name);
							 SendToCollector("Помилка при копіюванні файлу " + files->Strings[i] +
											 " до " + AppPath + "\\" + file_name);
							 result = -2;
							 break;
						   }
					   }
					 else
					   {
						 Log->Add("Файл " + AppPath + "\\" + file_name + " не потребує оновлення");
						 SendToCollector("Файл " + AppPath + "\\" + file_name + " не потребує оновлення");
					   }
				   }
				 else if (CopyFile(files->Strings[i].c_str(),
							  String(AppPath + "\\" + file_name).c_str(), 0) == 0)
				   {
					 //сповіщення про невдале копіювання
					 Log->Add("Помилка при копіюванні файлу " + files->Strings[i] +
							 " до " + AppPath + "\\" + file_name);
					 SendToCollector("Помилка при копіюванні файлу " + files->Strings[i] +
									 " до " + AppPath + "\\" + file_name);
					 result = -2;
					 break;
				   }
			   }

			if (result < 0) //якщо були помилки - відновимо попередні файли
			  {
				Log->Add("Виявлені помилки при оновленні, відновлення попередніх версій файлів...");
				SendToCollector("Виявлені помилки при оновленні, відновлення попередніх версій файлів...");

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

				files->Clear();
				GetFileList(files, UpdDir, "*.es", WITHOUT_DIRS, WITH_FULL_PATH);

				for (int i = 0; i < files->Count; i++) //видалимо завантажені скрипти
				   DeleteFile(files->Strings[i]);
			  }
		  }
	   catch(Exception &e)
		  {
			Log->Add("Оновлення Менеджера: помилка " + e.ToString());
			SendToCollector("Оновлення Менеджера: помилка " + e.ToString());
            result = -3;
          }
	 }
  __finally {delete files; delete upd_files;}

  return result;
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::ForceUpdateManager()
{
//тимчасово ввімкнемо форсований режим зупинки Менеджеру

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
	   Log->Add("Запуск Менеджера");
	   SendToCollector("Запуск Менеджера");

	   ShellExecute(NULL,
					L"open",
					String(AppPath + "\\ArmMngr.exe").c_str(),
					L"",
					NULL,
					SW_SHOWNORMAL);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Запуск Менеджера: " + e.ToString());
	   SendToCollector("Запуск Менеджера: "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TAMGuardian::StopManager()
{
  Log->Add("Зупинка Менеджера");
  SendToCollector("Зупинка Менеджера");

  int res;

  if (!ManagerRunning())
	{
	  Log->Add("Менеджер не запущено");
	  SendToCollector("Менеджер не запущено");

	  return 0;
	}
  else
	{
	  try
		 {
		   Log->Add("Знайдено активний процес, зупинка...");
		   SendToCollector("Знайдено активний процес, зупинка...");

		   SendToManager("#stop%0"); //посилаємо команду зупинки всіх з'єднань

		   int wait_cnt = 0;

		   while (ManagerActiveConnections) //чекаємо доки Менеджер повідомить про зупинку
			 {
			   Sleep(100);
			   wait_cnt++;

			   if (wait_cnt > 150) //нема відповіді 15 секунд - переходимо до зупинки аплікації
				 {
				   Log->Add("Немає відповіді 15 секунд - зупинка аплікації");
				   SendToCollector("Немає відповіді 15 секунд - зупинка аплікації");

				   break;
				 }
			 }

		   wait_cnt = 0;

		   PostMessage(FindHandleByName(L"Менеджер обміну файлами АРМ ВЗ"), WM_QUIT, 0, 0);

		   while (FindHandleByName(L"Менеджер обміну файлами АРМ ВЗ"))
			 {
			   Sleep(100);
			   wait_cnt++;

			   if (wait_cnt > 100) //Менеджер не відповідає, тому аварійно вирубаємо процес
				 {
				   Log->Add("Менеджер не відповідає 10 секунд, аварійна зупинка процесу");
				   SendToCollector("Менеджер не відповідає 10 секунд, аварійна зупинка процесу");

				   DWORD MngrPID = GetProcessByExeName(L"ArmMngr.exe");
				   TerminateManager(MngrPID);
				   break;
				 }
			 }

		   DWORD MngrPID = GetProcessByExeName(L"ArmMngr.exe");

		   if (MngrPID == 0)
			 {
			   Log->Add("Менеджер зупинено вдало");
			   SendToCollector("Менеджер зупинено вдало");

			   res = 1;
			 }
		   else if (ForceStop) //примусово зупиняємо Менеджер
			 {
			   Log->Add("Не вдалось коректно зупинити Менеджер, примусова зупинка...");
			   SendToCollector("Не вдалось коректно зупинити Менеджер, примусова зупинка...");

			   if (TerminateManager(MngrPID) > 0)
				 {
				   Log->Add("Процес Менеджера було примусово зупинено");
				   SendToCollector("Процес Менеджера було примусово зупинено");

				   res = 2;
				 }
			   else
				 {
				   Log->Add("Не вдалось примусово зупинити Менеджер");
				   SendToCollector("Не вдалось примусово зупинити Менеджер");

			  	   res = -2; //не вдалося отримати доступ до процессу
				 }
			 }
		   else
			 {
			   Log->Add("Не вдалось коректно зупинити Менеджер");
			   SendToCollector("Не вдалось коректно зупинити Менеджер");

			   res = -1; //не вдалось коректно зупинити Менеджер
			 }
		 }
	   catch (Exception &e)
		 {
		   Log->Add("Зупинка Менеджера: " + e.ToString());
		   SendToCollector("Зупинка Менеджера: "  + e.ToString());
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

  Log->Add("Аварійна зупинка процесу Менеджера");
  SendToCollector("Аварійна зупинка процесу Менеджера");

  try
	 {
	   proc = OpenProcess(PROCESS_TERMINATE, 0, mngr_pid);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Захоплення процесу Менеджера: " + e.ToString());
	   SendToCollector("Захоплення процесу Менеджера: "  + e.ToString());
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
				Log->Add("Знищення процесу Менеджера: виконано");
				SendToCollector("Знищення процесу Менеджера: виконано");
			  }
			catch (Exception &e)
			  {
				Log->Add("Знищення процесу Менеджера: " + e.ToString());
				SendToCollector("Знищення процесу Менеджера: "  + e.ToString());
				res = -1;
			  }
		 }
	  __finally {CloseHandle(proc);}
	}
  else
	{
	  Log->Add("Не вдалось отримати доступ до процесу");
	  SendToCollector("Не вдалось отримати доступ до процесу");

	  res = 0; //не вдалося отримати доступ до процессу
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
						 Log->Add("Знайдено оновлений ключовий файл, потрібна зупинка Менеджера");
						 SendToCollector("Знайдено оновлений ключовий файл, потрібна зупинка Менеджера");
					 	 break;
					   }
				   }
			   }
		  }
       catch(Exception &e)
		  {
			Log->Add("Перевірка файлів оновлень: помилка " + e.ToString());
			SendToCollector("Перевірка файлів оновлень: помилка " + e.ToString());
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
	   Log->Add("Перевірка на існування процесу Менеджера: " + e.ToString());
	   SendToCollector("Перевірка на існування процесу Менеджера: "  + e.ToString());
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
			Log->Add("Відправка до Коллектору: " + e.ToString());
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

			Log->Add("Надсилання Менеджеру команди: " + text);
			SendToCollector("Надсилання Менеджеру команди: " + text);

			ms->Position = 0;
			sender->IOHandler->Write(ms, ms->Size, true);

            FreeSimpleTCPSender(sender);
		  }
	   __finally {delete ms;}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Надсилання команд Менеджеру: " + e.ToString());
	   SendToCollector("Надсилання команд Менеджеру: " + e.ToString());
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
