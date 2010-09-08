/*
 * @(#)awt_Choice.h	1.46 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_CHOICE_H
#define AWT_CHOICE_H

#include "awt_Component.h"

#include "java_awt_Choice.h"
#include "sun_awt_windows_WChoicePeer.h"


/************************************************************************
 * Component class for system provided buttons
 */

class AwtChoice : public AwtComponent {
public:
    AwtChoice();

    virtual LPCTSTR GetClassName();
    static AwtChoice* Create(jobject peer, jobject hParent);

    virtual void Reshape(int x, int y, int w, int h);
    void ResetDropDownHeight();
    int GetDropDownHeight();

#ifdef DEBUG
    void VerifyState(); /* verify component and peer are in sync. */
#endif

    /*for multifont list */
    jobject PreferredItemSize(JNIEnv *env);

    /*
     * Windows message handler functions
     */
    MsgRouting WmNotify(UINT notifyCode);

    /* for multifont choice */
    MsgRouting OwnerDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting OwnerMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT& measureInfo);

    /* Workaround for bug #4338368 */
    MsgRouting WmKillFocus(HWND hWndGotFocus);
    MsgRouting WmMouseUp(UINT flags, int x, int y, int button);

    MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    INLINE HWND GetDBCSEditHandle() { return GetHWnd(); }
    virtual void SetFont(AwtFont *pFont);
    virtual BOOL InheritsNativeMouseWheelBehavior();
    virtual void SetDragCapture(UINT flags);
    virtual void ReleaseDragCapture(UINT flags);

    BOOL ActMouseMessage(MSG * pMsg);
    INLINE BOOL AwtChoice::IsChoiceOpened() {return SendMessage(CB_GETDROPPEDSTATE, 0, 0);}

    static BOOL mouseCapture;
    static BOOL skipNextMouseUp;

    // called on Toolkit thread from JNI
    static void _Reshape(void *param);
    static void _Select(void *param);
    static void _AddItems(void *param);
    static void _Remove(void *param);
    static void _RemoveAll(void *param);

private:
    int GetFieldHeight();
    int GetTotalHeight();
    MsgRouting killFocusRouting;
};

#endif /* AWT_CHOICE_H */
