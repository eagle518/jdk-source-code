/*
 * @(#)awt_PrintDialog.h	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_PRINT_DIALOG_H_
#define _AWT_PRINT_DIALOG_H_

#include "stdhdrs.h"
#include <commdlg.h>

/************************************************************************
 * AwtPrintDialog class
 */

class AwtPrintDialog {
public:
    static jfieldID AwtPrintDialog::controlID;
    static jfieldID AwtPrintDialog::parentID;
    static jfieldID AwtPrintDialog::pageID;
    static jmethodID AwtPrintDialog::setHWndMID;

    static BOOL PrintDlg(LPPRINTDLG);

    // called on Toolkit thread
    static jboolean _Show(void *param);

    static void _ToFront(void *param);
    static void _ToBack(void *param);
};

#endif
