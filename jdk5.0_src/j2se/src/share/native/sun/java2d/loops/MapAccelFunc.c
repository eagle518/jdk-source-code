/*
 * @(#)MapAccelFunc.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"

/*
 * This is a dummy function that satisfies the MapAccelFunction
 * contract by simply returning a pointer to the indicated
 * C function.  This function is only executed in the absence
 * of an implementation specific function that maps the C
 * functions to accelerated versions of the same operation.
 */
AnyFunc *MapAccelFunction(AnyFunc *c_func) {
    return c_func;
}
