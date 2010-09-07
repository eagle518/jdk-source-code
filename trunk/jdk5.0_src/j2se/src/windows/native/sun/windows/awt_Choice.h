/*
 * @(#)awt_Choice.h	1.41 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    static BOOL mouseCapture;
    static BOOL consumeNextMouseUp;

private:
    int GetFieldHeight();
    int GetTotalHeight();
};

#endif /* AWT_CHOICE_H */
