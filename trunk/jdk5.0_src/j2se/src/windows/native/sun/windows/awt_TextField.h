/*
 * @(#)awt_TextField.h	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_TEXTFIELD_H
#define AWT_TEXTFIELD_H

#include "awt_TextComponent.h"

#include "java_awt_TextField.h"
#include "sun_awt_windows_WTextFieldPeer.h"

#include <ole2.h>
#include <richedit.h>
#include <richole.h>

/************************************************************************
 * AwtTextField class
 */

class AwtTextField : public AwtTextComponent {
public:
    AwtTextField();

    static AwtTextField* Create(jobject self, jobject parent);

    /*
     *  Windows message handler functions
     */
    MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

  protected:
    void EditSetSel(CHARRANGE &cr);
    void EditGetSel(CHARRANGE &cr);
    LONG EditGetCharFromPos(POINT& pt);
};

#endif /* AWT_TEXTFIELD_H */
