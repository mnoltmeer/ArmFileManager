/*!
Copyright 2019 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Data.DB.hpp>
#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.Comp.DataSet.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.DatS.hpp>
#include <FireDAC.Phys.hpp>
#include <FireDAC.Phys.Intf.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Error.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>
#include <FireDAC.Stan.Param.hpp>
#include <FireDAC.Stan.Pool.hpp>
#include <FireDAC.UI.Intf.hpp>
#include <FireDAC.VCLUI.Wait.hpp>
#include <FireDAC.Phys.FB.hpp>
#include <FireDAC.Phys.FBDef.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <FireDAC.Phys.IBBase.hpp>
#include <IdCustomHTTPServer.hpp>
#include <IdHTTPServer.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Menus.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TFDConnection *StatConnection;
	TFDTransaction *WriteTransaction;
	TFDQuery *WriteQuery;
	TLabel *Label1;
	TLabel *Label2;
	TImage *Image2;
	TImage *Image3;
	TImage *Image4;
	TLabel *SucsTransCnt;
	TLabel *RollTransCnt;
	TLabel *ActTransCnt;
	TIdTCPServer *AMColServer;
	TMemo *LastRecLog;
	TFDPhysFBDriverLink *StatFBDriverLink;
	TTimer *ProcessingTimer;
	TLabel *PoolCount;
	TImage *Image1;
	TIdHTTPServer *HttpServer;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *ModuleVersion;
	TImageList *MenuImages;
	TTrayIcon *TrayIcon;
	TPopupMenu *PopupMenu;
	TMenuItem *IconPP3;
	TMenuItem *IconPP2;
	TMenuItem *IconPP1;
	TMenuItem *IconPP4;
	TMenuItem *IconPP5;
	TMenuItem *IconPP6;
	TImage *SwHttpOff;
	TImage *SwServerOff;
	TImage *SwServerOn;
	TImage *SwDBOff;
	TImage *SwDBOn;
	TImage *SwHttpOn;
	void __fastcall AMColServerExecute(TIdContext *AContext);
	void __fastcall SwServerOnClick(TObject *Sender);
	void __fastcall SwDBOnClick(TObject *Sender);
	void __fastcall SwServerOffClick(TObject *Sender);
	void __fastcall SwDBOffClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall StatConnectionAfterDisconnect(TObject *Sender);
	void __fastcall WriteTransactionAfterCommit(TObject *Sender);
	void __fastcall WriteTransactionAfterRollback(TObject *Sender);
	void __fastcall WriteTransactionAfterStartTransaction(TObject *Sender);
	void __fastcall ProcessingTimerTimer(TObject *Sender);
	void __fastcall HttpServerCommandGet(TIdContext *AContext, TIdHTTPRequestInfo *ARequestInfo,
          TIdHTTPResponseInfo *AResponseInfo);
	void __fastcall SwHttpOnClick(TObject *Sender);
	void __fastcall SwHttpOffClick(TObject *Sender);
	void __fastcall IconPP5Click(TObject *Sender);
	void __fastcall IconPP4Click(TObject *Sender);
	void __fastcall IconPP3Click(TObject *Sender);
	void __fastcall IconPP2Click(TObject *Sender);
	void __fastcall IconPP1Click(TObject *Sender);
	void __fastcall PopupMenuPopup(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
	void ConnectToDB();
	void DisconnectFromDB();
	void ShowServerNoCon();
	void ShowDBNoConn();
	void ShowServerCon();
	void ShowDBConn();
	void ShowHttpCon();
    void ShowHttpNoCon();
	void StartServer();
	void StopServer();
	void StartHttpServer();
	void StopHttpServer();
    void ProcessingMessages();
    int WorkWithDB(String args);
	int WriteToDB(String region_id,
				  String indexvz,
				  String date,
				  String time,
				  String event_type,
				  String text);
	void StartTransaction();
	void RollbackTransaction();
	void CommitTransaction();
	TStringStream *RequestToDB(TStringStream *ms,
							   String p_region,
							   String p_indexvz,
							   String p_evtype,
                               String p_evtxt,
							   String p_datefrom,
							   String p_dateto,
							   String p_sortby);
	TStringStream *RequestToDB(TStringStream *ms, unsigned int view_type);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;

enum {vt_events = 0, vt_files = 1, vt_today = 2, vt_guard = 3};

TFDTransaction *CreateNewTransactionObj();
TFDQuery *CreateNewQueryObj(TFDTransaction *);
void DeleteTransactionObj(TFDTransaction *t);
void DeleteQueryObj(TFDQuery *q);
//---------------------------------------------------------------------------
#endif
