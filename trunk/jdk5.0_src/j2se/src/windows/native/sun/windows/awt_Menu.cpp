/*
 * @(#)awt_Menu.cpp	1.45 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Menu.h"
#include "awt_MenuBar.h"
#include "awt_Frame.h"
#include <java_awt_Menu.h>
#include <sun_awt_windows_WMenuPeer.h>
#include <java_awt_MenuBar.h>
#include <sun_awt_windows_WMenuBarPeer.h>

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtMenuItem fields
 */

jmethodID AwtMenu::countItemsMID;
jmethodID AwtMenu::getItemMID;


/************************************************************************
 * AwtMenuItem methods
 */

AwtMenu::AwtMenu() {
    m_hMenu = NULL;
    m_parentMenu = NULL;
}

AwtMenu::~AwtMenu() {
    if (m_hMenu != NULL) {
	/*
	 * Don't verify -- may not be a valid anymore if its window
	 * was disposed of first.
	 */
        ::DestroyMenu(m_hMenu);
        m_hMenu = NULL;
    }
}

LPCTSTR AwtMenu::GetClassName() {
    return TEXT("SunAwtMenu");
}

/* Create a new AwtMenu object and menu.   */
AwtMenu* AwtMenu::Create(jobject self, AwtMenu* parentMenu)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtMenu* menu = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	target = env->GetObjectField(self, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	menu = new AwtMenu();

	menu->LinkObjects(env, self);
	menu->SetMenuContainer(parentMenu);
	{
	    HMENU hMenu = ::CreateMenu();
	    DASSERT(hMenu);
	    menu->SetHMenu(hMenu);

	    if (parentMenu != NULL) {
	        parentMenu->AddItem(menu);
	    }
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);

    return menu;
}

AwtMenuBar* AwtMenu::GetMenuBar() {
    return (m_parentMenu == NULL) ? NULL : m_parentMenu->GetMenuBar();
}

HWND AwtMenu::GetOwnerHWnd() {
    return (m_parentMenu == NULL) ? NULL : m_parentMenu->GetOwnerHWnd();
}

void AwtMenu::AddSeparator() {
    VERIFY(::AppendMenu(GetHMenu(), MF_SEPARATOR, 0, 0));
}

void AwtMenu::AddItem(AwtMenuItem* item)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(3) < 0) {
	return;
    }

    /* jitem is a java.awt.MenuItem */
    jobject jitem = item->GetTarget(env);

    jboolean enabled =
      (jboolean)env->GetBooleanField(jitem, AwtMenuItem::enabledID);

    UINT flags = MF_STRING | (enabled ? MF_ENABLED : MF_GRAYED);
    flags |= MF_OWNERDRAW;

    jstring label  =
      (jstring)(env)->GetObjectField(jitem, AwtMenuItem::labelID);

    if (_tcscmp(item->GetClassName(), TEXT("SunAwtMenu")) == 0) {
        AwtMenu* subMenu = (AwtMenu*)item;

        UINT menuID = static_cast<UINT>(reinterpret_cast<INT_PTR>(
            subMenu->GetHMenu()));
        VERIFY(::AppendMenu(GetHMenu(), flags | MF_POPUP, menuID,
                            (LPCTSTR)subMenu));
	if (GetRTL()) {
	    MENUITEMINFO  mif;
	    memset(&mif, 0, sizeof(MENUITEMINFO));
	    mif.cbSize = sizeof(MENUITEMINFO);
	    mif.fMask = MIIM_TYPE;
	    ::GetMenuItemInfo(GetHMenu(), menuID, FALSE, &mif);
	    mif.fType |= MFT_RIGHTJUSTIFY | MFT_RIGHTORDER;
	    ::SetMenuItemInfo(GetHMenu(), menuID, FALSE, &mif);
	}
        item->SetID(menuID);
        subMenu->SetParentMenu(this);

    } else {
	LPWSTR labelW = TO_WSTRING(label);
        if (labelW && (wcscmp(labelW, L"-") == 0)) {
            AddSeparator();
        } else {
            VERIFY(::AppendMenu(GetHMenu(), flags, item->GetID(),
                                (LPCTSTR)this));
	    if (GetRTL()) {
	        MENUITEMINFO  mif;
		memset(&mif, 0, sizeof(MENUITEMINFO));
		mif.cbSize = sizeof(MENUITEMINFO);
		mif.fMask = MIIM_TYPE;
		::GetMenuItemInfo(GetHMenu(), item->GetID(), FALSE, &mif);
		mif.fType |= MFT_RIGHTJUSTIFY | MFT_RIGHTORDER;
		::SetMenuItemInfo(GetHMenu(), item->GetID(), FALSE, &mif);
	    }
        }
    }

    env->DeleteLocalRef(jitem);
    env->DeleteLocalRef(label);
}

void AwtMenu::DeleteItem(UINT index)
{
    VERIFY(::RemoveMenu(GetHMenu(), index, MF_BYPOSITION));
}

void AwtMenu::SendDrawItem(AwtMenuItem* awtMenuItem,
			   DRAWITEMSTRUCT& drawInfo)
{
    if (_tcscmp(awtMenuItem->GetClassName(), TEXT("SunAwtMenu")) == 0) {
	AwtMenu* awtSubMenu = (AwtMenu*)awtMenuItem;
	awtSubMenu->DrawItem(drawInfo);
    }
    else {
	awtMenuItem->DrawItem(drawInfo);
    }
}

void AwtMenu::SendMeasureItem(AwtMenuItem* awtMenuItem,
			      HDC hDC, MEASUREITEMSTRUCT& measureInfo)
{
    if (_tcscmp(awtMenuItem->GetClassName(), TEXT("SunAwtMenu")) == 0) {
	AwtMenu* awtSubMenu = (AwtMenu*)awtMenuItem;
	awtSubMenu->MeasureItem(hDC, measureInfo);
    }
    else {
	awtMenuItem->MeasureItem(hDC, measureInfo);
    }
}

int AwtMenu::CountItem(jobject target)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jint nCount = env->CallIntMethod(target, AwtMenu::countItemsMID);
    DASSERT(!safe_ExceptionOccurred(env));
    return nCount;
}

AwtMenuItem* AwtMenu::GetItem(jobject target, jint index)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0) {
	return NULL;
    }
    jobject menuItem = env->CallObjectMethod(target, AwtMenu::getItemMID,
					     index);
    DASSERT(!safe_ExceptionOccurred(env));

    jobject wMenuItemPeer = GetPeerForTarget(env, menuItem);

    PDATA pData;
    AwtMenuItem* awtMenuItem = NULL;

    JNI_CHECK_PEER_GOTO(wMenuItemPeer, done);
    awtMenuItem = (AwtMenuItem*)pData;

 done:
    env->DeleteLocalRef(menuItem);
    env->DeleteLocalRef(wMenuItemPeer);

    return awtMenuItem;
}

void AwtMenu::DrawItems(DRAWITEMSTRUCT& drawInfo)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
    /* target is a java.awt.Menu */
    jobject target = GetTarget(env);

    int nCount = CountItem(target);
    for (int i = 0; i < nCount; i++) {
	AwtMenuItem* awtMenuItem = GetItem(target, i);
        if (awtMenuItem != NULL) {
            SendDrawItem(awtMenuItem, drawInfo);
        }
    }
    env->DeleteLocalRef(target);
}

void AwtMenu::DrawItem(DRAWITEMSTRUCT& drawInfo)
{
    DASSERT(drawInfo.CtlType == ODT_MENU);

    if (drawInfo.itemID ==
        static_cast<UINT>(reinterpret_cast<INT_PTR>(m_hMenu))) {
	DrawSelf(drawInfo);
	return;
    }
    DrawItems(drawInfo);
}

void AwtMenu::MeasureItems(HDC hDC, MEASUREITEMSTRUCT& measureInfo)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
   /* target is a java.awt.Menu */
    jobject target = GetTarget(env);
    int nCount = CountItem(target);
    for (int i = 0; i < nCount; i++) {
	AwtMenuItem* awtMenuItem = GetItem(target, i);
        if (awtMenuItem != NULL) {
	    SendMeasureItem(awtMenuItem, hDC, measureInfo);
        }
    }
    env->DeleteLocalRef(target);
}

void AwtMenu::MeasureItem(HDC hDC, MEASUREITEMSTRUCT& measureInfo)
{
    DASSERT(measureInfo.CtlType == ODT_MENU);

    if (measureInfo.itemID ==
        static_cast<UINT>(reinterpret_cast<INT_PTR>(m_hMenu))) {
        MeasureSelf(hDC, measureInfo);
	return;
    }

    MeasureItems(hDC, measureInfo);
}

BOOL AwtMenu::IsTopMenu()
{
    return (GetMenuBar() == GetParent());
}

LRESULT AwtMenu::WinThreadExecProc(ExecuteArgs * args)
{
    switch( args->cmdId ) {
	case MENU_ADDSEPARATOR:
	    this->AddSeparator();
	    break;

	case MENU_DELITEM:
	    this->DeleteItem(static_cast<UINT>(args->param1));
	    break;

	default:
	    AwtMenuItem::WinThreadExecProc(args);
	    break;
    }
    return 0L;
}

/************************************************************************
 * WMenuPeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_java_awt_Menu_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtMenu::countItemsMID = env->GetMethodID(cls, "countItemsImpl", "()I");
    AwtMenu::getItemMID = env->GetMethodID(cls, "getItemImpl",
					   "(I)Ljava/awt/MenuItem;");

    DASSERT(AwtMenu::countItemsMID != NULL);
    DASSERT(AwtMenu::getItemMID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * WMenuPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WMenuPeer
 * Method:    addSeparator
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WMenuPeer_addSeparator(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtObject::WinThreadExec(self, AwtMenu::MENU_ADDSEPARATOR);

    CATCH_BAD_ALLOC;
}


/*
 * Class:     sun_awt_windows_WMenuPeer
 * Method:    delItem
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WMenuPeer_delItem(JNIEnv *env, jobject self,
				       jint index)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtObject::WinThreadExec(self, AwtMenu::MENU_DELITEM, index);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WMenuPeer
 * Method:    createMenu
 * Signature: (Lsun/awt/windows/WMenuBarPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WMenuPeer_createMenu(JNIEnv *env, jobject self,
					  jobject menuBar)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(menuBar);
    AwtMenuBar* awtMenuBar = (AwtMenuBar *)pData;
    AwtToolkit::CreateComponent(self, awtMenuBar,
				(AwtToolkit::ComponentFactory)AwtMenu::Create,FALSE);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WMenuPeer
 * Method:    createSubMenu
 * Signature: (Lsun/awt/windows/WMenuPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WMenuPeer_createSubMenu(JNIEnv *env, jobject self,
					     jobject menu)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(menu);
    AwtMenu* awtMenu = (AwtMenu *)pData;
    AwtToolkit::CreateComponent(self, awtMenu,
				(AwtToolkit::ComponentFactory)AwtMenu::Create,FALSE);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
