// PEELFexplorer.h : main header file for the PEELFEXPLORER application
//

#if !defined(AFX_PEELFEXPLORER_H__352268E6_1C74_4682_B5A1_4102FCFF3CD8__INCLUDED_)
#define AFX_PEELFEXPLORER_H__352268E6_1C74_4682_B5A1_4102FCFF3CD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPEELFexplorerApp:
// See PEELFexplorer.cpp for the implementation of this class
//

class CPEELFexplorerApp : public CWinApp
{
public:
	CPEELFexplorerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPEELFexplorerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPEELFexplorerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEELFEXPLORER_H__352268E6_1C74_4682_B5A1_4102FCFF3CD8__INCLUDED_)
