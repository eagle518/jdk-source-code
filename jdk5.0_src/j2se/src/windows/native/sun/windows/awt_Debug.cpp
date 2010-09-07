/*
 * @(#)awt_Debug.cpp	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "awt.h"
#include "awt_Toolkit.h"
#include "debug_mem.h"

extern void DumpJavaStack();

#if defined(DEBUG)

////////////////////////////////////////////////////////////////////////////////////
// avoid pulling in our redefinition of 'new'
// since we actually need to implement it here
#if defined(new)
#undef new
#endif
//

void * operator new(size_t size, const char * filename, int linenumber) {
    void * ptr = DMem_AllocateBlock(size, filename, linenumber);
    if (ptr == NULL) {
        throw std::bad_alloc();
    }

    return ptr;
}

#if _MSC_VER >= 1200
void operator delete(void *ptr, const char*, int) {
    DASSERTMSG(FALSE, "This version of 'delete' should never get called!!!");
}
#endif
void operator delete(void *ptr) {
    DMem_FreeBlock(ptr);
}

////////////////////////////////////////////////////////////////////////////////////

static void DumpRegion(HRGN rgn) {    
    DWORD size = ::GetRegionData(rgn, 0, NULL);
    char* buffer = (char *)safe_Malloc(size);
    memset(buffer, 0, size);
    LPRGNDATA rgndata = (LPRGNDATA)buffer;
    rgndata->rdh.dwSize = sizeof(RGNDATAHEADER);
    rgndata->rdh.iType = RDH_RECTANGLES;
    VERIFY(::GetRegionData(rgn, size, rgndata));

    RECT* r = (RECT*)(buffer + rgndata->rdh.dwSize);
    for (DWORD i=0; i<rgndata->rdh.nCount; i++) {
	if ( !::IsRectEmpty(r) ) {
	    DTrace_PrintImpl("\trect %d %d %d %d\n", r->left, r->top, r->right, r->bottom);
	}
	r++;
    }

    free(buffer);
}

/*
 * DTRACE print callback to dump HDC clip region bounding rectangle
 */
void DumpClipRectangle(const char * file, int line, int argc, const char * fmt, va_list arglist) {
    const char *msg = va_arg(arglist, const char *);
    HDC		hdc = va_arg(arglist, HDC);
    RECT	r;
    
    DASSERT(argc == 2 && hdc != NULL);
    DASSERT(msg != NULL);

    ::GetClipBox(hdc, &r);
    DTrace_PrintImpl("%s: hdc=%x, %d %d %d %d\n", msg, hdc, r.left, r.top, r.right, r.bottom);
}

/*
 * DTRACE print callback to dump window's update region bounding rectangle
 */
void DumpUpdateRectangle(const char * file, int line, int argc, const char * fmt, va_list arglist) {
    const char *msg = va_arg(arglist, const char *);
    HWND	hwnd = va_arg(arglist, HWND);
    RECT	r;
    
    DASSERT(argc == 2 && ::IsWindow(hwnd));
    DASSERT(msg != NULL);

    ::GetUpdateRect(hwnd, &r, FALSE);
    HRGN rgn = ::CreateRectRgn(0,0,1,1);
    int updated = ::GetUpdateRgn(hwnd, rgn, FALSE);
    DTrace_PrintImpl("%s: hwnd=%x, %d %d %d %d\n", msg, hwnd, r.left, r.top, r.right, r.bottom);
    DumpRegion(rgn);
    DeleteObject(rgn);
}

//
// Declare a static object to init/fini the debug code
//
// specify that this static object will get constructed before 
// any other static objects (except CRT objects) so the debug
// code can be used anywhere during the lifetime of the AWT dll
#pragma warning( disable:4073 ) // disable warning about using init_seg(lib) in non-3rd party library code
#pragma init_seg( lib )

static volatile AwtDebugSupport	DebugSupport;
static int report_leaks = 0;

AwtDebugSupport::AwtDebugSupport() {
    DMem_Initialize();
    DTrace_Initialize();
    DAssert_SetCallback(AssertCallback);
}

AwtDebugSupport::~AwtDebugSupport() {
    if (report_leaks) {
        DMem_ReportLeaks();
    }
    DMem_Shutdown();
    DTrace_Shutdown();
}

void AwtDebugSupport::AssertCallback(const char * expr, const char * file, int line) {
    static const int ASSERT_MSG_SIZE = 1024;
    static const char * AssertFmt = 
	    "%s\r\n"
	    "File '%s', at line %d\r\n"
	    "GetLastError() is %x : %s\r\n"
	    "Do you want to break into the debugger?";

    static char	assertMsg[ASSERT_MSG_SIZE+1];
    DWORD   lastError = GetLastError();
    LPSTR	msgBuffer = NULL;
    int     ret;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_SYSTEM |
		  FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  lastError,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPSTR)&msgBuffer, // it's an output parameter when allocate buffer is used
		  0,
		  NULL);

    if (msgBuffer == NULL) {
	msgBuffer = "<Could not get GetLastError() message text>";
    }
    // format the assertion message
    _snprintf(assertMsg, ASSERT_MSG_SIZE, AssertFmt, expr, file, line, lastError, msgBuffer);
    LocalFree(msgBuffer);

    // tell the user the bad news
    fprintf(stderr, "*********************\n");
    fprintf(stderr, "AWT Assertion Failure\n");
    fprintf(stderr, "*********************\n");
    fprintf(stderr, "%s\n", assertMsg);
    fprintf(stderr, "*********************\n");
    ret = MessageBoxA(NULL, assertMsg, "AWT Assertion Failure",
	MB_YESNO|MB_ICONSTOP|MB_TASKMODAL);
    
    // if clicked Yes, break into the debugger
    if ( ret == IDYES ) {
	# if defined(_M_IX86)
	    _asm { int 3 };
	# else
	    DebugBreak();
	# endif
    }
    // otherwise, try to continue execution
}

void AwtDebugSupport::GenerateLeaksReport() {
    report_leaks = 1;
}

#endif // DEBUG
