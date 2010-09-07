/*
 * @(#)awt_PrintDialog.cpp	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_PrintDialog.h"
#include "awt_Dialog.h"
#include "awt_PrintControl.h"
#include "awt_dlls.h"
#include <sun_awt_windows_WPrintDialog.h>
#include <sun_awt_windows_WPrintDialogPeer.h>

jfieldID AwtPrintDialog::controlID;
jfieldID AwtPrintDialog::parentID;


BOOL
AwtPrintDialog::PrintDlg(LPPRINTDLG data) {
    AwtCommDialog::load_comdlg_procs();
    return static_cast<BOOL>(reinterpret_cast<INT_PTR>(
        AwtToolkit::GetInstance().InvokeFunction(
            reinterpret_cast<void *(*)(void *)>(AwtCommDialog::PrintDlgWrapper),
            data)));
}

static UINT_PTR CALLBACK
PrintDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uiMsg) {
        case WM_INITDIALOG: {
            // fix for 4632159 - disable CS_SAVEBITS
            DWORD style = ::GetClassLong(hdlg,GCL_STYLE);
            ::SetClassLong(hdlg,GCL_STYLE,style & ~CS_SAVEBITS);
            ::SetFocus(hdlg);
            break;
        }
    }
    return FALSE;
}

extern "C" {
JNIEXPORT void JNICALL
Java_sun_awt_windows_WPrintDialog_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtPrintDialog::controlID =
        env->GetFieldID(cls, "pjob", "Ljava/awt/print/PrinterJob;");
    
    DASSERT(AwtPrintDialog::controlID != NULL);
   
    AwtPrintControl::initIDs(env, cls);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WPrintDialog_setPeer(JNIEnv *env, jobject target,
					  jobject peer)
{
    TRY;

    env->SetObjectField(target, AwtComponent::peerID, peer);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WPrintDialogPeer_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtPrintDialog::parentID =
        env->GetFieldID(cls, "parent", "Lsun/awt/windows/WComponentPeer;");

    DASSERT(AwtPrintDialog::parentID != NULL);

    CATCH_BAD_ALLOC;
}

JNIEXPORT jboolean JNICALL
Java_sun_awt_windows_WPrintDialogPeer__1show(JNIEnv *env, jobject peer)
{
    TRY;

    DASSERT(peer != NULL);
    jobject target = env->GetObjectField(peer, AwtObject::targetID);
    DASSERT(target != NULL);
    jobject parent = env->GetObjectField(peer, AwtPrintDialog::parentID);
    DASSERT(parent != NULL);
    jobject control = env->GetObjectField(target, AwtPrintDialog::controlID);
    DASSERT(control != NULL);

    AwtComponent *awtParent = (AwtComponent *)JNI_GET_PDATA(parent);
    DASSERT(awtParent != NULL);
    
    PRINTDLG pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    if (!AwtPrintControl::InitPrintDialog(env, control, pd)) {
      return JNI_FALSE;
    }
    pd.lpfnPrintHook = (LPPRINTHOOKPROC)PrintDialogHookProc;
    pd.lpfnSetupHook = (LPSETUPHOOKPROC)PrintDialogHookProc;
    pd.Flags |= PD_ENABLESETUPHOOK | PD_ENABLEPRINTHOOK;    
    AwtDialog::ModalDisable(NULL);
    BOOL ret = AwtPrintDialog::PrintDlg(&pd);
    AwtDialog::ModalEnable(NULL);
    AwtDialog::ModalNextWindowToFront(awtParent->GetHWnd());
    if (ret) {
        AwtPrintControl::UpdateAttributes(env, control, pd);
	return JNI_TRUE;
    }

    return JNI_FALSE;

    CATCH_BAD_ALLOC_RET(0);
}
    
} /* extern "C" */

