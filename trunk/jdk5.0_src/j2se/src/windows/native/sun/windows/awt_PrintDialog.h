/*
 * @(#)awt_PrintDialog.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    static BOOL PrintDlg(LPPRINTDLG);
};

#endif
