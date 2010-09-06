/*
 * @(#)ValueContainer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

/*
 * This interface allows us to pass fields, variables, and 
 * array components through the same interfaces. This currently allows
 * more common code for type checking. In the future we could use it for
 * more. 
 */
interface ValueContainer {
    Type type() throws ClassNotLoadedException;
    Type findType(String signature) throws ClassNotLoadedException;
    String typeName();
    String signature();
}


