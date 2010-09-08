/*
 * @(#)awt_ScrollPane.h	1.34 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_SCROLLPANE_H
#define AWT_SCROLLPANE_H

#include "awt_Canvas.h"

#include "java_awt_ScrollPane.h"
#include "java_awt_Insets.h"
#include "sun_awt_windows_WScrollPanePeer.h"


/************************************************************************
 * AwtScrollPane class
 */

class AwtScrollPane : public AwtCanvas {
public:

    /* java.awt.ScrollPane fields */
    static jfieldID scrollbarDisplayPolicyID;
    static jfieldID hAdjustableID;
    static jfieldID vAdjustableID;

    /* java.awt.ScrollPaneAdjustable fields */
    static jfieldID unitIncrementID;
    static jfieldID blockIncrementID;

    /* sun.awt.windows.WScrollPanePeer methods */
    static jmethodID postScrollEventID;

    AwtScrollPane();

    virtual LPCTSTR GetClassName();
    
    static AwtScrollPane* Create(jobject self, jobject hParent);

    void SetInsets(JNIEnv *env);
    void RecalcSizes(int parentWidth, int parentHeight, 
		     int childWidth, int childHeight);
    virtual void Show(JNIEnv *env);
    virtual void Reshape(int x, int y, int w, int h);
    virtual void BeginValidate() {}
    virtual void EndValidate() {}
    BOOL ActMouseMessage(MSG* pMsg);

    /*
     * Fix for bug 4046446
     * Returns scroll position for the appropriate scrollbar.
     */
    int GetScrollPos(int orient);
  
    /*
     * Windows message handler functions
     */
    virtual MsgRouting WmNcHitTest(UINT x, UINT y, LRESULT& retVal);
    virtual MsgRouting WmHScroll(UINT scrollCode, UINT pos, HWND hScrollBar);
    virtual MsgRouting WmVScroll(UINT scrollCode, UINT pos, HWND hScrollBar);

    virtual MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    // some methods invoked on Toolkit thread
    static jint _GetOffset(void *param);
    static void _SetInsets(void *param);
    static void _SetScrollPos(void *param);
    static void _SetSpans(void *param);

#ifdef DEBUG
    virtual void VerifyState(); /* verify target and peer are in sync. */
#endif

private:
    void PostScrollEvent(int orient, int scrollCode, int pos);
    void SetScrollInfo(int orient, int max, int page, BOOL disableNoScroll);
};

#endif /* AWT_SCROLLPANE_H */
