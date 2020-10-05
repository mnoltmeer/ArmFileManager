object CfgForm: TCfgForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = #1053#1072#1083#1072#1096#1090#1091#1074#1072#1085#1085#1103' '#1052#1077#1085#1077#1076#1078#1077#1088#1091' '#1092#1072#1081#1083#1110#1074' '#1040#1056#1052' '#1042#1047
  ClientHeight = 400
  ClientWidth = 424
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label2: TLabel
    Left = 8
    Top = 132
    Width = 149
    Height = 13
    Caption = #1055#1086#1088#1090' '#1074#1110#1076#1076#1072#1083#1077#1085#1086#1075#1086' '#1082#1086#1085#1090#1088#1086#1083#1102':'
  end
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 92
    Height = 13
    Caption = 'ID '#1088#1086#1073#1086#1095#1086#1111' '#1089#1090#1072#1085#1094#1110#1111':'
  end
  object Label3: TLabel
    Left = 8
    Top = 51
    Width = 60
    Height = 13
    Caption = #1030#1085#1076#1077#1082#1089' '#1042#1055#1047':'
  end
  object Label4: TLabel
    Left = 8
    Top = 91
    Width = 95
    Height = 13
    Caption = 'ID '#1088#1077#1075#1110#1086#1085#1091' ('#1075#1088#1091#1087#1080'):'
  end
  object AddAutoStart: TCheckBox
    Left = 8
    Top = 288
    Width = 201
    Height = 17
    Caption = #1057#1090#1074#1086#1088#1080#1090#1080' '#1079#1072#1074#1076#1072#1085#1085#1103' '#1091' '#1072#1074#1090#1086#1079#1072#1087#1091#1089#1082#1091
    TabOrder = 0
  end
  object AddFirewallRule: TCheckBox
    Left = 8
    Top = 208
    Width = 241
    Height = 17
    Caption = #1044#1086#1076#1072#1090#1080' '#1074#1080#1082#1083#1102#1095#1077#1085#1085#1103' '#1091' '#1073#1088#1072#1085#1076#1084#1072#1091#1077#1088' Windows'
    TabOrder = 1
  end
  object UseGuardian: TCheckBox
    Left = 8
    Top = 168
    Width = 321
    Height = 17
    Caption = #1042#1080#1082#1086#1088#1080#1089#1090#1086#1074#1091#1074#1072#1090#1080' Guardian '#1076#1083#1103' '#1082#1086#1085#1090#1088#1086#1083#1102' '#1088#1086#1073#1086#1090#1080' '#1055#1088#1086#1075#1088#1072#1084#1080
    TabOrder = 2
  end
  object HideWindow: TCheckBox
    Left = 8
    Top = 248
    Width = 249
    Height = 17
    Caption = #1055#1088#1080#1093#1086#1074#1091#1074#1072#1090#1080' '#1074#1110#1082#1085#1086' '#1055#1088#1086#1075#1088#1072#1084#1080' '#1087#1110#1076' '#1095#1072#1089' '#1079#1072#1087#1091#1089#1082#1091
    TabOrder = 3
  end
  object RemAdmPort: TEdit
    Left = 163
    Top = 129
    Width = 57
    Height = 21
    TabOrder = 4
    Text = '7894'
  end
  object OpenCfgAfterExit: TCheckBox
    Left = 8
    Top = 328
    Width = 393
    Height = 17
    Caption = 
      #1042#1110#1076#1082#1088#1080#1090#1080' '#1082#1086#1085#1092#1110#1075#1091#1088#1072#1094#1110#1081#1085#1080#1081' '#1092#1072#1081#1083' main.cfg '#1076#1083#1103' '#1076#1077#1090#1072#1083#1100#1085#1086#1075#1086' '#1085#1072#1083#1072#1096#1090#1091#1074#1072#1085 +
      #1085#1103
    TabOrder = 5
  end
  object Save: TButton
    Left = 174
    Top = 369
    Width = 75
    Height = 25
    Caption = #1047#1072#1074#1077#1088#1096#1080#1090#1080
    TabOrder = 6
    OnClick = SaveClick
  end
  object StationID: TEdit
    Left = 120
    Top = 5
    Width = 121
    Height = 21
    TabOrder = 7
    Text = 'DEMO'
  end
  object IndexVZ: TEdit
    Left = 120
    Top = 48
    Width = 121
    Height = 21
    TabOrder = 8
    Text = '00000'
  end
  object RegionID: TEdit
    Left = 120
    Top = 88
    Width = 121
    Height = 21
    TabOrder = 9
    Text = 'TEST'
  end
  object AutoStartForAll: TCheckBox
    Left = 215
    Top = 288
    Width = 146
    Height = 17
    Caption = #1044#1083#1103' '#1074#1089#1110#1093' '#1082#1086#1088#1080#1089#1090#1091#1074#1072#1095#1110#1074
    TabOrder = 10
  end
end
