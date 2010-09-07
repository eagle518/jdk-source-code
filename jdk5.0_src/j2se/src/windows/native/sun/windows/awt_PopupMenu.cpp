/*
 * @(#)awt_PopupMenu.cpp	1.36 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_PopupMenu.h"

#include "awt_Event.h"

#include <java_awt_PopupMenu.h>
#include <sun_awt_windows_WPopupMenuPeer.h>
#include <java_awt_Event.h>

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtPopupMenu class methods
 */

AwtPopupMenu::AwtPopupMenu() {
    m_parent = NULL;
}

AwtPopupMenu::~AwtPopupMenu() {
    m_parent = NULL;
}

LPCTSTR AwtPopupMenu::GetClassName() {
  return TEXT("SunAwtPopupMenu");
}

/* Create a new AwtPopupMenu object and menu.   */
AwtPopupMenu* AwtPopupMenu::Create(jobject self, AwtComponent* parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtPopupMenu* popupMenu = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	target = env->GetObjectField(self, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	popupMenu = new AwtPopupMenu();

	{
	    popupMenu->LinkObjects(env, self);

	    HMENU hMenu = ::CreatePopupMenu();
	    DASSERT(hMenu);
	    popupMenu->SetHMenu(hMenu);
	    popupMenu->SetParent(parent);
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return popupMenu;
}

void AwtPopupMenu::Show(JNIEnv *env, jobject event)
{
    /*
     * Convert the event's XY to absolute coordinates.  The XY is
     * relative to the origin component, which is passed by PopupMenu
     * as the event's target.
     */
    if (env->EnsureLocalCapacity(2) < 0) {
	return;
    }
    jobject origin = (env)->GetObjectField(event, AwtEvent::targetID);
    jobject peerOrigin = GetPeerForTarget(env, origin);
    PDATA pData;
    JNI_CHECK_PEER_GOTO(peerOrigin, done);
    {
        AwtComponent* awtOrigin = (AwtComponent*)pData;
        POINT pt;
        pt.x = (env)->GetIntField(event, AwtEvent::xID);
        pt.y = (env)->GetIntField(event, AwtEvent::yID);
        ::MapWindowPoints(awtOrigin->GetHWnd(), 0, (LPPOINT)&pt, 1);

        // Adjust to account for the Inset values
        RECT rctInsets;
        awtOrigin->GetInsets(&rctInsets);
        pt.x -= rctInsets.left;
        pt.y -= rctInsets.top;

        /* Invoke the popup. */
        ::TrackPopupMenu( m_hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, awtOrigin->GetHWnd(), NULL );
    }
 done:
    env->DeleteLocalRef(origin);
    env->DeleteLocalRef(peerOrigin);
}

void AwtPopupMenu::AddItem(AwtMenuItem *item)
{
    AwtMenu::AddItem(item);
    if (GetMenuContainer() != NULL) return;
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
    jobject target = GetTarget(env);
    if (!(jboolean)env->GetBooleanField(target, AwtMenuItem::enabledID)) {
        item->Enable(FALSE);
    }
    env->DeleteLocalRef(target);
}

void AwtPopupMenu::Enable(BOOL isEnabled)
{
    AwtMenu *menu = GetMenuContainer();
    if (menu != NULL) {
        AwtMenu::Enable(isEnabled);
        return;
    }
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
    jobject target = GetTarget(env);
    int nCount = CountItem(target);
    for (int i = 0; i < nCount; ++i) {
        AwtMenuItem *item = GetItem(target,i);
        jobject jitem = item->GetTarget(env);
        BOOL bItemEnabled = isEnabled && (jboolean)env->GetBooleanField(jitem, 
            AwtMenuItem::enabledID);
        LPWSTR labelW = TO_WSTRING((jstring)env->GetObjectField(jitem, 
            AwtMenuItem::labelID));
        if (labelW != NULL && wcscmp(labelW,L"-") != 0) {
            item->Enable(bItemEnabled);
        }
        env->DeleteLocalRef(jitem);
    }
    env->DeleteLocalRef(target);
}

BOOL AwtPopupMenu::IsDisabledAndPopup()
{
    if (GetMenuContainer() != NULL) return FALSE;
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return FALSE;
    }
    jobject target = GetTarget(env);    
    BOOL bEnabled = (jboolean)env->GetBooleanField(target, 
            AwtMenuItem::enabledID);
    env->DeleteLocalRef(target);
    return !bEnabled;
}
        
/************************************************************************
 * WPopupMenuPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WPopupMenuPeer
 * Method:    createMenu
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WPopupMenuPeer_createMenu(JNIEnv *env, jobject self,
					       jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtComponent* awtParent = (AwtComponent *)pData;
    AwtToolkit::CreateComponent(
	self, awtParent, (AwtToolkit::ComponentFactory)AwtPopupMenu::Create, FALSE);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WPopupMenuPeer
 * Method:    _show
 * Signature: (Ljava/awt/Event;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WPopupMenuPeer__1show(JNIEnv *env, jobject self,
					   jobject event)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(event, "null Event");

    // Invoke popup on toolkit thread -- yet another Win32 restriction.
    AwtPopupMenu* popupMenu = (AwtPopupMenu*)pData;
    AwtToolkit::GetInstance().SendMessage(WM_AWT_POPUPMENU_SHOW,
					  (WPARAM)popupMenu, (LPARAM)event);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
