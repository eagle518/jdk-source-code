/*
 * @(#)awt_Debug.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(_AWT_DEBUG_H)
#define _AWT_DEBUG_H

#include "debug_assert.h"
#include "debug_trace.h"

#if defined(DEBUG)
    #if defined(new)
    	#error new has already been defined!
    #endif
    class AwtDebugSupport {
	public:
	    AwtDebugSupport();
	    ~AwtDebugSupport();

	    static void AssertCallback(const char * expr, const char * file,
				       int line);
	    /* This method signals that the VM is exiting cleanly, and thus the
	       the debug memory manager should dump a leaks report when the
	       VM has finished exiting. This method should not be called for
	       termination exits (such as <CTRL>-C) */
	    static void GenerateLeaksReport();
    };

    extern void * operator new(size_t size, const char * filename, int linenumber);
#if _MSC_VER >= 1200
    /* VC 6.0 is more strict about enforcing matching placement new & delete */
    extern void operator delete(void *ptr, const char*, int);
#endif
    extern void operator delete(void *ptr);
    extern void DumpClipRectangle(const char * file, int line, int argc, const char * fmt, va_list arglist);
    extern void DumpUpdateRectangle(const char * file, int line, int argc, const char * fmt, va_list arglist);

    #define AWT_DUMP_UPDATE_RECTANGLE(_msg, _hwnd) \
	_DTrace_Template(DumpUpdateRectangle, 2, "", (_msg), (_hwnd), 0, 0, 0, 0, 0, 0)

    #define AWT_DUMP_CLIP_RECTANGLE(_msg, _hwnd) \
	_DTrace_Template(DumpClipRectangle, 2, "", (_msg), (_hwnd), 0, 0, 0, 0, 0, 0)

    #define new		new(__FILE__, __LINE__)
    
    #define VERIFY(exp)		DASSERT(exp)
    #define UNIMPLEMENTED()	DASSERT(FALSE)

    /* Disable inlining. */
    #define INLINE
#else
    #define AWT_DUMP_UPDATE_RECTANGLE(_msg, _hwnd) ((void)0)
    #define AWT_DUMP_CLIP_RECTANGLE(_msg, _hwnd) ((void)0)

    #define UNIMPLEMENTED() \
    	SignalError(0, JAVAPKG "NullPointerException","unimplemented");

    /*
    * VERIFY macro -- assertion where expression is always evaluated 
    * (normally used for BOOL functions).
    */
    #define VERIFY(exp) ((void)(exp))
    
    /* Enable inlining. */
    #define INLINE inline
#endif // DEBUG

#endif // _AWT_DEBUG_H
