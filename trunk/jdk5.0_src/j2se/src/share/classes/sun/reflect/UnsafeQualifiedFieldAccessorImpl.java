/*
 * @(#)UnsafeQualifiedFieldAccessorImpl.java	1.1 04/05/10
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import sun.misc.Unsafe;

/** 
 * Base class for sun.misc.Unsafe-based FieldAccessors for fields with
 * final or volatile qualifiers. These differ from unqualified
 * versions in that (1) they check for read-only status (2) they use
 * the volatile forms of Unsafe get/put methods. (When accessed via
 * reflection, finals act as slightly "lighter" forms of volatiles. So
 * the volatile forms are heavier than necessary in terms of
 * underlying reordering rules and memory barriers, but preserve
 * correctness.)
 */

abstract class UnsafeQualifiedFieldAccessorImpl
    extends UnsafeFieldAccessorImpl
{
    protected final boolean isReadOnly;

    UnsafeQualifiedFieldAccessorImpl(Field field, boolean isReadOnly) {
        super(field);
        this.isReadOnly = isReadOnly;
    }
}
