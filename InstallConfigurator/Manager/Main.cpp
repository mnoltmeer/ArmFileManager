//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\..\work-functions\MyFunc.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCfgForm *CfgForm;
String AppPath;
//---------------------------------------------------------------------------
__fastcall TCfgForm::TCfgForm(TComponent* Owner)
	: TForm(Owner)
{
  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));
}
//---------------------------------------------------------------------------

void __fastcall TCfgForm::SaveClick(TObject *Sender)
{
  try
	 {
	   SetConfigLine(AppPath + "\\main.cfg", "StationID", StationID->Text);
	   SetConfigLine(AppPath + "\\main.cfg", "IndexVZ", IndexVZ->Text);
	   SetConfigLine(AppPath + "\\main.cfg", "RegionID", RegionID->Text);
	   SetConfigLine(AppPath + "\\main.cfg", "RemAdmPort", RemAdmPort->Text);

	   if (UseGuardian->Checked)
		 SetConfigLine(AppPath + "\\main.cfg", "UseGuardian", "1");
	   else
		 SetConfigLine(AppPath + "\\main.cfg", "UseGuardian", "0");

	   if (AddFirewallRule->Checked)
		 SetConfigLine(AppPath + "\\main.cfg", "FirewallRule", "0");
	   else
		 SetConfigLine(AppPath + "\\main.cfg", "FirewallRule", "1");

	   if (HideWindow->Checked)
		 SetConfigLine(AppPath + "\\main.cfg", "HideWnd", "1");
	   else
		 SetConfigLine(AppPath + "\\main.cfg", "HideWnd", "0");

	   if (AddAutoStart->Checked)
		 {
		   SetConfigLine(AppPath + "\\main.cfg", "EnableAutoStart", "1");
		   SetConfigLine(AppPath + "\\main.cfg", "AutoStartForAllUsers", IntToStr((int)AutoStartForAll->Checked));
		   AddAppAutoStart("ArmFileManager", AppPath + "\\ArmMngr.exe", AutoStartForAll->Checked);
		 }
	   else
		 {
		   SetConfigLine(AppPath + "\\main.cfg", "EnableAutoStart", "0");
		   RemoveAppAutoStart("ArmFileManager", FOR_CURRENT_USER);
		   RemoveAppAutoStart("ArmFileManager", FOR_ALL_USERS);
		 }
	 }
  catch (Exception &e)
	 {
	   SaveLog(AppPath + "\\exceptions.log", e.ToString());
	 }

  Close();
}
//---------------------------------------------------------------------------
void __fastcall TCfgForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  if (OpenCfgAfterExit->Checked)
	{
	  ShellExecute(NULL,
				   L"open",
				   L"notepad.exe",
				   String(AppPath + "\\main.cfg").c_str(),
				   NULL,
				   SW_SHOWNORMAL);
    }
}
//---------------------------------------------------------------------------

