/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ValEdit.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ComCtrls.hpp>

//---------------------------------------------------------------------------
class TAURAForm : public TForm
{
__published:	// IDE-managed Components
	TValueListEditor *CfgList;
	TEdit *Host;
	TButton *Connect;
	TButton *ReadCfg;
	TButton *SendCfg;
	TEdit *Port;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TComboBox *CfgKind;
	TMemo *Log;
	TButton *GetLog;
	TButton *GetStatus;
	TButton *CmdRun;
	TButton *CmdStop;
	TComboBox *ServList;
	TLabel *Label4;
	TEdit *RemCompFilePath;
	TButton *GetFile;
	TLabel *GetFileStatus;
	TButton *GetThreadList;
	TLabel *Label5;
	TMemo *ActionLog;
	TLabel *Label6;
	TButton *Shutdown;
	TButton *NewCfg;
	TOpenDialog *OpenCfgDialog;
	TLabel *Label7;
	TLabel *ModuleVersion;
	TButton *RestartGuard;
	TButton *PacketCmdSend;
	TBitBtn *EditBook;
	TBitBtn *DeleteFromBook;
	TBitBtn *AddToBook;
	TBitBtn *AddGroupBook;
	TTreeView *AddrList;
	TBitBtn *ImportInAddrBook;
	TButton *SaveCfgList;
	TButton *DelCfg;
	TBitBtn *ExportFromAddrBook;
	TSaveDialog *SaveCfgDialog;
	TButton *SaveLog;
	void __fastcall ConnectClick(TObject *Sender);
	void __fastcall PortClick(TObject *Sender);
	void __fastcall ReadCfgClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall SendCfgClick(TObject *Sender);
	void __fastcall HostKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall PortKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall GetLogClick(TObject *Sender);
	void __fastcall GetStatusClick(TObject *Sender);
	void __fastcall CmdRunClick(TObject *Sender);
	void __fastcall CmdStopClick(TObject *Sender);
	void __fastcall GetFileClick(TObject *Sender);
	void __fastcall GetThreadListClick(TObject *Sender);
	void __fastcall AddrListClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall AddrListDblClick(TObject *Sender);
	void __fastcall NewCfgClick(TObject *Sender);
	void __fastcall ShutdownClick(TObject *Sender);
	void __fastcall RestartGuardClick(TObject *Sender);
	void __fastcall PacketCmdSendClick(TObject *Sender);
	void __fastcall EditBookClick(TObject *Sender);
	void __fastcall AddToBookClick(TObject *Sender);
	void __fastcall DeleteFromBookClick(TObject *Sender);
	void __fastcall AddGroupBookClick(TObject *Sender);
	void __fastcall ImportInAddrBookClick(TObject *Sender);
	void __fastcall SaveCfgListClick(TObject *Sender);
	void __fastcall DelCfgClick(TObject *Sender);
	void __fastcall ExportFromAddrBookClick(TObject *Sender);
	void __fastcall SaveLogClick(TObject *Sender);

private:	// User declarations
	int ReadTmpCfg(String cfg);
	int ReadServerList();
	int ReadRemoteVersion();
	int AskToServer(const wchar_t *host, int port, TStringStream *rw_bufer);
	int SendToServer(const wchar_t *host, int port, TStringStream *rw_bufer);
	TIdTCPClient *CreateSender(const wchar_t *host, int port);
	void FreeSender(TIdTCPClient *sender);
	void AddActionLog(String status);
    void __fastcall AURAClientConnected(TObject *Sender);
	void __fastcall AURAClientDisconnected(TObject *Sender);
    void CreateNewForm();
	void CreateEditForm();
	void CreatePacketForm();
    void CreateRecpList(std::vector<int> *recp_list);

public:		// User declarations
	__fastcall TAURAForm(TComponent* Owner);

	void __fastcall EditFormShow(TObject *Sender);
	void __fastcall EditApplyClick(TObject *Sender);
	void __fastcall EditCancelClick(TObject *Sender);

	void __fastcall NewFormShow(TObject *Sender);
	void __fastcall NewApplyClick(TObject *Sender);
	void __fastcall NewCancelClick(TObject *Sender);

	void __fastcall PacketFormShow(TObject *Sender);
	void __fastcall PackApplyClick(TObject *Sender);
	void __fastcall PackCancelClick(TObject *Sender);
	void __fastcall RecpListMouseWheelDown(TObject *Sender,
										   TShiftState Shift,
										   const TPoint &MousePos,
										   bool &Handled);
	void __fastcall RecpListMouseWheelUp(TObject *Sender,
										 TShiftState Shift,
										 const TPoint &MousePos,
										 bool &Handled);
	void __fastcall PackAllClick(TObject *Sender);
	void __fastcall RecpListMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall PackGroupListChange(TObject *Sender);
};
//---------------------------------------------------------------------------
extern PACKAGE TAURAForm *AURAForm;

//---------------------------------------------------------------------------
#endif
