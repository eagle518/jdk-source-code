/*
 * @(#)awt_Toolkit.h	1.77 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * The Toolkit class has two functions: it instantiates the AWT 
 * ToolkitPeer's native methods, and provides the DLL's core functions.
 *
 * There are two ways this DLL can be used: either as a dynamically-
 * loaded Java native library from the interpreter, or by a Windows-
 * specific app.  The first manner requires that the Toolkit provide
 * all support needed so the app can function as a first-class Windows
 * app, while the second assumes that the app will provide that 
 * functionality.  Which mode this DLL functions in is determined by
 * which initialization paradigm is used. If the Toolkit is constructed
 * normally, then the Toolkit will have its own pump. If it is explicitly
 * initialized for an embedded environment (via a static method on
 * sun.awt.windows.WToolkit), then it will rely on an external message
 * pump.
 *
 * The most basic functionality needed is a Windows message pump (also
 * known as a message loop).  When an Java app is started as a console
 * app by the interpreter, the Toolkit needs to provide that message
 * pump if the AWT is dynamically loaded.  
 */

#ifndef AWT_TOOLKIT_H
#define AWT_TOOLKIT_H

#include "awt.h"
#include "awtmsg.h"
#include "awt_Multimon.h"
#include "Trace.h"

#include "sun_awt_windows_WToolkit.h"

class AwtObject;
class AwtDialog;
class AwtDropTarget;

typedef VOID (CALLBACK* IDLEPROC)(VOID);
typedef BOOL (CALLBACK* PEEKMESSAGEPROC)(MSG&);

/*
 * class JNILocalFrame
 * Push/PopLocalFrame helper
 */
class JNILocalFrame {
  public:
    INLINE JNILocalFrame(JNIEnv *env, int size) {
        m_env = env;
        int result = m_env->PushLocalFrame(size);
        if (result < 0) {
            DASSERT(FALSE);
            JNU_ThrowOutOfMemoryError(m_env, "Can't allocate localRefs");
        }
    }
    INLINE ~JNILocalFrame() { m_env->PopLocalFrame(NULL); }
  private:
    JNIEnv* m_env;
};

/*
 * class CriticalSection
 * ~~~~~ ~~~~~~~~~~~~~~~~
 * Lightweight intra-process thread synchronization. Can only be used with
 * other critical sections, and only within the same process.
 */
class CriticalSection {
  public:
    INLINE  CriticalSection() { ::InitializeCriticalSection(&rep); }
    INLINE ~CriticalSection() { ::DeleteCriticalSection(&rep); }

    class Lock {
      public:
        INLINE Lock(const CriticalSection& cs) : critSec(cs) {
            ::EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&critSec.rep));
        }
       INLINE ~Lock() {
            ::LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&critSec.rep));
        }
      private:
        const CriticalSection& critSec;
    };
    friend Lock;

  private:
    CRITICAL_SECTION rep;

    CriticalSection(const CriticalSection&);
    const CriticalSection& operator =(const CriticalSection&);

  public:
    virtual void    Enter           (void)
    {
        ::EnterCriticalSection(&rep);
    }
    virtual void    Leave           (void)
    {
        ::LeaveCriticalSection(&rep);
    }
};

// Macros for using CriticalSection objects that help trace
// lock/unlock actions
#define CRITICAL_SECTION_ENTER(cs) { \
    J2dTraceLn4(J2D_TRACE_VERBOSE, \
		"CS.Wait:  tid, cs, file, line = 0x%x, 0x%x, %s, %d", \
	    	GetCurrentThreadId(), &(cs), __FILE__, __LINE__); \
    (cs).Enter(); \
    J2dTraceLn4(J2D_TRACE_VERBOSE, \
		"CS.Enter: tid, cs, file, line = 0x%x, 0x%x, %s, %d", \
	    	GetCurrentThreadId(), &(cs), __FILE__, __LINE__); \
}

#define CRITICAL_SECTION_LEAVE(cs) { \
    J2dTraceLn4(J2D_TRACE_VERBOSE, \
		"CS.Leave: tid, cs, file, line = 0x%x, 0x%x, %s, %d", \
	    	GetCurrentThreadId(), &(cs), __FILE__, __LINE__); \
    (cs).Leave(); \
    J2dTraceLn4(J2D_TRACE_VERBOSE, \
		"CS.Left:  tid, cs, file, line = 0x%x, 0x%x, %s, %d", \
	    	GetCurrentThreadId(), &(cs), __FILE__, __LINE__); \
}

/************************************************************************
 * AwtToolkit class
 */

class AwtToolkit {
public:
    enum {
	KB_STATE_SIZE = 256
    };

    /* java.awt.Toolkit method ids */
    static jmethodID getDefaultToolkitMID;
    static jmethodID getFontMetricsMID;
	static jmethodID insetsMID;

    /* sun.awt.windows.WToolkit ids */
    static jmethodID windowsSettingChangeMID;
    static jmethodID displayChangeMID;

    BOOL m_isDynamicLayoutSet;

    AwtToolkit();
    ~AwtToolkit();

    BOOL Initialize(BOOL localPump);
    BOOL Dispose();

    void SetDynamicLayout(BOOL dynamic);
    BOOL IsDynamicLayoutSet();
    BOOL IsDynamicLayoutSupported();
    BOOL IsDynamicLayoutActive();

    static void PrintWinVersion(); 

    INLINE BOOL localPump() { return m_localPump; }
    INLINE BOOL VerifyComponents() { return FALSE; } // TODO: Use new DebugHelper class to set this flag
    INLINE HWND GetHWnd() { return m_toolkitHWnd; }

    INLINE HMODULE GetModuleHandle() { return m_dllHandle; }
    INLINE void SetModuleHandle(HMODULE h) { m_dllHandle = h; }

    INLINE static DWORD MainThread() { return GetInstance().m_mainThreadId; }
    INLINE void VerifyActive() throw (awt_toolkit_shutdown) {
        if (!m_isActive && m_mainThreadId != ::GetCurrentThreadId()) {
	    throw awt_toolkit_shutdown();
	}
    }
    INLINE BOOL IsDisposed() { return m_isDisposed; }
    static UINT GetMouseKeyState();
    static void GetKeyboardState(PBYTE keyboardState);

    static ATOM RegisterClass();
    static void UnregisterClass();
    INLINE LRESULT SendMessage(UINT msg, WPARAM wParam=0, LPARAM lParam=0) {
	return ::SendMessage(GetHWnd(), msg, wParam, lParam);
    }
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, 
                                    LPARAM lParam);
    static LRESULT CALLBACK GetMessageFilter(int code, WPARAM wParam, 
                                             LPARAM lParam);
    static LRESULT CALLBACK ForegroundIdleFilter(int code, WPARAM wParam, 
                                                 LPARAM lParam);

    INLINE static AwtToolkit& GetInstance() { return theInstance; }
    INLINE void SetPeer(JNIEnv *env, jobject wToolkit) { 
        AwtToolkit &tk = AwtToolkit::GetInstance();
	if (tk.m_peer != NULL) {
	    env->DeleteGlobalRef(tk.m_peer);
	}
	tk.m_peer = (wToolkit != NULL) ? env->NewGlobalRef(wToolkit) : NULL;
    }

    // is this thread the main thread?

    INLINE static BOOL IsMainThread() {
	return GetInstance().m_mainThreadId == ::GetCurrentThreadId();
    }

    // post a message to the message pump thread

    INLINE BOOL PostMessage(UINT msg, WPARAM wp=0, LPARAM lp=0) {
        return ::PostMessage(GetHWnd(), msg, wp, lp);
    }
 
    // cause the message pump thread to call the function synchronously now!

    INLINE void * InvokeFunction(void*(*ftn)(void)) {
	return (void *)SendMessage(WM_AWT_INVOKE_VOID_METHOD, (WPARAM)ftn, 0);
    }
    INLINE void InvokeFunction(void (*ftn)(void)) {
        InvokeFunction((void*(*)(void))ftn);
    }
    INLINE void * InvokeFunction(void*(*ftn)(void *), void* param) {
	return (void *)SendMessage(WM_AWT_INVOKE_METHOD, (WPARAM)ftn,
				   (LPARAM)param);
    }
    INLINE void InvokeFunction(void (*ftn)(void *), void* param) {
        InvokeFunction((void*(*)(void*))ftn, param);
    }

    // cause the message pump thread to call the function later ...

    INLINE void InvokeFunctionLater(void (*ftn)(void *), void* param) {
        if (!PostMessage(WM_AWT_INVOKE_METHOD, (WPARAM)ftn, (LPARAM)param)) {
            JNIEnv* env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
            JNU_ThrowInternalError(env, "Message not posted, native event queue may be full.");
        }
    }

   // cause the message pump thread to synchronously synchronize on the handle

    INLINE void WaitForSingleObject(HANDLE handle) {
	SendMessage(WM_AWT_WAIT_FOR_SINGLE_OBJECT, 0, (LPARAM)handle);
    }

    /*
     * Create an AwtXxxx C++ component using a given factory
     */
    typedef void (*ComponentFactory)(void*, void*);
    static void CreateComponent(void* hComponent, void* hParent, 
                                ComponentFactory compFactory, BOOL isParentALocalReference=TRUE);

    static void DestroyComponent(class AwtComponent* comp);

    // constants used to PostQuitMessage

    static const int EXIT_ENCLOSING_LOOP;
    static const int EXIT_ALL_ENCLOSING_LOOPS;

    // ...

    void QuitMessageLoop(int status);

    UINT MessageLoop(IDLEPROC lpIdleFunc, PEEKMESSAGEPROC lpPeekMessageFunc);
    BOOL PumpWaitingMessages(PEEKMESSAGEPROC lpPeekMessageFunc);
    BOOL PreProcessMsg(MSG& msg);
    BOOL PreProcessMouseMsg(class AwtComponent* p, MSG& msg);
    BOOL PreProcessKeyMsg(class AwtComponent* p, MSG& msg);

    /* Create an ID which maps to an AwtObject pointer, such as a menu. */
    UINT CreateCmdID(AwtObject* object);

    // removes cmd id mapping
    void RemoveCmdID(UINT id);

    /* Return the AwtObject associated with its ID. */
    AwtObject* LookupCmdID(UINT id);

    /* Return the current application icon. */
    HICON GetAwtIcon();

    /* Turns on/off dialog modality for the system. */
    INLINE AwtDialog* SetModal(AwtDialog* frame) { 
	AwtDialog* previousDialog = m_pModalDialog;
	m_pModalDialog = frame;
	return previousDialog;
    };
    INLINE void ResetModal(AwtDialog* oldFrame) { m_pModalDialog = oldFrame; };
    INLINE BOOL IsModal() { return (m_pModalDialog != NULL); };
    INLINE AwtDialog* GetModalDialog(void) { return m_pModalDialog; };

    /* Stops the current message pump (normally a modal dialog pump) */
    INLINE void StopMessagePump() { m_breakOnError = TRUE; }

    /* Debug settings */
    INLINE void SetVerbose(long flag)   { m_verbose = (flag != 0); }
    INLINE void SetVerify(long flag)    { m_verifyComponents = (flag != 0); }
    INLINE void SetBreak(long flag)     { m_breakOnError = (flag != 0); }
    INLINE void SetHeapCheck(long flag);

    static void SetBusy(BOOL busy);

    static VOID CALLBACK PrimaryIdleFunc();
    static VOID CALLBACK SecondaryIdleFunc();
    static BOOL CALLBACK CommonPeekMessageFunc(MSG& msg);
    static BOOL activateKeyboardLayout(HKL hkl);
private:
    HWND CreateToolkitWnd(LPCTSTR name);

    BOOL m_localPump;
    DWORD m_mainThreadId;
    HWND m_toolkitHWnd;
    BOOL m_verbose;
    BOOL m_isActive; // set to FALSE at beginning of Dispose
    BOOL m_isDisposed; // set to TRUE at end of Dispose

    BOOL m_vmSignalled; // set to TRUE if QUERYENDSESSION has successfully
                        // raised SIGTERM

    BOOL m_verifyComponents;
    BOOL m_breakOnError;

    BOOL  m_breakMessageLoop;
    UINT  m_messageLoopResult;

    class AwtComponent* m_lastMouseOver;
    BOOL                m_mouseDown;

    HHOOK m_hGetMessageHook;
    UINT_PTR  m_timer;

    class AwtCmdIDList* m_cmdIDs;
    BYTE		m_lastKeyboardState[KB_STATE_SIZE];
    CriticalSection	m_lockKB;

    static AwtToolkit theInstance;

    /* The current modal dialog frame (normally NULL). */
    AwtDialog* m_pModalDialog;

    /* The WToolkit peer instance */
    jobject m_peer;

    HMODULE m_dllHandle;  /* The module handle. */

/* track display changes - used by palette-updating code.
   This is a workaround for a windows bug that prevents
   WM_PALETTECHANGED event from occurring immediately after
   a WM_DISPLAYCHANGED event.
  */
private:
    BOOL m_displayChanged;  /* Tracks displayChanged events */

public:
    BOOL HasDisplayChanged() { return m_displayChanged; }
    void ResetDisplayChanged() { m_displayChanged = FALSE; }


/* cursor support
 */
 private:
    BOOL m_nCustomCursor;
    BOOL m_nHasThemes;

    void _begin_cursors();
    void _end_cursors();
    void _detect_themes();

 public:
    void NotifyCustomCursor() { m_nCustomCursor = TRUE; }
    void NotifySystemCursor() { m_nCustomCursor = FALSE; }
    void UpdateThemesStatus() { _detect_themes(); }
    BOOL IsCustomCursor() { return m_nCustomCursor; }
    BOOL AreThemesInstalled() { return m_nHasThemes; }
    
 private:
    static JNIEnv *m_env;
    static HANDLE m_thread;
 public:
    static void SetEnv(JNIEnv *env);
    static JNIEnv* GetEnv();    
};

/*
 * Class to encapsulate the extraction of the java string contents
 * into a buffer and the cleanup of the buffer 
 */
class JavaStringBuffer {
  public:
    JavaStringBuffer(JNIEnv *env, jstring jstr);
    INLINE ~JavaStringBuffer() { delete[] buffer; }
    INLINE operator LPTSTR() { return buffer; }
    INLINE operator LPARAM() { return (LPARAM)buffer; }  /* for SendMessage */

  private:
    LPTSTR buffer;
};
#endif /* AWT_TOOLKIT_H */
