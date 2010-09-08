/*
 * @(#)awt_Checkbox.h	1.34 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_CHECKBOX_H
#define AWT_CHECKBOX_H

#include "awt_Component.h"

#include "java_awt_Checkbox.h"
#include "sun_awt_windows_WCheckboxPeer.h"


/************************************************************************
 * Component class for system provided Checkboxes
 */

class AwtCheckbox : public AwtComponent {
public:

    /* check size in Windows is always the same */
    static const int CHECK_SIZE;

    /* java.awt.Checkbox field ids */
    static jfieldID labelID;
    static jfieldID groupID;
    static jfieldID stateID;

    AwtCheckbox();

    virtual LPCTSTR GetClassName();

    /* Create a new AwtCheckbox object and window.       */
    static AwtCheckbox* Create(jobject self, jobject hParent);

    /* get state of multifont checkbox */
    BOOL GetState();

    /* get check mark size */
    static int GetCheckSize();

    /*  Windows message handler functions */
    MsgRouting WmMouseUp(UINT flags, int x, int y, int button);
    MsgRouting WmMouseDown(UINT flags, int x, int y, int button);
    MsgRouting WmNotify(UINT notifyCode);
    MsgRouting OwnerDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting WmPaint(HDC hDC);

    MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    BOOL ActMouseMessage(MSG* pMsg);

    // called on Toolkit thread from JNI
    static void _SetLabel(void *param);
    static void _SetCheckboxGroup(void *param);
    static void _SetState(void *param);

#ifdef DEBUG
    virtual void VerifyState(); /* verify checkbox and peer are in sync. */
#endif

private:
    /* for state of LButtonDown */
    BOOL m_fLButtonDowned;
};

#endif /* AWT_CHECKBOX_H */
