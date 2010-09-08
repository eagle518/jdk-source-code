/*
 * @(#)awt_Label.h	1.27 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_LABEL_H
#define AWT_LABEL_H

#include "awt_Component.h"

#include "java_awt_Label.h"
#include "sun_awt_windows_WLabelPeer.h"


/************************************************************************
 * AwtLabel class
 */

class AwtLabel : public AwtComponent {
public:
    /*
     * java.awt.Label fields
     */
    static jfieldID textID;
    static jfieldID alignmentID;

    AwtLabel();

    virtual LPCTSTR GetClassName();
    
    static AwtLabel* Create(jobject label, jobject parent);

    /*
     * Windows message handler functions
     */
    virtual MsgRouting WmPaint(HDC hDC);
    virtual MsgRouting WmPrintClient(HDC hDC, LPARAM flags);
    virtual MsgRouting WmEraseBkgnd(HDC hDC, BOOL& didErase);

    /*
     * if WM_PAINT was recieving when we can not paint
     * then setup m_needPaint end when can call this function
     */
    void LazyPaint();
     /*
      * Enable/disable component
      */
    virtual void Enable(BOOL bEnable);

    // some methods called on Toolkit thread
    static void _SetText(void *param);
    static void _SetAlignment(void *param);
    static void _LazyPaint(void *param);

private:
    BOOL m_needPaint; // flags for lazy paint of Label

    void DoPaint(HDC hDC, RECT& r);
};

#endif /* AWT_LABEL_H */
