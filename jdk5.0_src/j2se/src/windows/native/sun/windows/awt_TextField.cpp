/*
 * @(#)awt_TextField.cpp	1.50 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_TextField.h"
#include "awt_TextComponent.h"
#include "awt_dlls.h"
#include "awt_KeyboardFocusManager.h"
#include "awt_Canvas.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtTextField methods
 */

AwtTextField::AwtTextField() {
}

/* Create a new AwtTextField object and window.   */
AwtTextField* AwtTextField::Create(jobject peer, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtTextField* c = NULL;

    try {
        PDATA pData;
	AwtCanvas* awtParent;
	JNI_CHECK_PEER_GOTO(parent, done);
	awtParent = (AwtCanvas*)pData;

	JNI_CHECK_NULL_GOTO(awtParent, "null awtParent", done);

	target = env->GetObjectField(peer, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	c = new AwtTextField();

	{
	    DWORD style = WS_CHILD | WS_CLIPSIBLINGS |
	        ES_LEFT | ES_AUTOHSCROLL |
	        (IS_WIN4X ? 0 : WS_BORDER);
	    DWORD exStyle = IS_WIN4X ? WS_EX_CLIENTEDGE : 0;
	    if (GetRTL()) {
	        exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
		if (GetRTLReadingOrder())
		    exStyle |= WS_EX_RTLREADING;
	    }

	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);

	    c->CreateHWnd(env, L"", style, exStyle,
			  x, y, width, height,
			  awtParent->GetHWnd(),
			  reinterpret_cast<HMENU>(static_cast<INT_PTR>(
                awtParent->CreateControlID())),
			  ::GetSysColor(COLOR_WINDOWTEXT),
			  ::GetSysColor(COLOR_WINDOW),
			  peer);

	    c->m_backgroundColorSet = TRUE;
	    /* suppress inheriting parent's color. */
            c->UpdateBackground(env, target);
            c->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                           MAKELPARAM(1, 1));
            /*
             * Fix for BugTraq Id 4260109.
             * Set the text limit to the maximum.
             */  
            c->SendMessage(EM_SETLIMITTEXT);

	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);

    return c;
}

void AwtTextField::EditSetSel(CHARRANGE &cr) {
    SendMessage(EM_SETSEL, cr.cpMin, cr.cpMax);
}

void AwtTextField::EditGetSel(CHARRANGE &cr) {
    SendMessage(EM_SETSEL, reinterpret_cast<WPARAM>(&cr.cpMin), reinterpret_cast<LPARAM>(&cr.cpMax));
}

LONG AwtTextField::EditGetCharFromPos(POINT& pt) {
    return static_cast<LONG>(SendMessage(EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y)));
}

MsgRouting
AwtTextField::HandleEvent(MSG *msg, BOOL synthetic)
{
    MsgRouting returnVal;
    /*
     * RichEdit 1.0 control starts internal message loop if the 
     * left mouse button is pressed while the cursor is not over 
     * the current selection or the current selection is empty.
     * Because of this we don't receive WM_MOUSEMOVE messages
     * while the left mouse button is pressed. To work around
     * this behavior we process the relevant mouse messages
     * by ourselves.
     * By consuming WM_MOUSEMOVE messages we also don't give 
     * the RichEdit control a chance to recognize a drag gesture 
     * and initiate its own drag-n-drop operation.
     */
    /**
     * In non-focusable mode we don't pass mouse messages to native window thus making user unable
     * to select the text. Below is the code from awt_TextArea.cpp which implements selection
     * functionality. For safety this code is only being executed in non-focusable mode.
     */
    if (!IsFocusable()) { 
        if (msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONDBLCLK) {
            CHARRANGE cr;
        
            LONG lCurPos = EditGetCharFromPos(msg->pt);

            EditGetSel(cr);
            /*
             * NOTE: Plain EDIT control always clears selection on mouse 
             * button press. We are clearing the current selection only if 
             * the mouse pointer is not over the selected region.
             * In this case we sacrifice backward compatibility 
             * to allow dnd of the current selection.
             */
            if (msg->message == WM_LBUTTONDBLCLK) {
                SetStartSelectionPos(static_cast<LONG>(SendMessage(
                    EM_FINDWORDBREAK, WB_MOVEWORDLEFT, lCurPos)));
                SetEndSelectionPos(static_cast<LONG>(SendMessage(
                    EM_FINDWORDBREAK, WB_MOVEWORDRIGHT, lCurPos)));
            } else {
                SetStartSelectionPos(lCurPos);
                SetEndSelectionPos(lCurPos);
            }
            cr.cpMin = GetStartSelectionPos();
            cr.cpMax = GetEndSelectionPos();
            EditSetSel(cr);

            delete msg;
            return mrConsume;
        } else if (msg->message == WM_LBUTTONUP) {

            /*
             * If the left mouse button is pressed on the selected region
             * we don't clear the current selection. We clear it on button
             * release instead. This is to allow dnd of the current selection.
             */
            if (GetStartSelectionPos() == -1 && GetEndSelectionPos() == -1) {
                CHARRANGE cr;

                LONG lCurPos = EditGetCharFromPos(msg->pt);

                cr.cpMin = lCurPos;
                cr.cpMax = lCurPos;
                EditSetSel(cr);
            }

            /* 
             * Cleanup the state variables when left mouse button is released.
             * These state variables are designed to reflect the selection state 
             * while the left mouse button is pressed and be set to -1 otherwise.
             */
            SetStartSelectionPos(-1);
            SetEndSelectionPos(-1);
            SetLastSelectionPos(-1);

            delete msg;
            return mrConsume;
        } else if (msg->message == WM_MOUSEMOVE && (msg->wParam & MK_LBUTTON)) {

            /* 
             * We consume WM_MOUSEMOVE while the left mouse button is pressed,
             * so we have to simulate autoscrolling when mouse is moved outside
             * of the client area.
             */
            POINT p;
            RECT r;
            BOOL bScrollLeft = FALSE;
            BOOL bScrollRight = FALSE;
            BOOL bScrollUp = FALSE;
            BOOL bScrollDown = FALSE;
        
            p.x = msg->pt.x;
            p.y = msg->pt.y;
            VERIFY(::GetClientRect(GetHWnd(), &r));
        
            if (p.x < 0) {
                bScrollLeft = TRUE;
                p.x = 0;
            } else if (p.x > r.right) {
                bScrollRight = TRUE;
                p.x = r.right - 1;
            }
            LONG lCurPos = EditGetCharFromPos(p);
        
            if (GetStartSelectionPos() != -1 && 
                GetEndSelectionPos() != -1 &&
                lCurPos != GetLastSelectionPos()) {
            
                CHARRANGE cr;
            
                SetLastSelectionPos(lCurPos);
            
                cr.cpMin = GetStartSelectionPos();
                cr.cpMax = GetLastSelectionPos();

                EditSetSel(cr);
            }
        
            if (bScrollLeft == TRUE || bScrollRight == TRUE) {
                SCROLLINFO si;
                memset(&si, 0, sizeof(si));
                si.cbSize = sizeof(si);
                si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;

                VERIFY(::GetScrollInfo(GetHWnd(), SB_HORZ, &si));
                if (bScrollLeft == TRUE) {
                    si.nPos = si.nPos - si.nPage / 2;
                    si.nPos = max(si.nMin, si.nPos);
                } else if (bScrollRight == TRUE) {
                    si.nPos = si.nPos + si.nPage / 2;
                    si.nPos = min(si.nPos, si.nMax);
                }
                /*
                 * Okay to use 16-bit position since RichEdit control adjusts 
                 * its scrollbars so that their range is always 16-bit.
                 */
                DASSERT(abs(si.nPos) < 0x8000);
                SendMessage(WM_HSCROLL, 
                            MAKEWPARAM(SB_THUMBPOSITION, LOWORD(si.nPos)));
            }
            delete msg;
            return mrConsume;
        }
    }
    /*
     * Store the 'synthetic' parameter so that the WM_PASTE security check
     * happens only for synthetic events.
     */
    m_synthetic = synthetic;
    returnVal = AwtComponent::HandleEvent(msg, synthetic);
    m_synthetic = FALSE;

    return returnVal;
}


/************************************************************************
 * WTextFieldPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WTextFieldPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WTextFieldPeer_create(JNIEnv *env, jobject self,
					   jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent,
				(AwtToolkit::ComponentFactory)
				AwtTextField::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTextFieldPeer
 * Method:    setEchoCharacter
 * Signature: (C)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WTextFieldPeer_setEchoCharacter(JNIEnv *env, jobject self,
						     jchar ch)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtComponent* c = (AwtComponent*)pData;
    c->SendMessage(EM_SETPASSWORDCHAR, ch);
    /*
     * Fix for BugTraq ID 4307281.
     * Force redraw so that changes will take effect.
     */
    VERIFY(::InvalidateRect(c->GetHWnd(), NULL, FALSE));

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
