#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generateJvmOffsetsMain.c	1.5 03/12/23 16:37:38"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generateJvmOffsetsMain.c	1.5 03/12/23 16:37:38"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "generateJvmOffsets.h"

const char *HELP =
    "HELP: generateJvmOffsets {-header | -index | -table} \n";

int main(int argc, const char *argv[]) {
    GEN_variant gen_var;

    if (argc != 2) {
        printf(HELP);
	return 1; 
    }

    if (0 == strcmp(argv[1], "-header")) {
        gen_var = GEN_OFFSET;
    }
    else if (0 == strcmp(argv[1], "-index")) {
        gen_var = GEN_INDEX;
    }
    else if (0 == strcmp(argv[1], "-table")) {
        gen_var = GEN_TABLE;
    }
    else {
        printf(HELP);
	return 1; 
    }
    return generateJvmOffsets(gen_var);
}
