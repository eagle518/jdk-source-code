#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generateJvmOffsets.h	1.2 03/12/23 16:37:38"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <strings.h>

typedef enum GEN_variant {
	GEN_OFFSET = 0,
	GEN_INDEX  = 1,
	GEN_TABLE  = 2
} GEN_variant;

extern "C" {
	int generateJvmOffsets(GEN_variant gen_var);
	void gen_prologue(GEN_variant gen_var);
	void gen_epilogue(GEN_variant gen_var);
}
