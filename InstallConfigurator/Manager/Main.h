//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TCfgForm : public TForm
{
__published:	// IDE-managed Components
	TCheckBox *AddAutoStart;
	TCheckBox *AddFirewallRule;
	TCheckBox *UseGuardian;
	TCheckBox *HideWindow;
	TLabel *Label2;
	TEdit *RemAdmPort;
	TCheckBox *OpenCfgAfterExit;
	TButton *Save;
	TLabel *Label1;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *StationID;
	TEdit *IndexVZ;
	TEdit *RegionID;
	TCheckBox *AutoStartForAll;
	void __fastcall SaveClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
	__fastcall TCfgForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCfgForm *CfgForm;
//---------------------------------------------------------------------------
#endif
