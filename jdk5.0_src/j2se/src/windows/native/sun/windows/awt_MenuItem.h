/*
 * @(#)awt_MenuItem.h	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_MENUITEM_H
#define AWT_MENUITEM_H

#include "awt_Object.h"
#include "awt_Component.h"

#include <java_awt_MenuItem.h>
#include <sun_awt_windows_WMenuItemPeer.h>
#include <java_awt_Menu.h>
#include <sun_awt_windows_WMenuPeer.h>
#include <java_awt_MenuComponent.h>
#include <java_awt_peer_MenuComponentPeer.h>
#include <java_awt_FontMetrics.h>

class AwtMenu;


/************************************************************************
 * MenuItem class
 */

class AwtMenuItem : public AwtObject {
public:
    // id's for methods executed on toolkit thread
    enum {
	MENUITEM_SETLABEL,
	MENUITEM_ENABLE,
	MENUITEM_SETSTATE,
	MENUITEM_DISPOSE,
	MENUITEM_LAST
    };

    /* java.awt.MenuComponent fields */
    static jfieldID fontID;
    static jfieldID privateKeyID;
    static jfieldID appContextID;

    /* java.awt.MenuItem fields */
    static jfieldID labelID;
    static jfieldID enabledID;

    /* java.awt.CheckboxMenuItem fields */
    static jfieldID stateID;

    /* sun.awt.windows.WMenuItemPeer fields */
    static jfieldID isCheckboxID;
    static jfieldID shortcutLabelID;

    static jmethodID getDefaultFontMID;

    AwtMenuItem();
    virtual ~AwtMenuItem();

    virtual LPCTSTR GetClassName();

    void AwtMenuItem::LinkObjects(jobject peer);
    static AwtMenuItem* Create(jobject self, jobject menu);

    INLINE AwtMenu* GetMenuContainer() { return m_menuContainer; }
    INLINE void SetMenuContainer(AwtMenu* menu) { m_menuContainer = menu; }
    INLINE UINT GetID() { return m_Id; }
    INLINE void SetID(UINT id) { m_Id = id; }
    INLINE void SetNewID() {
        DASSERT(!m_freeId);
        m_Id = AwtToolkit::GetInstance().CreateCmdID(this);
        m_freeId = TRUE;
    }

    // Convert Language ID to CodePage
    static UINT LangToCodePage(LANGID idLang);
    /* Execute the command associated with this item. */
    virtual void DoCommand();

    void LinkObjects(JNIEnv *env, jobject peer);
    INLINE CriticalSection& GetLock() { return m_lock; }

    /* for multifont menuitem */
    INLINE jstring GetJavaString(JNIEnv *env) {
        if (env->EnsureLocalCapacity(2) < 0) {
	    return NULL;
	}
	jobject target = GetTarget(env);
	jstring res = (jstring)env->GetObjectField(target,
						   AwtMenuItem::labelID);
	env->DeleteLocalRef(target);
	return res;
    }
// Added by waleed for BIDI Support
    // returns the right to left status
    INLINE static BOOL GetRTLReadingOrder() {
        return sm_rtlReadingOrder;
    }
    // returns the right to left status
    INLINE static BOOL GetRTL() {
        return sm_rtl;
    }
    INLINE static LANGID GetSubLanguage() {
        return SUBLANGID(m_idLang);
    }
    // returns the current code page that should be used in
    // all MultiByteToWideChar and WideCharToMultiByte calls.
    // This code page should also be use in IsDBCSLeadByteEx.
    INLINE static UINT GetCodePage() {
        return m_CodePage;
    }
    INLINE static LANGID GetInputLanguage() {
        return m_idLang;
    }
// end waleed

    virtual void DrawItem(DRAWITEMSTRUCT& drawInfo);
    void DrawSelf(DRAWITEMSTRUCT& drawInfo);

    virtual void MeasureItem(HDC hDC, MEASUREITEMSTRUCT& measureInfo);
    void MeasureSelf(HDC hDC, MEASUREITEMSTRUCT& measureInfo);

    jobject GetFont(JNIEnv *env);
    jobject GetFontMetrics(JNIEnv *env, jobject font);
    jobject GetDefaultFont(JNIEnv *env);

    virtual BOOL IsTopMenu();
    void DrawCheck(HDC hDC, RECT rect);

    void SetLabel(LPCTSTR sb);
    virtual void Enable(BOOL isEnabled);
    void SetState(BOOL isChecked);

    /*
     * Windows message handler functions
     */
    MsgRouting WmNotify(UINT notifyCode);

    virtual LRESULT WinThreadExecProc(ExecuteArgs * args);
    virtual BOOL IsDisabledAndPopup() {
        return FALSE;
    }

protected:
    AwtMenu* m_menuContainer;  /* The menu object containing this item */
    UINT m_Id;                 /* The id of this item */

private:
    CriticalSection m_lock;
    BOOL m_freeId;

    // Added for bi-di support By Waleed
    static UINT m_CodePage;
    // Current input language (=low word of keyboardlayout handle)
    // m_idLang is shared by all instance of AwtComponent because
    // keyboardlayout is shared.
    static LANGID m_idLang;
    static BOOL m_isWin95;

    static BOOL sm_rtl;
    static BOOL sm_rtlReadingOrder;

public:
    static HBITMAP bmpCheck;
    static jobject systemFont;
};

#endif /* AWT_MENUITEM_H */
