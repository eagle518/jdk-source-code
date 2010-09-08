/*
 * @(#)awt_Frame.h	1.80 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_FRAME_H
#define AWT_FRAME_H

#include "awt_Window.h"
#include "awt_MenuBar.h" //add for multifont
#include "awt_Toolkit.h"
#include "Hashtable.h"

#include "java_awt_Frame.h"
#include "sun_awt_windows_WFramePeer.h"


/************************************************************************
 * AwtFrame class
 */

#define AWT_FRAME_WINDOW_CLASS_NAME TEXT("SunAwtFrame")


class AwtFrame : public AwtWindow {
public:
    enum FrameExecIds {
	FRAME_SETMENUBAR
    };

    /* int handle field for sun.awt.windows.WEmbeddedFrame */
    static jfieldID handleID;

    /* int state field for java.awt.Frame */
    static jfieldID stateID;

    /* boolean undecorated field for java.awt.Frame */
    static jfieldID undecoratedID;

    /* method id for WEmbeddedFrame.requestActivate() method */
    static jmethodID activateEmbeddingTopLevelMID;

    AwtFrame();
    virtual ~AwtFrame();

    virtual LPCTSTR GetClassName();
    
    /* Create a new AwtFrame.  This must be run on the main thread. */
    static AwtFrame* Create(jobject self, jobject parent);

    /* Returns whether this frame is embedded in an external native frame. */
    INLINE BOOL IsEmbeddedFrame() { return m_isEmbedded; }

    INLINE BOOL IsSimpleWindow() { return FALSE; }

    /* Confirms that EmbeddedFrame is currently requesting activation */
    INLINE BOOL IsEmbeddedFrameActivationRequest() { return m_isEmbeddedFrameActivationRequest; }

    /* Returns whether this window is in iconified state. */
    INLINE BOOL isIconic() { return m_iconic; }
    INLINE void setIconic(BOOL b) { m_iconic = b; }

    /* Returns whether this window is in zoomed state. */
    INLINE BOOL isZoomed() { return m_zoomed; }
    INLINE void setZoomed(BOOL b) { m_zoomed = b; }

    void SendWindowStateEvent(int oldState, int newState);

    void Show();

    INLINE void DrawMenuBar() { VERIFY(::DrawMenuBar(GetHWnd())); }

    virtual void DoUpdateIcon();
    virtual HICON GetEffectiveIcon(int iconType);
    
    /*for WmDrawItem and WmMeasureItem method */
    AwtMenuBar* GetMenuBar();
    void SetMenuBar(AwtMenuBar*);

    MsgRouting WmGetMinMaxInfo(LPMINMAXINFO lpmmi);
    MsgRouting WmSize(UINT type, int w, int h);
    MsgRouting WmActivate(UINT nState, BOOL fMinimized, HWND opposite);
    MsgRouting WmDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting WmMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT& measureInfo);
    MsgRouting WmEnterMenuLoop(BOOL isTrackPopupMenu);
    MsgRouting WmExitMenuLoop(BOOL isTrackPopupMenu);
    MsgRouting WmMouseUp(UINT flags, int x, int y, int button);
    MsgRouting WmMouseMove(UINT flags, int x, int y);
    MsgRouting WmNcMouseDown(WPARAM hitTest, int x, int y, int button);
    MsgRouting WmGetIcon(WPARAM iconType, LRESULT& retVal);
    MsgRouting WmShowWindow(BOOL show, UINT status);

    virtual MsgRouting WmSysCommand(UINT uCmdType, int xPos, int yPos);

    LRESULT WinThreadExecProc(ExecuteArgs * args);

    INLINE BOOL IsUndecorated() { return m_isUndecorated; }

    INLINE HWND GetProxyFocusOwner() {
	DASSERT(AwtToolkit::MainThread() == ::GetCurrentThreadId());
	if (m_proxyFocusOwner == NULL) {
	    CreateProxyFocusOwner();
	}
	return m_proxyFocusOwner;
    }

    INLINE HWND GetLastProxiedFocusOwner() { return m_lastProxiedFocusOwner; }
    INLINE void SetLastProxiedFocusOwner(HWND hwnd) { m_lastProxiedFocusOwner = hwnd; }

    void SetMaximizedBounds(int x, int y, int w, int h);
    void ClearMaximizedBounds();

    // returns true if the frame is inputmethod window 
    INLINE BOOL isInputMethodWindow() { return m_isInputMethodWindow; }
    // adjusts the IME candidate window position if needed
    void AdjustCandidateWindowPos();

    void SynthesizeWmActivate(BOOL doActivate, HWND opposite);

    void ActivateFrameOnMouseDown();
    BOOL ActivateEmbeddedFrameOnSetFocus(HWND hWndLostFocus);
    void DeactivateEmbeddedFrameOnKillFocus(HWND hWndGotFocus);
    BOOL ActivateEmbeddedFrameHelper(HWND oppositeToplevelHWnd);

    // invoked on Toolkit thread
    static jobject _GetBoundsPrivate(void *param);

    // some methods called on Toolkit thread
    static void _SetState(void *param);
    static jint _GetState(void *param);
    static void _SetMaximizedBounds(void *param);
    static void _ClearMaximizedBounds(void *param);
    static void _SetMenuBar(void *param);
    static void _SetIMMOption(void *param);
    static void _SynthesizeWmActivate(void *param);
    static void _NotifyModalBlocked(void *param);

protected:
    /* The frame is undecorated. */
    BOOL m_isUndecorated;

private:
    static LRESULT CALLBACK ProxyWindowProc(HWND hwnd, UINT message,
					    WPARAM wParam, LPARAM lParam);
    void CreateProxyFocusOwner();
    void DestroyProxyFocusOwner();

    /* The frame's embedding parent (if any) */
    HWND m_parentWnd;

    /* The frame's menubar. */
    AwtMenuBar* menuBar;

    /* The frame is an EmbeddedFrame. */
    BOOL m_isEmbedded;

    /* EmbeddedFrame is to be activated on demand only */
    BOOL m_isEmbeddedFrameActivationRequest;

    /* used so that calls to ::MoveWindow in SetMenuBar don't propogate
       because they are immediately followed by calls to Component.resize */
    BOOL m_ignoreWmSize;

    /* tracks whether or not menu on this frame is dropped down */
    BOOL m_isMenuDropped;

    /* The frame is an InputMethodWindow */
    BOOL m_isInputMethodWindow;

    /* Receives all keyboard input when an AwtWindow which is not an AwtFrame
       or an AwtDialog (or one of its children) has the logical input focus. */
    HWND m_proxyFocusOwner;

    /* Retains the last/current sm_focusOwner proxied. Actually, it should be
     * a component of an owned window last/currently active. */
    HWND m_lastProxiedFocusOwner;

    /* 
     * Fix for 4823903.
     * Retains a focus proxied window to set the focus correctly
     * when its owner get activated.
     */
    AwtWindow *m_actualFocusedWindow;
    
    /* The original, default WndProc for m_proxyFocusOwner. */
    WNDPROC m_proxyDefWindowProc;

    BOOL m_iconic;          /* are we in an iconic state */
    BOOL m_zoomed;          /* are we in a zoomed state */

    BOOL  m_maxBoundsSet;
    POINT m_maxPos;
    POINT m_maxSize;

    BOOL isInManualMoveOrSize;
    WPARAM grabbedHitTest;
    POINT savedMousePos;

    /*
     * Hashtable<Thread, BlockedThreadStruct> - a table that contains all the
     * information about non-toolkit threads with modal blocked embedded
     * frames. This information includes: number of blocked embedded frames
     * created on the the thread, and mouse and modal hooks installed for
     * that thread. For every thread each hook is installed only once
     */
    static Hashtable sm_BlockedThreads;
};

#endif /* AWT_FRAME_H */
