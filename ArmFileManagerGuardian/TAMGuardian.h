/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TAMGuardianH
#define TAMGuardianH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <IdFTP.hpp>
#include "..\..\MyFunc\ThreadSafeLog.h"
//---------------------------------------------------------------------------

const int IFsz = 3;
static wchar_t ImportantFiles[IFsz][32] = {L"ArmMngr.exe",
										   L"main.cfg",
										   L"ELI.dll"};

class TAMGuardian : public TThread
{
private:
	bool Update;
	bool ForceStop;
	String UpdDir;
	TIdTCPClient *Collector;
	TThreadSafeLog *Log;
    bool UseCollector;
	String CollectorHost;
	int CollectorPort;
	int MngrAdmPort;
	bool MngrActConn;

protected:
	void __fastcall Execute();
public:
	__fastcall TAMGuardian(bool CreateSuspended);
	int __fastcall UpdateManager();
	int __fastcall ForceUpdateManager();
	int __fastcall StopManager();
	void __fastcall RunManager();
	int __fastcall TerminateManager(DWORD mngr_pid);
	bool __fastcall NeedToStopManager();
	bool __fastcall ManagerRunning();
	int __fastcall SendToCollector(String text);
    void __fastcall SendToManager(String text);

	__property bool UpdateStarted = {read = Update, write = Update};
	__property bool UseForceStop = {read = ForceStop, write = ForceStop};
	__property String UpdatesDirectory = {read = UpdDir, write = UpdDir};
	__property bool UsingCollector = {read = UseCollector, write = UseCollector};
	__property TIdTCPClient *CollectorServer = {read = Collector, write = Collector};
	__property TThreadSafeLog *ServiceLog = {read = Log, write = Log};
	__property String UsedCollectorHost = {read = CollectorHost, write = CollectorHost};
	__property int UsedCollectorPort = {read = CollectorPort, write = CollectorPort};
	__property int UsedMngrAdmPort = {read = MngrAdmPort, write = MngrAdmPort};
	__property bool ManagerActiveConnections = {read = MngrActConn, write = MngrActConn};
};
//---------------------------------------------------------------------------

bool IsImportantFile(const wchar_t *filename);
#endif
