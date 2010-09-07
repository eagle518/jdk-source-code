/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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
