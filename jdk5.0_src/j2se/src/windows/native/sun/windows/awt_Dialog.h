/*
 * @(#)awt_Dialog.h	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_DIALOG_H
#define AWT_DIALOG_H

#include "awt_Frame.h"

#include "java_awt_Dialog.h"
#include "sun_awt_windows_WDialogPeer.h"


/************************************************************************
 * AwtDialog class
 */

class AwtDialog : public AwtFrame {
public:

    /* java.awt.Dialog field ids */
    static jfieldID titleID;

    /* boolean undecorated field for java.awt.Dialog */
    static jfieldID undecoratedID;

    AwtDialog();
    virtual ~AwtDialog();

    virtual LPCTSTR GetClassName();
    virtual void  FillClassInfo(WNDCLASS *lpwc);
    virtual void SetResizable(BOOL isResizable);

    void Show();
    void ShowModal();
    void EndModal();
    static BOOL CALLBACK CountHWnds(HWND hWnd, AwtDialog* obj);
    static BOOL CALLBACK DisableHWnds(HWND hWnd, AwtDialog* obj);

    /* Create a new AwtDialog.  This must be run on the main thread. */
    static AwtDialog* Create(jobject peer, jobject hParent);
    virtual MsgRouting WmShowModal();
    virtual MsgRouting WmEndModal();
    virtual MsgRouting WmStyleChanged(int wStyleType, LPSTYLESTRUCT lpss);
    virtual MsgRouting WmSize(UINT type, int w, int h);
    MsgRouting WmNcMouseDown(WPARAM hitTest, int x, int y, int button);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    // utility functions for application-modal dialogs
    static void ModalDisable(HWND hwndDlg);
    static void ModalEnable(HWND hwndDlg);
    static void ModalNextWindowToFront(HWND hwndDlg);
    static void SetDisabledLevelToGreatest(AwtFrame *frame);

private:
    static int GetDisabledLevel(HWND hwnd);
    static void SetDisabledLevel(HWND hwnd, int disabledLevel);
    static BOOL IsInitiallyDisabled(HWND hwnd);
    static void SetInitiallyDisabled(HWND hwnd);
    static void IncrementDisabledLevel(HWND hwnd, int increment);
    static void ResetDisabledLevel(HWND hwnd);
    static BOOL CALLBACK DisableTopLevelsCallback(HWND hWnd, LPARAM lParam);
    static BOOL CALLBACK EnableTopLevelsCallback(HWND hWnd, LPARAM lParam);
    static BOOL CALLBACK NextWindowToFrontCallback(HWND hWnd, LPARAM lParam);
    static BOOL CALLBACK FindGreatestDisabledLevelCallback(HWND hWnd,
                                                           LPARAM lParam);
    void UpdateSystemMenu();
    void UpdateDialogIcon();

    HWND  m_modalWnd;
};

#endif /* AWT_DIALOG_H */



