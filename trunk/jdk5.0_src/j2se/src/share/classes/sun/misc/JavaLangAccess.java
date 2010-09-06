/*
 * @(#)JavaLangAccess.java	1.3 04/05/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import sun.reflect.ConstantPool;
import sun.reflect.annotation.AnnotationType;

public interface JavaLangAccess {
    /** Return the constant pool for a class. */
    ConstantPool getConstantPool(Class klass);

    /**
     * Set the AnnotationType instance corresponding to this class.
     * (This method only applies to annotation types.)
     */
    void setAnnotationType(Class klass, AnnotationType annotationType);

    /**
     * Get the AnnotationType instance corresponding to this class.
     * (This method only applies to annotation types.)
     */
    AnnotationType getAnnotationType(Class klass);
}
