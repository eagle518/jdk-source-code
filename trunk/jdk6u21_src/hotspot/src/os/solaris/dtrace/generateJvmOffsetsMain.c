/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
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


#include "generateJvmOffsets.h"

const char *HELP =
    "HELP: generateJvmOffsets {-header | -index | -table} \n";

int main(int argc, const char *argv[]) {
    GEN_variant gen_var;

    if (argc != 2) {
        printf("%s", HELP);
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
        printf("%s", HELP);
        return 1;
    }
    return generateJvmOffsets(gen_var);
}
