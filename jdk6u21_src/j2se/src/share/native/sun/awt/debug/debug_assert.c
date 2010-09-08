/*
 * @(#)debug_assert.c	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if defined(DEBUG)

#include "debug_util.h"

enum {
    MAX_ASSERT_MSG = 255+FILENAME_MAX+1
};
    
static DASSERT_CALLBACK	PfnAssertCallback = NULL;

void DAssert_Impl(const char *msg, const char * filename, int linenumber) {
    if (PfnAssertCallback != NULL) {
	(*PfnAssertCallback)(msg, filename, linenumber);
    } else {
	fprintf(stderr, "Assert fail in file %s, line %d\n\t%s\n", filename, linenumber, msg);
	fflush(stderr);
	assert(FALSE);
    }
}

void DAssert_SetCallback(DASSERT_CALLBACK pfn) {
    PfnAssertCallback = pfn;
}

#endif  /* defined(DEBUG) */

/* The following line is only here to prevent compiler warnings 
 * on release (non-debug) builds 
 */
static int dummyVariable = 0;  

