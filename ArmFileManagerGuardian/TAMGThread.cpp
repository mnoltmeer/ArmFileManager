/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TAMGuardian.h"
#include "TAMThread.h"
#include "TAMGThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TAMGThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TAMGThread::TAMGThread(bool CreateSuspended)
	: TAMThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TAMGThread::Execute()
{
  try
	 {
	   Work();
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "ID: " + IntToStr(Connection->ServerID) + ", " +
								 "Thread: " + IntToStr(int(Connection->ServerThreadID)) + ", " +
								 "TAMGThread::Execute(): " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAMGThread::Work()
{
  if (!Connection)
    throw new Exception("Некоректний вказівник TExchangeConnect*");

  int ex_res;

  PassedTime = Connection->ConnectionConfig->MonitoringInterval;
  WaitingInterval = Connection->ConnectionConfig->MonitoringInterval;

  while (!Terminated)
	{
	  if (Connection->Working() &&
		  Connection->ConnectionConfig->StartAtTime &&
		  Connection->ConnectionConfig->TimeStart > Time())
		{
		  Connection->ServerStatus = "Очікування...";
		  this->Sleep(300);
		}
	  else if (Connection->Working() && (PassedTime >= WaitingInterval))
		{
		  PassedTime = 0;
		  ex_res = Connection->Exchange();

		  switch (ex_res)
			{
			  case ES_SUCC_DL:
				{
				  if (Connection->ConnectionConfig->LocDirDl == Guardian->UpdatesDirectory)
					Guardian->UpdateStarted = true;

				  break;
				}

			  case ES_SUCC_DL_UL:
				{
				  if (Connection->ConnectionConfig->LocDirDl == Guardian->UpdatesDirectory)
					Guardian->UpdateStarted = true;

				  break;
				}

			  case ES_ERROR_STOP: Terminate(); break;
			}

		  if (Connection->ConnectionConfig->RunOnce)
			Connection->Stop();
		}
	  else if (Connection->Working() && (PassedTime < WaitingInterval))
		{
		  this->Sleep(100);
		  PassedTime += 100;
		}
	  else if (!Connection->Working())
        this->Sleep(300);
	}
}
//---------------------------------------------------------------------------
