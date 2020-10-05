/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TAMEliThreadH
#define TAMEliThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
//---------------------------------------------------------------------------

#include "eli_interface.h"

class TAMEliThread : public TThread
{
private:
	double passed, interval;
	ELI_INTERFACE *iface;
	String script_path;
	bool logging;

	/*ELI_INTERFACE *GetIface(){return iface;}
	void SetIface(ELI_INTERFACE *val){if (val){iface = val;}}

	String GetPath(){return script_path;}
	void SetPath(String val){script_path = val;}

	bool GetLogging(){return logging;}
	void SetLogging(bool val){logging = val;}*/

protected:
	void __fastcall Execute();
public:
	__fastcall TAMEliThread(bool CreateSuspended);
	inline void __fastcall SetRunInterval(int val){if (val > 0){interval = val * 60000;}};

	__property ELI_INTERFACE *ELIInterface = {read = iface, write = iface};
	__property String ScriptPath = {read = script_path, write = script_path};
	__property bool Logging = {read = logging, write = logging};
};
//---------------------------------------------------------------------------
#endif
