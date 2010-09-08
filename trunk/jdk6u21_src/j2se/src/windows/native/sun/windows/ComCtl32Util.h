/*
 * @(#)ComCtl32Util.h	1.3 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _COMCTL32UTIL_H
#define _COMCTL32UTIL_H

#include "awt_Component.h"

#include <commctrl.h>

/*
 * comctl32.dll version 6 subclassing - taken from PlatformSDK/Include/commctrl.h
 */
typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, \
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

typedef BOOL (WINAPI *PFNSETWINDOWSUBCLASS)(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, \
    DWORD_PTR dwRefData);
typedef BOOL (WINAPI *PFNREMOVEWINDOWSUBCLASS)(HWND hWnd, SUBCLASSPROC pfnSubclass, \
    UINT_PTR uIdSubclass);

typedef LRESULT (WINAPI *PFNDEFSUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class ComCtl32Util
{
    public:
        static INLINE ComCtl32Util &GetInstance() {
            return m_Instance;
        }

        // loads comctl32.dll and checks if required routines are available
        // called from AwtToolkit::AwtToolkit()
        void InitLibraries();
        // unloads comctl32.dll
        // called from AwtToolkit::Dispose()
        void FreeLibraries();
        
        INLINE BOOL IsToolTipControlInitialized() {
            return m_bToolTipControlInitialized;
        }

        //-- comctl32.dll version 6 subclassing API --//

        INLINE BOOL IsNewSubclassing() {
            return m_bNewSubclassing;
        }

        // if comctl32.dll version 6 is used returns NULL, otherwise
        // returns default window proc
        WNDPROC SubclassHWND(HWND hwnd, WNDPROC _WindowProc);
        // DefWindowProc is the same as returned from SubclassHWND
        void UnsubclassHWND(HWND hwnd, WNDPROC _WindowProc, WNDPROC _DefWindowProc);
        // DefWindowProc is the same as returned from SubclassHWND or NULL
        LRESULT DefWindowProc(WNDPROC _DefWindowProc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        ComCtl32Util();
        ~ComCtl32Util();

        static ComCtl32Util m_Instance;

        HMODULE hModComCtl32;
        
        PFNSETWINDOWSUBCLASS m_lpfnSetWindowSubclass;
        PFNREMOVEWINDOWSUBCLASS m_lpfnRemoveWindowSubclass;
        PFNDEFSUBCLASSPROC m_lpfnDefSubclassProc;

        typedef BOOL (WINAPI * InitCommonControlsExType)(const LPINITCOMMONCONTROLSEX lpInitCtrls);
        InitCommonControlsExType fn_InitCommonControlsEx;

        void InitCommonControls();

        BOOL m_bNewSubclassing;
        BOOL m_bToolTipControlInitialized;

        // comctl32.dll version 6 window proc
        static LRESULT CALLBACK SharedWindowProc(HWND hwnd, UINT message,
                                                 WPARAM wParam, LPARAM lParam,
                                                 UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

        void InitFields();
};

#endif // _COMCTL32UTIL_H
