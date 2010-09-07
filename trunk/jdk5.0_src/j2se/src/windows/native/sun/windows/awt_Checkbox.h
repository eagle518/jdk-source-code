/*
 * @(#)awt_Checkbox.h	1.30 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    MsgRouting WmNotify(UINT notifyCode);
    MsgRouting OwnerDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting WmPaint(HDC hDC);

    MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    BOOL ActMouseMessage(MSG* pMsg);

#ifdef DEBUG
    virtual void VerifyState(); /* verify checkbox and peer are in sync. */
#endif

private:
    /* for state of LButtonDown */
    BOOL m_fLButtonDowned;
};

#endif /* AWT_CHECKBOX_H */
