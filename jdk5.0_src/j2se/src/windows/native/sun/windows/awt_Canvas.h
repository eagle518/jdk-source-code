/*
 * @(#)awt_Canvas.h	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_CANVAS_H
#define AWT_CANVAS_H

#include "awt_Component.h"
#include "sun_awt_windows_WCanvasPeer.h"


/************************************************************************
 * AwtCanvas class
 */

class AwtCanvas : public AwtComponent {
public:
    AwtCanvas();
    virtual ~AwtCanvas();

    virtual LPCTSTR GetClassName();
    static AwtCanvas* Create(jobject self, jobject hParent);

    virtual MsgRouting WmEraseBkgnd(HDC hDC, BOOL& didErase);
    virtual MsgRouting WmPaint(HDC hDC);

    virtual MsgRouting HandleEvent(MSG *msg, BOOL synthetic);
};

#endif /* AWT_CANVAS_H */
