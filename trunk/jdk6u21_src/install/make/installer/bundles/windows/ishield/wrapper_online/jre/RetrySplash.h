/*
 * %W% %E%
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// RetrySplash.h : Declaration of the CRetrySplashScreen
//
// By Paul Klingaman
//

#ifndef __RETRY_SPLASH_H_
#define __RETRY_SPLASH_H_

#include "resource.h"

class CRetrySplashScreen :
    public CDialogImpl<CRetrySplashScreen>
{
public:
    
    CRetrySplashScreen();
    ~CRetrySplashScreen();
    
    enum { IDD = IDD_RETRY_SPLASH };
    
    BEGIN_MSG_MAP(CRetrySplashScreen)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
    END_MSG_MAP()
    
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   
private:
    HFONT CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
	void  FreeGDIResources ();
	HDC	    m_hMemDC;
    HBITMAP m_hBitmap;
	BITMAP	m_bmBannerJFX;
	HFONT   m_hDialogFont;
    HFONT   m_hDialogHeaderFont;
    
};

#endif //__RETRY_SPLASH_H_

