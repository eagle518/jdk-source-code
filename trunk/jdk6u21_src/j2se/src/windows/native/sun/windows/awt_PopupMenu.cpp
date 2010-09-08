/*
 * @(#)awt_PopupMenu.cpp	1.44 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_PopupMenu.h"

#include "awt_Event.h"
#include "awt_Window.h"

#include <java_awt_PopupMenu.h>
#include <sun_awt_windows_WPopupMenuPeer.h>
#include <java_awt_Event.h>

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/***********************************************************************/
// struct for _Show method
struct ShowStruct {
    jobject self;
    jobject event;
};

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

        SetLastError(0);
        HMENU hMenu = ::CreatePopupMenu();
        // fix for 5088782
        if (!CheckMenuCreation(env, self, hMenu))
        {
            env->DeleteLocalRef(target);
            return NULL;
        }

        popupMenu->SetHMenu(hMenu);

        popupMenu->LinkObjects(env, self);
        popupMenu->SetParent(parent);
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return popupMenu;
}

void AwtPopupMenu::Show(JNIEnv *env, jobject event, BOOL isTrayIconPopup)
{
    /*
     * For not TrayIcon popup.
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
        UINT flags = 0;
        pt.x = (env)->GetIntField(event, AwtEvent::xID);
        pt.y = (env)->GetIntField(event, AwtEvent::yID);
        
        if (!isTrayIconPopup) {
            ::MapWindowPoints(awtOrigin->GetHWnd(), 0, (LPPOINT)&pt, 1);

            // Adjust to account for the Inset values
            RECT rctInsets;
            awtOrigin->GetInsets(&rctInsets);
            pt.x -= rctInsets.left;
            pt.y -= rctInsets.top;

            flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;

        } else {
            ::SetForegroundWindow(awtOrigin->GetHWnd());

            flags = TPM_NONOTIFY | TPM_RIGHTALIGN | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN;
        }

        /* Invoke the popup. */
        ::TrackPopupMenu(m_hMenu, flags, pt.x, pt.y, 0, awtOrigin->GetHWnd(), NULL);

        if (isTrayIconPopup) {
            ::PostMessage(awtOrigin->GetHWnd(), WM_NULL, 0, 0);
        }
    }
 done:
    env->DeleteLocalRef(origin);
    env->DeleteLocalRef(peerOrigin);
    env->DeleteGlobalRef(event);
}

void AwtPopupMenu::_Show(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    static jclass popupMenuCls;
    if (popupMenuCls == NULL) {
        jclass popupMenuClsLocal = 
            env->FindClass("java/awt/PopupMenu");
        if (!popupMenuClsLocal) {
            /* exception already thrown */
            return;
        }
        popupMenuCls = (jclass)env->NewGlobalRef(popupMenuClsLocal);
        env->DeleteLocalRef(popupMenuClsLocal);
    }

    static jfieldID isTrayIconPopupID;
    if (isTrayIconPopupID == NULL) {
        isTrayIconPopupID = env->GetFieldID(popupMenuCls, "isTrayIconPopup", "Z");
        DASSERT(isTrayIconPopupID);
    }

    if (AwtToolkit::IsMainThread()) {
        ShowStruct *ss = (ShowStruct*)param;
        if (ss->self != NULL) {
            PDATA pData = JNI_GET_PDATA(ss->self);
            if (pData) {
                AwtPopupMenu *p = (AwtPopupMenu *)pData;
                jobject target = p->GetTarget(env);    
                BOOL isTrayIconPopup = env->GetBooleanField(target, isTrayIconPopupID);
                env->DeleteLocalRef(target);
                p->Show(env, ss->event, isTrayIconPopup);
            }
            env->DeleteGlobalRef(ss->self);
            delete ss;
        }
    } else {
        AwtToolkit::GetInstance().InvokeFunction(AwtPopupMenu::_Show, param);
    }   
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

    ShowStruct *ss = new ShowStruct;
    ss->self = env->NewGlobalRef(self);
    ss->event = env->NewGlobalRef(event);

    AwtToolkit::GetInstance().SyncCall(AwtPopupMenu::_Show, ss);
    // global ref is deleted in _Show() and ss is deleted in Show()

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
