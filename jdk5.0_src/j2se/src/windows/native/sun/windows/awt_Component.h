/*
 * @(#)awt_Component.h	1.174 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_COMPONENT_H
#define AWT_COMPONENT_H

#include "awtmsg.h"
#include "awt_Object.h"
#include "awt_Font.h"
#include "awt_Brush.h"
#include "awt_Pen.h"
#include "awt_Win32GraphicsDevice.h"
#include "Win32SurfaceData.h"

#include "java_awt_Component.h"
#include "sun_awt_windows_WComponentPeer.h"
#include "java_awt_event_KeyEvent.h"
#include "java_awt_event_FocusEvent.h"
#include "java_awt_event_MouseEvent.h"
#include "java_awt_event_WindowEvent.h"
#include "java_awt_Dimension.h"

extern LPCTSTR szAwtComponentClassName;

// property name tagging windows disabled by modality
static LPCTSTR ModalDisableProp = TEXT("SunAwtModalDisableProp");
static LPCTSTR DrawingStateProp = TEXT("SunAwtDrawingStateProp");
const UINT32 INITIALLY_DISABLED_MASK =	0x80000000;
const UINT32 DISABLED_LEVEL_MASK =	0x7FFFFFFF;

const UINT IGNORE_KEY = (UINT)-1;
const UINT MAX_ACP_STR_LEN = 7; // ANSI CP identifiers are no longer than this

#define LEFT_BUTTON 1
#define MIDDLE_BUTTON 2
#define RIGHT_BUTTON 3
#define DBL_CLICK 4

// Whether to check for embedded frame and adjust location
#define CHECK_EMBEDDED 0
#define DONT_CHECK_EMBEDDED 1

class AwtPopupMenu;

class AwtDropTarget;

class DDrawDisplayMode;

struct WmComponentSetFocusData;

/*
 * Message routing codes
 */
enum MsgRouting {
    mrPassAlong,    /* pass along to next in chain */
    mrDoDefault,    /* skip right to underlying default behavior */
    mrConsume,      /* consume msg & terminate routing immediatly, 
		     * don't pass anywhere 
		     */
};

/************************************************************************
 * AwtComponent class
 */

class AwtComponent : public AwtObject {
public:
    enum {
	// combination of all mouse button flags
	ALL_MK_BUTTONS = MK_LBUTTON|MK_MBUTTON|MK_RBUTTON
    };

    /* java.awt.Component fields and method IDs */
    static jfieldID peerID;
    static jfieldID xID;
    static jfieldID yID;
    static jfieldID widthID;
    static jfieldID heightID;
    static jfieldID visibleID;
    static jfieldID backgroundID;
    static jfieldID foregroundID;
    static jfieldID enabledID;
    static jfieldID parentID;
    static jfieldID cursorID;
    static jfieldID graphicsConfigID;
    static jfieldID privateKeyID;
    static jfieldID peerGCID;
    static jfieldID focusableID;
    static jfieldID appContextID;
    static jfieldID hwndID;

    static jmethodID getFontMID;
    static jmethodID getToolkitMID;
    static jmethodID isEnabledMID;
    static jmethodID getLocationOnScreenMID;
    static jmethodID resetGCMID;
    static jmethodID replaceSurfaceDataMID;
    static jmethodID replaceSurfaceDataLaterMID;

    static const UINT WmAwtIsComponent;

    AwtComponent();
    virtual ~AwtComponent();

    /*
     * Dynamic class registration & creation 
     */
    virtual LPCTSTR GetClassName() = 0;
    virtual void FillClassInfo(WNDCLASS *lpwc);
    virtual void RegisterClass();
    virtual void UnregisterClass();

    void CreateHWnd(JNIEnv *env, LPCWSTR title, 
		    DWORD windowStyle, DWORD windowExStyle,
                    int x, int y, int w, int h,
                    HWND hWndParent, HMENU hMenu, 
                    COLORREF colorForeground, COLORREF colorBackground,
                    jobject peer);
    void InitPeerGraphicsConfig(JNIEnv *env, jobject peer);

    void DestroyHWnd();
    void UpdateBackground(JNIEnv *env, jobject target);
    virtual void SubclassHWND();
    void UnsubclassHWND();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
	WPARAM wParam, LPARAM lParam);

    /*
     * Access to the various objects of this aggregate component 
     */
    INLINE HWND GetHWnd() { return m_hwnd; }
    INLINE void SetHWnd(HWND hwnd) { m_hwnd = hwnd; }

    static AwtComponent* GetComponent(HWND hWnd);

    INLINE CriticalSection& GetLock() { return m_lock; }

    /*
     * Access to the properties of the component 
     */
    INLINE COLORREF GetColor() { return m_colorForeground; }
    virtual void SetColor(COLORREF c);
    HPEN GetForegroundPen();

    COLORREF GetBackgroundColor();
    virtual void SetBackgroundColor(COLORREF c);
    HBRUSH GetBackgroundBrush();
    INLINE BOOL IsBackgroundColorSet() { return m_backgroundColorSet; }

    virtual void SetFont(AwtFont *pFont);

    INLINE void SetText(LPCTSTR text) { ::SetWindowText(GetHWnd(), text); }
    INLINE int GetText(LPTSTR buffer, int size) { 
        return ::GetWindowText(GetHWnd(), buffer, size); 
    }
    INLINE int GetTextLength() { return ::GetWindowTextLength(GetHWnd()); }

    virtual void GetInsets(RECT* rect) { 
        VERIFY(::SetRectEmpty(rect));
    }
    
    /* 
     * Enable/disable component
     */
    virtual void Enable(BOOL bEnable);

    /*
     * Validate and call handleExpose on rects of UpdateRgn
     */
    void PaintUpdateRgn(const RECT *insets);

    static HWND GetTopLevelParentForWindow(HWND hwndDescendant);

    static jobject FindHeavyweightUnderCursor(BOOL useCache);

    /*
     * Returns the parent component.  If no parent window, or the
     * parent window isn't an AwtComponent, returns NULL.  
     */
    AwtComponent* GetParent();

    /* Get the component's immediate container. */
    class AwtWindow* GetContainer();

    /* Is a component a container? Used by above method */
    virtual BOOL IsContainer() { return FALSE;} // Plain components can't

    /**
     * Perform some actions which by default are being performed by Default Window procedure of
     * this window class
     * For detailed comments see implementation in awt_Component.cpp
     */
    virtual BOOL ActMouseMessage(MSG * pMsg);
    /**
     * Returns TRUE if this message will this component to become focused. Returns FALSE otherwise.
     */
    inline BOOL IsFocusingMessage(UINT message) {
        return message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK;
    }

    BOOL IsFocusable();

    /*
     * Returns an increasing unsigned value used for child control IDs.  
     * There is no attempt to reclaim command ID's.
     */
    INLINE UINT CreateControlID() { return m_nextControlID++; }

    // returns the current keyboard layout
    INLINE static HKL GetKeyboardLayout() {
	return m_hkl;
    } 

    // returns the current code page that should be used in 
    // all MultiByteToWideChar and WideCharToMultiByte calls.
    // This code page should also be use in IsDBCSLeadByteEx.
    INLINE static UINT GetCodePage()
    {
        return m_CodePage;
    }
    
// Added by waleed for BIDI Support
    // returns the right to left status
    INLINE static BOOL GetRTLReadingOrder() {
        return sm_rtlReadingOrder;
    }
    // returns the right to left status
    INLINE static BOOL GetRTL()	{
        return sm_rtl;
    }
    // returns the current sub language
    INLINE static LANGID GetSubLanguage() {
        return SUBLANGID(m_idLang);
    }
// end waleed

    // returns the current input language
    INLINE static LANGID GetInputLanguage()
    {
        return m_idLang;
    }
    // Convert Language ID to CodePage
    static UINT LangToCodePage(LANGID idLang);

    /*
     * methods on this component
     */
    virtual void Show();
    virtual void Hide();
    virtual void Reshape(int x, int y, int w, int h);

    /*
     * Fix for 4046446.
     * Component size/position helper, for the values above the short int limit.
     */
    static BOOL SetWindowPos(HWND wnd, HWND after,
			     int x, int y, int w, int h, UINT flags);
 
    /*
     * Sets the scrollbar values.  'bar' can be either SB_VERT or
     * SB_HORZ.  'min', 'value', and 'max' can have the value INT_MAX
     * which means that the value should not be changed. 
     */
    void SetScrollValues(UINT bar, int min, int value, int max);

    INLINE LRESULT SendMessage(UINT msg, WPARAM wParam=0, LPARAM lParam=0) {
        DASSERT(GetHWnd());
        return ::SendMessage(GetHWnd(), msg, wParam, lParam);
    }
    INLINE virtual LONG GetStyle() {
        DASSERT(GetHWnd());
        return ::GetWindowLong(GetHWnd(), GWL_STYLE);
    }
    INLINE virtual void SetStyle(LONG style) {
        DASSERT(GetHWnd());
        // SetWindowLong() error handling as recommended by Win32 API doc.
        ::SetLastError(0);
        DWORD ret = ::SetWindowLong(GetHWnd(), GWL_STYLE, style);
        DASSERT(ret != 0 || ::GetLastError() == 0);
    }
    INLINE virtual LONG GetStyleEx() {
        DASSERT(GetHWnd());
        return ::GetWindowLong(GetHWnd(), GWL_EXSTYLE);
    }
    INLINE virtual void SetStyleEx(LONG style) {
        DASSERT(GetHWnd());
        // SetWindowLong() error handling as recommended by Win32 API doc.
        ::SetLastError(0);
        DWORD ret = ::SetWindowLong(GetHWnd(), GWL_EXSTYLE, style);
        DASSERT(ret != 0 || ::GetLastError() == 0);
    }

    virtual BOOL NeedDblClick() { return FALSE; }

    /* for multifont component */
    static void DrawWindowText(HDC hDC, jobject font, jstring text,
			       int x, int y);
    static void DrawGrayText(HDC hDC, jobject font, jstring text,
			     int x, int y);

    void DrawListItem(JNIEnv *env, DRAWITEMSTRUCT &drawInfo);

    void MeasureListItem(JNIEnv *env, MEASUREITEMSTRUCT &measureInfo);

    jstring GetItemString(JNIEnv *env, jobject target, jint index);

    jint GetFontHeight(JNIEnv *env);

    virtual jobject PreferredItemSize(JNIEnv *env) {DASSERT(FALSE); return NULL; }

    INLINE BOOL isEnabled() {
	JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
	if (env->EnsureLocalCapacity(2) < 0) {
	    return NULL;
	}
	jobject self = GetPeer(env);
	jobject target = env->GetObjectField(self, AwtObject::targetID);
	BOOL e = env->CallBooleanMethod(target, AwtComponent::isEnabledMID);
        DASSERT(!safe_ExceptionOccurred(env));
	
	env->DeleteLocalRef(target);

        return e;
    }

    void SendKeyEventToFocusOwner(jint id, jlong when, jint raw, jint cooked,
				  jint modifiers, jint keyLocation,
				  MSG *msg = NULL);
    /*
     * Allocate and initialize a new java.awt.event.KeyEvent, and  
     * post it to the peer's target object.  No response is expected 
     * from the target.
     */
    void SendKeyEvent(jint id, jlong when, jint raw, jint cooked,
		      jint modifiers, jint keyLocation,
		      MSG *msg = NULL);

    /*
     * Allocate and initialize a new java.awt.event.MouseEvent, and 
     * post it to the peer's target object.  No response is expected 
     * from the target.
     */
    void SendMouseEvent(jint id, jlong when, jint x, jint y, 
                        jint modifiers, jint clickCount,
			jboolean popupTrigger, jint button = 0,
			MSG *msg = NULL);

    /*
     * Allocate and initialize a new java.awt.event.MouseWheelEvent, and 
     * post it to the peer's target object.  No response is expected 
     * from the target.
     */
    void SendMouseWheelEvent(jint id, jlong when, jint x, jint y, 
			     jint modifiers, jint clickCount,
			     jboolean popupTrigger, jint scrollType, 
			     jint scrollAmount, jint wheelRotation, 
			     MSG *msg = NULL);

    /*
     * Allocate and initialize a new java.awt.event.FocusEvent, and 
     * post it to the peer's target object.  No response is expected 
     * from the target.
     */
    void SendFocusEvent(jint id, HWND opposite);

    /* Forward a filtered event directly to the subclassed window.
       synthetic should be TRUE iff the message was generated because
       of a synthetic Java event, rather than a native event. */
    virtual MsgRouting HandleEvent(MSG *msg, BOOL synthetic);

    /* Event->message synthesizer methods. */
    void SynthesizeKeyMessage(JNIEnv *env, jobject keyEvent);
    void SynthesizeMouseMessage(JNIEnv *env, jobject mouseEvent);

    /* Components which inherit native mouse wheel behavior will
     * return TRUE.  These are TextArea, Choice, FileDialog, and
     * List.  All other Components return FALSE.
     */ 
    virtual BOOL InheritsNativeMouseWheelBehavior();

    /* Functions for MouseWheel support on Windows95
     * These should only be called if running on 95
     */
    static void Wheel95Init();
    INLINE static UINT Wheel95GetMsg() {return sm_95WheelMessage;}
    static UINT Wheel95GetScrLines();

    /* Determines whether the component is obscured by another window */
    jboolean IsObscured();

    /* Invalidate the specified rectangle. */
    virtual void Invalidate(RECT* r);

    /* Begin and end deferred window positioning. */
    virtual void BeginValidate();
    virtual void EndValidate();

    /* Keyboard conversion routines. */
    static void InitDynamicKeyMapTable();
    static void BuildDynamicKeyMapTable();
    static jint GetJavaModifiers();
    static jint GetButton(int mouseButton);
    static UINT WindowsKeyToJavaKey(UINT windowsKey, UINT modifiers);
    static void JavaKeyToWindowsKey(UINT javaKey, UINT *windowsKey, UINT *modifiers, UINT originalWindowsKey);

    INLINE static void AwtComponent::JavaKeyToWindowsKey(UINT javaKey,
				       UINT *windowsKey, UINT *modifiers)
    {
        JavaKeyToWindowsKey(javaKey, windowsKey, modifiers, IGNORE_KEY);
    }

    enum TransOps {NONE, LOAD, SAVE};
    
    UINT WindowsKeyToJavaChar(UINT wkey, UINT modifiers, TransOps ops);

    /* routines used for input method support */
    void SetInputMethod(jobject im, BOOL useNativeCompWindow);
    static int GetContextData(HIMC hIMC, DWORD dwIndex, LPVOID* lplpData);
    static jstring MakeJavaString(LPWSTR lpStrW, int cStrW);
    static int MakeClauseInformation(int cStrW,
				     int cClauseW, DWORD *lpClauseW, 
				     int cReadStrW, LPWSTR lpReadStrW, 
				     int cReadClauseW, DWORD *lpReadClauseW, 
				     int** lpBndClauseW, jstring** lpReadingClauseW );
    static int MakeAttributeInformation(int cStrW,
					int cAttrW, BYTE* lpAttrW, 
					int** lpBndAttrW, BYTE** lpValAttrW );
    void SendInputMethodEvent(jint id, jstring text, int cClause,
			      int *rgClauseBoundary, jstring *rgClauseReading,
			      int cAttrBlock, int *rgAttrBoundary,
			      BYTE *rgAttrValue, int commitedTextLength,
			      int caretPos, int visiblePos);
    void InquireCandidatePosition();
    HIMC ImmGetContext();
    HIMC ImmAssociateContext(HIMC himc);
    HWND GetProxyFocusOwner();
    void CallProxyDefWindowProc(UINT message, 
                                WPARAM wParam, 
                                LPARAM lParam, 
                                LRESULT &retVal,
                                MsgRouting &mr);

    /*
     * Windows message handler functions
     */
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

    /* return true if msg is processed */
    virtual MsgRouting PreProcessMsg(MSG& msg); 

    virtual MsgRouting WmCreate() {return mrDoDefault;}
    virtual MsgRouting WmClose() {return mrDoDefault;}
    virtual MsgRouting WmDestroy();

    virtual MsgRouting WmActivate(UINT nState, BOOL fMinimized, HWND opposite) 
    {
	return mrDoDefault;
    }

    virtual MsgRouting WmEraseBkgnd(HDC hDC, BOOL& didErase) 
    {
	return mrDoDefault;
    }

    virtual MsgRouting WmPaint(HDC hDC);
    virtual MsgRouting WmGetMinMaxInfo(LPMINMAXINFO lpmmi);
    virtual MsgRouting WmMove(int x, int y);
    virtual MsgRouting WmSize(UINT type, int w, int h);
    virtual MsgRouting WmSizing();
    virtual MsgRouting WmShowWindow(BOOL show, UINT status);
    virtual MsgRouting WmSetFocus(HWND hWndLost);
    virtual MsgRouting WmKillFocus(HWND hWndGot);
    void WmComponentSetFocus(WmComponentSetFocusData *data);
    // Use instead of ::SetFocus to maintain special focusing semantics for
    // Windows which are not Frames/Dialogs.
    BOOL AwtSetFocus();

    virtual MsgRouting WmCtlColor(HDC hDC, HWND hCtrl, 
				  UINT ctlColor, HBRUSH& retBrush);
    virtual MsgRouting WmHScroll(UINT scrollCode, UINT pos, HWND hScrollBar);
    virtual MsgRouting WmVScroll(UINT scrollCode, UINT pos, HWND hScrollBar);
    
    virtual MsgRouting WmMouseEnter(UINT flags, int x, int y);
    virtual MsgRouting WmMouseDown(UINT flags, int x, int y, int button);
    virtual MsgRouting WmMouseUp(UINT flags, int x, int y, int button);
    virtual MsgRouting WmMouseMove(UINT flags, int x, int y);
    virtual MsgRouting WmMouseExit(UINT flags, int x, int y);
    virtual MsgRouting WmMouseWheel(UINT flags, int x, int y,
                                    int wheelRotation);
    virtual MsgRouting WmNcMouseDown(WPARAM hitTest, int x, int y, int button);
    virtual MsgRouting WmNcMouseUp(int x, int y, int button);

    // NB: 64-bit: vkey is wParam of the message, but other API's take
    // vkey parameters of type UINT, so we do the cast before dispatching.
    virtual MsgRouting WmKeyDown(UINT vkey, UINT repCnt, UINT flags, BOOL system);
    virtual MsgRouting WmKeyUp(UINT vkey, UINT repCnt, UINT flags, BOOL system);

    virtual MsgRouting WmChar(UINT character, UINT repCnt, UINT flags, BOOL system);
    virtual MsgRouting WmIMEChar(UINT character, UINT repCnt, UINT flags, BOOL system);
    virtual MsgRouting WmInputLangChange(UINT charset, HKL hKeyBoardLayout);
    virtual MsgRouting WmForwardChar(WCHAR character, LPARAM lParam,
				     BOOL synthethic);
    virtual MsgRouting WmPaste();

    virtual void SetCompositionWindow();
    virtual void OpenCandidateWindow(int x, int y);
    virtual void SetCandidateWindow(int iCandType, int x, int y);
    virtual MsgRouting WmImeSetContext(BOOL fSet, LPARAM *lplParam);
    virtual MsgRouting WmImeNotify(WPARAM subMsg, LPARAM bitsCandType);
    virtual MsgRouting WmImeStartComposition();
    virtual MsgRouting WmImeEndComposition();
    virtual MsgRouting WmImeComposition(WORD wChar, LPARAM flags);

    virtual MsgRouting WmTimer(UINT_PTR timerID) {return mrDoDefault;}

    virtual MsgRouting WmCommand(UINT id, HWND hWndCtrl, UINT notifyCode);
    /* reflected WmCommand from parent */
    virtual MsgRouting WmNotify(UINT notifyCode);  

    virtual MsgRouting WmCompareItem(UINT /*ctrlId*/, 
                                     COMPAREITEMSTRUCT &compareInfo, 
                                     LRESULT &result);
    virtual MsgRouting WmDeleteItem(UINT /*ctrlId*/, 
                                    DELETEITEMSTRUCT &deleteInfo);
    virtual MsgRouting WmDrawItem(UINT ctrlId,
				  DRAWITEMSTRUCT &drawInfo);
    virtual MsgRouting WmMeasureItem(UINT ctrlId,
				     MEASUREITEMSTRUCT &measureInfo);
    /* Fix 4181790 & 4223341 : These functions get overridden in owner-drawn
     * components instead of the Wm... versions.
     */
    virtual MsgRouting OwnerDrawItem(UINT ctrlId,
				     DRAWITEMSTRUCT &drawInfo);
    virtual MsgRouting OwnerMeasureItem(UINT ctrlId,
					MEASUREITEMSTRUCT &measureInfo);

    virtual MsgRouting WmPrint(HDC hDC, LPARAM flags);
    virtual MsgRouting WmPrintClient(HDC hDC, LPARAM flags);

    virtual MsgRouting WmNcCalcSize(BOOL fCalcValidRects, 
				    LPNCCALCSIZE_PARAMS lpncsp,
				    LRESULT &retVal);
    virtual MsgRouting WmNcPaint(HRGN hrgn);
    virtual MsgRouting WmNcHitTest(UINT x, UINT y, LRESULT &retVal);
    virtual MsgRouting WmSysCommand(UINT uCmdType, int xPos, int yPos);
    virtual MsgRouting WmExitSizeMove();
    virtual MsgRouting WmEnterMenuLoop(BOOL isTrackPopupMenu);
    virtual MsgRouting WmExitMenuLoop(BOOL isTrackPopupMenu);

    virtual MsgRouting WmQueryNewPalette(LRESULT &retVal);
    virtual MsgRouting WmPaletteChanged(HWND hwndPalChg);
    virtual MsgRouting WmPaletteIsChanging(HWND hwndPalChg);
    virtual MsgRouting WmStyleChanged(int wStyleType, LPSTYLESTRUCT lpss);
    virtual MsgRouting WmSettingChange(UINT wFlag, LPCTSTR pszSection);

    virtual MsgRouting WmContextMenu(HWND hCtrl, UINT xPos, UINT yPos) {
        return mrDoDefault;
    }

    void UpdateColorModel();

    jintArray CreatePrintedPixels(SIZE &loc, SIZE &size);

    virtual BOOL WmDDCreateSurface(Win32SDOps* wsdo);
    virtual MsgRouting WmDDEnterFullScreen(HMONITOR monitor);
    virtual MsgRouting WmDDExitFullScreen(HMONITOR monitor);
    virtual MsgRouting WmDDSetDisplayMode(HMONITOR monitor, DDrawDisplayMode* pDisplayMode);

    static void * GetNativeFocusOwner();
    static void * GetNativeFocusedWindow();
    static void   ClearGlobalFocusOwner(jobject activeWindow);

    /*
     * HWND, AwtComponent and Java Peer interaction 
     *
     * Link the C++, Java peer, and HWNDs together. 
     */
    void LinkObjects(JNIEnv *env, jobject peer);

    void UnlinkObjects();

    static BOOL QueryNewPaletteCalled() { return m_QueryNewPaletteCalled; }

#ifdef DEBUG
    virtual void VerifyState(); /* verify component and peer are in sync. */
#else
    void VerifyState() {}       /* no-op */
#endif

    virtual AwtDropTarget* CreateDropTarget(JNIEnv* env);
    virtual void DestroyDropTarget();

    INLINE virtual HWND GetDBCSEditHandle() { return NULL; }
    // State for native drawing API
    INLINE jint GetDrawState() { return GetDrawState(m_hwnd); }
    INLINE void SetDrawState(jint state) { SetDrawState(m_hwnd, state); }    // State for native drawing API

    INLINE virtual BOOL IsTopLevel() { return FALSE; }
    INLINE virtual BOOL IsEmbeddedFrame() { return FALSE; }
    INLINE virtual BOOL IsScrollbar() { return FALSE; }

    static INLINE BOOL IsTopLevelHWnd(HWND hwnd) {
        AwtComponent *comp = AwtComponent::GetComponent(hwnd);
        return (comp != NULL && comp->IsTopLevel());
    }
    static INLINE BOOL IsEmbeddedFrameHWnd(HWND hwnd) {
        AwtComponent *comp = AwtComponent::GetComponent(hwnd);
        return (comp != NULL && comp->IsEmbeddedFrame());  
    }

    static jint GetDrawState(HWND hwnd);
    static void SetDrawState(HWND hwnd, jint state);
    
    static HWND GetHWnd(JNIEnv* env, jobject target);

    static MSG* CreateMessage(UINT message, WPARAM wParam, LPARAM lParam, int x, int y);
    static void InitMessage(MSG* msg, UINT message, WPARAM wParam, LPARAM lParam, int x, int y);

    static HWND sm_focusOwner;
    static HWND sm_focusedWindow;

    static BOOL m_isWin95;  
    static BOOL m_isWin2000;  
    static BOOL m_isWinNT;  

protected:
    static AwtComponent* GetComponentImpl(HWND hWnd);

    HWND     m_hwnd;
    UINT     m_myControlID;	/* its own ID from the view point of parent */
    BOOL     m_backgroundColorSet;
    BOOL     m_dragged;         /* in drag operation */

    static BOOL sm_suppressFocusAndActivation;
    static HWND sm_realFocusOpposite;
    
    virtual INLINE HWND GetWrappeeHandle() { return NULL; }
    virtual void SetDragCapture(UINT flags);
    virtual void ReleaseDragCapture(UINT flags);

    // 95 support for mouse wheel
    static UINT sm_95WheelMessage;
    static UINT sm_95WheelSupport;

private:
    
    COLORREF m_colorForeground;
    COLORREF m_colorBackground;

    AwtPen*  m_penForeground;
    AwtBrush* m_brushBackground;

    WNDPROC  m_DefWindowProc;

    UINT     m_nextControlID;   /* provides a unique ID for child controls */

    CriticalSection m_lock;

    // DeferWindowPos handle for batched-up window positioning
    HDWP     m_hdwp;
    // Counter to handle nested calls to Begin/EndValidate
    UINT     m_validationNestCount;

    AwtDropTarget* m_dropTarget; // associated DropTarget object

    // When we process WM_INPUTLANGCHANGE we remember the keyboard
    // layout handle and associated input language and codepage.
    // We also invalidate VK translation table for VK_OEM_* codes
    static HKL    m_hkl;
    static UINT   m_CodePage;
    static LANGID m_idLang;

    static BOOL sm_rtl;
    static BOOL sm_rtlReadingOrder;

    jobject m_InputMethod;
    BOOL    m_useNativeCompWindow;
    LPARAM  m_bitsCandType;
    UINT    m_PendingLeadByte;

    void SetComponentInHWND();

    // Determines whether a given virtual key is on the numpad
    static BOOL IsNumPadKey(UINT vkey, BOOL extended);
  
    // Determines the keyLocation of a given key
    static jint GetKeyLocation(UINT wkey, UINT flags); 
    static jint GetShiftKeyLocation(UINT wkey, UINT flags); 

    // Cache for FindComponent
    static HWND sm_cursorOn;

    static BOOL m_QueryNewPaletteCalled;

    BOOL m_skipNextSetFocus;

    int windowMoveLockPosX;
    int windowMoveLockPosY;
    int windowMoveLockPosCX;
    int windowMoveLockPosCY;

private:
    /*
     * The association list of children's IDs and corresponding components. 
     * Some components like Choice or List are required their sizes while
     * the creations of themselfs are in progress.
     */
    class ChildListItem {
    public:
        ChildListItem(UINT id, AwtComponent* component) {
            m_ID = id;
            m_Component = component;
            m_next = NULL;
        }
        ~ChildListItem() {
            if (m_next != NULL)
                delete m_next;
        }

        UINT m_ID;
        AwtComponent* m_Component;
        ChildListItem* m_next;
    };

public:
    INLINE void PushChild(UINT id, AwtComponent* component) {
        ChildListItem* child = new ChildListItem(id, component);
        child->m_next = m_childList;
        m_childList = child;
    }
    
    static void SetParent(void * param);
private:
    AwtComponent* SearchChild(UINT id);
    void RemoveChild(UINT id) ;

    ChildListItem* m_childList;
};

// DC management objects; these classes are used to track the list of
// DC's associated with a given Component.  Then DC's can be released
// appropriately on demand or on window destruction to avoid resource
// leakage.
class DCItem {
public:
    HDC		    hDC;
    HWND	    hWnd;
    DCItem	    *next;
};
class DCList {
    DCItem	    *head;
    CriticalSection listLock;
public:
    DCList() { head = NULL; }

    void	    AddDC(HDC hDC, HWND hWnd);
    void	    AddDCItem(DCItem *newItem);
    DCItem	    *RemoveDC(HDC hDC);
    DCItem	    *RemoveAllDCs(HWND hWnd);
    void	    RealizePalettes(int screen);
};

struct WmComponentSetFocusData {
    jobject lightweightChild;
    jboolean temporary;
    jboolean focusedWindowChangeAllowed;
    jlong time;

    jboolean success;
};

void ReleaseDCList(HWND hwnd, DCList &list);
void MoveDCToPassiveList(HDC hDC);

jlong nowMillisUTC();
jlong nowMillisUTC(DWORD event_offset);

#include "ObjectList.h"

#endif /* AWT_COMPONENT_H */
