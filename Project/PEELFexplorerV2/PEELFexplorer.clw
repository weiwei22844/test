; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPEELFexplorerDlg
LastTemplate=CListCtrl
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PEELFexplorer.h"

ClassCount=4
Class1=CPEELFexplorerApp
Class2=CPEELFexplorerDlg
Class3=CAboutDlg

ResourceCount=4
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_PEELFEXPLORER_DIALOG
Class4=CReportCtrl
Resource4=IDR_MAINMENU

[CLS:CPEELFexplorerApp]
Type=0
HeaderFile=PEELFexplorer.h
ImplementationFile=PEELFexplorer.cpp
Filter=N

[CLS:CPEELFexplorerDlg]
Type=0
HeaderFile=PEELFexplorerDlg.h
ImplementationFile=PEELFexplorerDlg.cpp
Filter=W
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CPEELFexplorerDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=PEELFexplorerDlg.h
ImplementationFile=PEELFexplorerDlg.cpp
Filter=D
LastObject=IDOK

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PEELFEXPLORER_DIALOG]
Type=1
Class=CPEELFexplorerDlg
ControlCount=2
Control1=IDC_TREE_FILEINFO,SysTreeView32,1350631463
Control2=IDC_LIST_FileReport,SysListView32,1350631425

[MNU:IDR_MAINMENU]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_CLOSE
Command3=ID_FILE_EXIT
Command4=ID_ABOUT
CommandCount=4

[CLS:CReportCtrl]
Type=0
HeaderFile=ReportCtrl.h
ImplementationFile=ReportCtrl.cpp
BaseClass=CListCtrl
Filter=W
LastObject=CReportCtrl

