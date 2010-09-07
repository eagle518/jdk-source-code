/*
 * @(#)awt_Menu.h	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_MENU_H
#define AWT_MENU_H

#include "awt_MenuItem.h"

#include <java_awt_MenuItem.h>
#include <sun_awt_windows_WMenuItemPeer.h>
#include <java_awt_Menu.h>
#include <sun_awt_windows_WMenuPeer.h>

class AwtMenuBar;


/************************************************************************
 * AwtMenu class
 */

class AwtMenu : public AwtMenuItem {
public:
    // id's for methods executed on toolkit thread
    enum {
	MENU_ADDSEPARATOR = MENUITEM_LAST+1,
	MENU_DELITEM,
	MENU_LAST
    };

    /* method ids for java.awt.Menu */
    static jmethodID countItemsMID;
    static jmethodID getItemMID;

    AwtMenu();
    virtual ~AwtMenu();
    virtual LPCTSTR GetClassName();

    /* Create a new AwtMenu.  This must be run on the main thread. */
    static AwtMenu* Create(jobject self, AwtMenu* parentMenu);

    INLINE HMENU GetHMenu() { return m_hMenu; }
    INLINE void SetHMenu(HMENU hMenu) { m_hMenu = hMenu; }

    virtual AwtMenuBar* GetMenuBar();

    INLINE AwtMenu* GetParent() { return m_parentMenu; }
    INLINE void SetParentMenu(AwtMenu* parent) { m_parentMenu = parent; }

    void AddSeparator();
    virtual void AddItem(AwtMenuItem *item);
    virtual void DeleteItem(UINT index);

    virtual HWND GetOwnerHWnd();

    /*for multifont menu */
    BOOL IsTopMenu();
    virtual AwtMenuItem* GetItem(jobject target, long index);

    virtual int CountItem(jobject target);

    virtual void SendDrawItem(AwtMenuItem* awtMenuItem, 
			      DRAWITEMSTRUCT& drawInfo);
    virtual void SendMeasureItem(AwtMenuItem* awtMenuItem, HDC hDC, 
				 MEASUREITEMSTRUCT& measureInfo);
    void DrawItem(DRAWITEMSTRUCT& drawInfo);
    void DrawItems(DRAWITEMSTRUCT& drawInfo);
    void MeasureItem(HDC hDC, MEASUREITEMSTRUCT& measureInfo);
    void MeasureItems(HDC hDC, MEASUREITEMSTRUCT& measureInfo);

    virtual LRESULT WinThreadExecProc(ExecuteArgs * args);
protected:
    HMENU    m_hMenu;
    AwtMenu* m_parentMenu;	/* can also be an AwtMenuBar */
};

#endif /* AWT_MENU_H */
