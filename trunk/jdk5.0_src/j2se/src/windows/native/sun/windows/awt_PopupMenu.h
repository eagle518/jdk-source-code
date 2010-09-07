/*
 * @(#)awt_PopupMenu.h	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_POPUPMENU_H
#define AWT_POPUPMENU_H

#include "awt_Menu.h"

#include <java_awt_MenuItem.h>
#include <sun_awt_windows_WMenuItemPeer.h>
#include <java_awt_PopupMenu.h>
#include <sun_awt_windows_WPopupMenuPeer.h>


/************************************************************************
 * AwtPopupMenu class
 */

class AwtPopupMenu : public AwtMenu {
public:
    AwtPopupMenu();
    virtual ~AwtPopupMenu();
    virtual LPCTSTR GetClassName();

    /* Create a new AwtPopupMenu.  This must be run on the main thread. */
    static AwtPopupMenu* Create(jobject self, AwtComponent* parent);

    /* Display the popup modally. */
    void Show(JNIEnv *env, jobject event);

    virtual AwtMenuBar* GetMenuBar() { return NULL; }
    INLINE AwtMenu* GetParent() { return NULL; }
    INLINE void SetParentMenu(AwtMenu* parent) { DASSERT(FALSE); }
    INLINE void SetParent(AwtComponent* parent) { m_parent = parent; }
    virtual HWND GetOwnerHWnd() { 
        return (m_parent == NULL) ? NULL : m_parent->GetHWnd(); 
    }
    virtual void Enable(BOOL isEnabled);
    virtual BOOL IsDisabledAndPopup();
    virtual void AddItem(AwtMenuItem *item);

protected:
    AwtComponent* m_parent;
};

#endif /* AWT_POPUPMENU_H */
