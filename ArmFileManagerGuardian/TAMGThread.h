/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TAMGThreadH
#define TAMGThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
//---------------------------------------------------------------------------

//стани роботи з'єднання
#define ES_WAIT 0
#define ES_SUCC_DL 1
#define ES_SUCC_UL 2
#define ES_SUCC_DL_UL 3
#define ES_NO_EXCHANGE -1
#define ES_ERROR_STOP -2

class TAMGThread : public TAMThread
{
private:
	TAMGuardian *Guardian;

	//TAMGuardian *GetGuardian(){return Guardian;}
	//void SetGuardian(TAMGuardian *ptr){Guardian = ptr;}

	void __fastcall Work();

protected:
	void __fastcall Execute();

public:
	__fastcall TAMGThread(bool CreateSuspended);

	__property TAMGuardian *GuardianThread = {read = Guardian, write = Guardian};
};
//---------------------------------------------------------------------------
#endif
