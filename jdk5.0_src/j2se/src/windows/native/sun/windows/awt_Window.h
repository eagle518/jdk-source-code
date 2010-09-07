/*
 * @(#)awt_Window.h	1.57 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_WINDOW_H
#define AWT_WINDOW_H

#include "awt_Canvas.h"

#include "java_awt_Window.h"
#include "sun_awt_windows_WWindowPeer.h"

class AwtFrame;

/************************************************************************
 * AwtWindow class
 */

class AwtWindow : public AwtCanvas {
public:

    /* java.awt.Window field ids */
    static jfieldID warningStringID;
    static jfieldID locationByPlatformID;
    static jfieldID screenID; /* screen number passed over from WindowPeer */

    /* sun.awt.windows.WWindowPeer field ids */
    static jfieldID focusableWindowID;

    AwtWindow();
    virtual ~AwtWindow();

    virtual LPCTSTR GetClassName();
    virtual void FillClassInfo(WNDCLASS *lpwc);

    static AwtWindow* Create(jobject self, jobject parent);

    /* Update the insets for this Window (container), its peer &
     * optional other
     */
    BOOL UpdateInsets(jobject insets = 0);

    INLINE AwtFrame * GetOwningFrameOrDialog() { return m_owningFrameDialog; }

    HWND GetTopLevelHWnd();
    
    /* Subtract inset values from a window origin. */
    INLINE void SubtractInsetPoint(int& x, int& y) {
	x -= m_insets.left;
	y -= m_insets.top;
    }

    virtual void GetInsets(RECT* rect) {
        VERIFY(::CopyRect(rect, &m_insets));
    }

    /* to make embedded frames easier */
    virtual BOOL IsEmbeddedFrame() { return FALSE;}

    /* We can hold children */
    virtual BOOL IsContainer() { return TRUE;}

    virtual BOOL IsUndecorated() { return TRUE; }

    virtual void Invalidate(RECT* r);
    virtual void Reshape(int x, int y, int width, int height);
    virtual void Show();
    virtual void SetResizable(BOOL isResizable);
    BOOL IsResizable();
    virtual void RecalcNonClient();
    virtual void RedrawNonClient();
    virtual int  GetScreenImOn();
    virtual void CheckIfOnNewScreen();

    static void ToBack(void *data);

    /* Post events to the EventQueue */
    void SendComponentEvent(jint eventId);
    void SendWindowEvent(jint id, HWND opposite = NULL,
			 jint oldState = 0, jint newState = 0);

    BOOL IsFocusableWindow();
    /*
     * Windows message handler functions
     */
    virtual MsgRouting WmActivate(UINT nState, BOOL fMinimized, HWND opposite);
    static void BounceActivation(void *self); // used by WmActivate
    virtual MsgRouting WmCreate();
    virtual MsgRouting WmClose();
    virtual MsgRouting WmDestroy();
    virtual MsgRouting WmDDEnterFullScreen(HMONITOR monitor);
    virtual MsgRouting WmDDExitFullScreen(HMONITOR monitor);
    virtual MsgRouting WmMove(int x, int y);
    virtual MsgRouting WmSize(UINT type, int w, int h);
    virtual MsgRouting WmPaint(HDC hDC);
    virtual MsgRouting WmSysCommand(UINT uCmdType, int xPos, int yPos);
    virtual MsgRouting WmExitSizeMove();
    virtual MsgRouting WmSettingChange(UINT wFlag, LPCTSTR pszSection);
    virtual MsgRouting WmNcCalcSize(BOOL fCalcValidRects,
				    LPNCCALCSIZE_PARAMS lpncsp, LRESULT& retVal);
    virtual MsgRouting WmNcPaint(HRGN hrgn);
    // virtual MsgRouting WmNcHitTest(UINT x, UINT y, LRESULT& retVal);

    virtual MsgRouting HandleEvent(MSG *msg, BOOL synthetic);
    virtual void WindowResized();
    BOOL IsExactlyWindow() {
        return _tcscmp(GetClassName(), AwtWindow::GetClassName()) == 0;
    }

    void moveToDefaultLocation(); /* moves Window to X,Y specified by Window Manger */

    INLINE virtual BOOL IsTopLevel() { return TRUE; }

private:
    RECT m_insets;          /* a cache of the insets being used */
    RECT m_old_insets;      /* help determine if insets change */
    BOOL m_resizing;        /* in the middle of a resizing operation */
    POINT m_sizePt;         /* the last value of WM_SIZE */
    RECT m_warningRect;     /* The window's warning banner area, if any. */
    AwtFrame *m_owningFrameDialog; /* The nearest Frame/Dialog which owns us */

    int m_screenNum;

    void InitOwner(AwtWindow *owner);
};

#endif /* AWT_WINDOW_H */
