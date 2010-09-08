/*
 * @(#)awt_TextField.h	1.25 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    // invoked on Toolkit thread
    static void _SetEchoChar(void *param);

  protected:
    LONG EditGetCharFromPos(POINT& pt);
    virtual void Reshape(int x, int y, int w, int h);

private:
    void EditSetSel(CHARRANGE &cr);
    void initialRescroll();

    bool m_initialRescrollFlag;
};

#endif /* AWT_TEXTFIELD_H */
