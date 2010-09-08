/*
 * @(#)ComCtl32Util.cpp	1.3 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "ComCtl32Util.h"

ComCtl32Util ComCtl32Util::m_Instance;

void ComCtl32Util::InitFields() {
    m_bNewSubclassing = FALSE;
    m_lpfnSetWindowSubclass = NULL;
    m_lpfnRemoveWindowSubclass = NULL;
    m_lpfnDefSubclassProc = NULL;

    m_bToolTipControlInitialized = FALSE;
    fn_InitCommonControlsEx = NULL;
}

ComCtl32Util::ComCtl32Util() {
    hModComCtl32 = NULL;
    InitFields();
}

ComCtl32Util::~ComCtl32Util() {
    DASSERT(hModComCtl32 == NULL);
}

void ComCtl32Util::InitLibraries() {
    if (hModComCtl32 == NULL) {
        hModComCtl32 = ::LoadLibrary(TEXT("comctl32.dll"));
        if (hModComCtl32 != NULL) {
            m_lpfnSetWindowSubclass = (PFNSETWINDOWSUBCLASS)::GetProcAddress(hModComCtl32, "SetWindowSubclass");
            m_lpfnRemoveWindowSubclass = (PFNREMOVEWINDOWSUBCLASS)::GetProcAddress(hModComCtl32, "RemoveWindowSubclass");
            m_lpfnDefSubclassProc = (PFNDEFSUBCLASSPROC)::GetProcAddress(hModComCtl32, "DefSubclassProc");

            m_bNewSubclassing = (m_lpfnSetWindowSubclass != NULL) &&
                                (m_lpfnRemoveWindowSubclass != NULL) &&
                                (m_lpfnDefSubclassProc != NULL);

            fn_InitCommonControlsEx = (ComCtl32Util::InitCommonControlsExType)::GetProcAddress(hModComCtl32, "InitCommonControlsEx");
            InitCommonControls();
        }
    }
}

void ComCtl32Util::FreeLibraries() {
    if (hModComCtl32 != NULL) {
        InitFields();
        ::FreeLibrary(hModComCtl32);
        hModComCtl32 = NULL;
    }
}

WNDPROC ComCtl32Util::SubclassHWND(HWND hwnd, WNDPROC _WindowProc) {
    if (m_bNewSubclassing) {
        DASSERT(hModComCtl32 != NULL);
        const SUBCLASSPROC p = SharedWindowProc; // let compiler check type of SharedWindowProc
        m_lpfnSetWindowSubclass(hwnd, p, (UINT_PTR)_WindowProc, NULL); // _WindowProc is used as subclass ID
        return NULL;
    } else {
        return (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_WindowProc);
    }
}

void ComCtl32Util::UnsubclassHWND(HWND hwnd, WNDPROC _WindowProc, WNDPROC _DefWindowProc) {
    if (m_bNewSubclassing) {
        DASSERT(hModComCtl32 != NULL);
        DASSERT(_DefWindowProc == NULL);
        const SUBCLASSPROC p = SharedWindowProc; // let compiler check type of SharedWindowProc
        m_lpfnRemoveWindowSubclass(hwnd, p, (UINT_PTR)_WindowProc); // _WindowProc is used as subclass ID
    } else {
        ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_DefWindowProc);
    }
}

LRESULT ComCtl32Util::DefWindowProc(WNDPROC _DefWindowProc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (m_bNewSubclassing) {
        DASSERT(hModComCtl32 != NULL);
        DASSERT(_DefWindowProc == NULL);
        return m_lpfnDefSubclassProc(hwnd, msg, wParam, lParam);
    } else if (_DefWindowProc != NULL) {
        return ::CallWindowProc(_DefWindowProc, hwnd, msg, wParam, lParam);
    } else {
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

LRESULT ComCtl32Util::SharedWindowProc(HWND hwnd, UINT msg,
                                       WPARAM wParam, LPARAM lParam,
                                       UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    TRY;

    WNDPROC _WindowProc = (WNDPROC)uIdSubclass;
    return ::CallWindowProc(_WindowProc, hwnd, msg, wParam, lParam);

    CATCH_BAD_ALLOC_RET(0);
}

void ComCtl32Util::InitCommonControls()
{
    if (fn_InitCommonControlsEx == NULL) {
        return;
    }

    INITCOMMONCONTROLSEX iccex;
    iccex.dwSize = sizeof(iccex);
    iccex.dwICC = ICC_TAB_CLASSES;
    m_bToolTipControlInitialized = fn_InitCommonControlsEx(&iccex);
}

