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
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>

//---------------------------------------------------------------------------
class TAURAForm : public TForm
{
__published:	// IDE-managed Components
	TOpenDialog *OpenCfgDialog;
	TSaveDialog *SaveCfgDialog;
	TPanel *LogPanel;
	TMemo *Log;
	TPanel *MainPanel;
	TPanel *AddrBookPanel;
	TTreeView *AddrList;
	TPanel *ActionLogPanel;
	TMemo *ActionLog;
	TPanel *Panel6;
	TLabel *Label6;
	TButton *SaveLog;
	TPanel *ControlsPanel;
	TValueListEditor *CfgList;
	TPanel *AdrBookBtPanel;
	TBitBtn *AddGroupBook;
	TBitBtn *AddToBook;
	TBitBtn *DeleteFromBook;
	TBitBtn *EditBook;
	TBitBtn *ImportInAddrBook;
	TBitBtn *ExportFromAddrBook;
	TPanel *LogFilterPanel;
	TLabel *Label9;
	TComboBox *LogFilter;
	TPopupMenu *ConnPopupMenu;
	TMenuItem *PPConfig;
	TMenuItem *PPConnection;
	TMenuItem *PPConfigShow;
	TMenuItem *PPConfigRemove;
	TMenuItem *PPConnectionStart;
	TMenuItem *PPConnectionStop;
	TMenuItem *PPConfigAdd;
	TPanel *ControlHeaderPanel;
	TLabel *Label2;
	TEdit *Host;
	TLabel *Label3;
	TEdit *Port;
	TLabel *Label1;
	TButton *Connect;
	TLabel *Label7;
	TLabel *ModuleVersion;
	TPanel *ControlFooterPanel;
	TButton *SaveCfgList;
	TButton *GetStatus;
	TButton *GetThreadList;
	TButton *GetLog;
	TLabel *Label5;
	TLabel *Label8;
	TComboBox *CfgKind;
	TComboBox *ServList;
	TButton *ReadCfg;
	TButton *NewCfg;
	TButton *CmdRun;
	TButton *CmdStop;
	TButton *SendCfg;
	TButton *DelCfg;
	TLabel *Label4;
	TEdit *RemCompFilePath;
	TButton *GetFile;
	TLabel *GetFileStatus;
	TButton *Shutdown;
	TButton *RestartGuard;
	TButton *PacketCmdSend;
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
	void __fastcall LogFilterChange(TObject *Sender);
	void __fastcall PPConfigShowClick(TObject *Sender);
	void __fastcall PPConfigRemoveClick(TObject *Sender);
	void __fastcall PPConnectionStartClick(TObject *Sender);
	void __fastcall PPConnectionStopClick(TObject *Sender);
	void __fastcall CfgListMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall CfgListMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall PPConfigAddClick(TObject *Sender);

private:	// User declarations
	int __fastcall ReadTmpCfg(String cfg);
	int __fastcall ReadServerList();
	int __fastcall ReadRemoteVersion();
	int __fastcall AskToServer(const wchar_t *host, int port, TStringStream *rw_bufer);
	int __fastcall SendToServer(const wchar_t *host, int port, TStringStream *rw_bufer);
	TIdTCPClient* __fastcall CreateSender(const wchar_t *host, int port);
	void __fastcall FreeSender(TIdTCPClient *sender);
	void __fastcall AddActionLog(String status);
    void __fastcall AURAClientConnected(TObject *Sender);
	void __fastcall AURAClientDisconnected(TObject *Sender);
	void __fastcall CreateNewForm();
	void __fastcall CreateEditForm();
	void __fastcall CreatePacketForm();
	void __fastcall CreateRecpList(std::vector<int> *recp_list);
	void __fastcall RequestLog(int conn_id);
	int __fastcall GetConnectionID(const String &str_with_id);

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
