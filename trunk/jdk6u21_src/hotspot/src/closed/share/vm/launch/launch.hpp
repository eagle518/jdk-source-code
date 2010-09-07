/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include <stdlib.h>
# include <string.h>

typedef void (*dll_func)(...);

dll_func vm_entry_point();

extern char *vm_library_name;
