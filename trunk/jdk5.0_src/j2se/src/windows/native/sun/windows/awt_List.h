/*
 * @(#)awt_List.h	1.43 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_LIST_H
#define AWT_LIST_H

#include "awt_Component.h"

#include "java_awt_List.h"
#include "sun_awt_windows_WListPeer.h"


/************************************************************************
 * AwtList class
 */

class AwtList : public AwtComponent {
public:
    AwtList();
    virtual ~AwtList();

    virtual LPCTSTR GetClassName();
    
    static AwtList* Create(jobject peer, jobject parent);

    virtual BOOL NeedDblClick() { return TRUE; }

    INLINE void Select(int pos) {
        if (isMultiSelect) {
            SendListMessage(LB_SETSEL, TRUE, pos);
        }
        else {
            SendListMessage(LB_SETCURSEL, pos);
        }
    }
    INLINE void Deselect(int pos) {
        if (isMultiSelect) {
            SendListMessage(LB_SETSEL, FALSE, pos);
        }
        else {
            SendListMessage(LB_SETCURSEL, (WPARAM)-1);
        }
    }
    INLINE UINT GetCount() {
        LRESULT index = SendListMessage(LB_GETCOUNT);
        DASSERT(index != LB_ERR);
        return static_cast<UINT>(index);
    }
    INLINE UINT GetCurrentSelection() {
        LRESULT index = SendListMessage(LB_GETCURSEL);
        DASSERT(index != LB_ERR);
        return static_cast<UINT>(index);
    }

    INLINE void InsertString(WPARAM index, LPTSTR str) {
        VERIFY(SendListMessage(LB_INSERTSTRING, index, (LPARAM)str) != LB_ERR);
    }
    INLINE BOOL IsItemSelected(UINT index) {
        LRESULT ret = SendListMessage(LB_GETSEL, index);
        DASSERT(ret != LB_ERR);
        return (ret > 0);
    }
    INLINE BOOL InvalidateList(CONST RECT* lpRect, BOOL bErase) {
        DASSERT(GetListHandle());
        return InvalidateRect(GetListHandle(), lpRect, bErase);
    }

    // Adjust the horizontal scrollbar as necessary
    void AdjustHorizontalScrollbar();
    void UpdateMaxItemWidth();

    INLINE long GetMaxWidth() {
        return m_nMaxWidth;
    }

    INLINE void CheckMaxWidth(long nWidth) {
        if (nWidth > m_nMaxWidth) {
            m_nMaxWidth = nWidth;
            AdjustHorizontalScrollbar();
        }
    }

    BOOL ActMouseMessage(MSG* pMsg);

    // Netscape : Change the font on the list and redraw the 
    // items nicely. 
    virtual void SetFont(AwtFont *pFont);
    
    /* Set whether a list accepts single or multiple selections. */
    void SetMultiSelect(BOOL ms);

    /*for multifont list */
    jobject PreferredItemSize(JNIEnv *envx);

    /*
     * Windows message handler functions
     */
    MsgRouting WmMouseUp(UINT flags, int x, int y, int button);
    MsgRouting WmNotify(UINT notifyCode);
    MsgRouting WmKeyDown(UINT vkey, UINT repCnt, UINT flags, BOOL system);

    /* for multifont list */
    MsgRouting OwnerDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
    MsgRouting OwnerMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT& measureInfo);

    //for horizontal scrollbar
    MsgRouting WmSize(UINT type, int w, int h);

    MsgRouting WmCtlColor(HDC hDC, HWND hCtrl, UINT ctlColor,
			  HBRUSH& retBrush);
    MsgRouting WmSetFocus(HWND hWndLostFocus);
    MsgRouting WmKillFocus(HWND hWndGotFocus);

    MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    MsgRouting WmPrint(HDC hDC, LPARAM flags);

    static LRESULT CALLBACK WrapProc(HWND hWnd, UINT message, 
	WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListProc(HWND hWnd, UINT message, 
	WPARAM wParam, LPARAM lParam);

    LRESULT DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

    virtual void SetDragCapture(UINT flags);
    virtual void ReleaseDragCapture(UINT flags);
    void Reshape(int x, int y, int w, int h);

    INLINE LRESULT SendListMessage(UINT msg, WPARAM wParam=0, LPARAM lParam=0) 
    {
        DASSERT(GetListHandle() != NULL);
        return ::SendMessage(GetListHandle(), msg, wParam, lParam);
    }
    INLINE virtual LONG GetStyle() {
        DASSERT(GetListHandle());
        return ::GetWindowLong(GetListHandle(), GWL_STYLE);
    }
    INLINE virtual void SetStyle(LONG style) {
        DASSERT(GetListHandle());
        // SetWindowLong() error handling as recommended by Win32 API doc.
        ::SetLastError(0);
        LONG ret = ::SetWindowLong(GetListHandle(), GWL_STYLE, style);
        DASSERT(ret != 0 || ::GetLastError() == 0);
    }
    INLINE virtual LONG GetStyleEx() {
        DASSERT(GetListHandle());
        return ::GetWindowLong(GetListHandle(), GWL_EXSTYLE);
    }
    INLINE virtual void SetStyleEx(LONG style) {
        DASSERT(GetListHandle());
        // SetWindowLong() error handling as recommended by Win32 API doc.
        ::SetLastError(0);
        LONG ret = ::SetWindowLong(GetListHandle(), GWL_EXSTYLE, style);
        DASSERT(ret != 0 || ::GetLastError() == 0);
    }

    void SubclassHWND();
    void UnsubclassHWND();

    INLINE HWND GetDBCSEditHandle() { return GetListHandle(); }

    virtual BOOL InheritsNativeMouseWheelBehavior();

protected:
    INLINE HWND GetListHandle() { return GetWrappeeHandle(); }
    HWND    SetListHandle(HWND hList) { 
	HWND hOldList = m_hList;
	m_hList = hList; 
	return hOldList;
    }

    static BOOL IsListOwnerMessage(UINT message) {
	switch (message) {
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_COMMAND:
#if defined(WIN32)
	case WM_CTLCOLORLISTBOX:
#else
	case WM_CTLCOLOR:
#endif
	    return TRUE;
	}
	return FALSE;
    }

    static BOOL IsAwtMessage(UINT message) {
	return (message >= WM_APP);
    }

    void CreateList(DWORD style, DWORD exStyle, int w, int h);
    void DestroyList();

    virtual INLINE HWND GetWrappeeHandle() { return m_hList; }

private:
    BOOL isMultiSelect;
    BOOL isWrapperPrint;

    // The width of the longest item in the listbox
    long m_nMaxWidth;
    WNDPROC  m_listDefWindowProc;    
    HWND     m_hList;

};

#endif /* AWT_LIST_H */
