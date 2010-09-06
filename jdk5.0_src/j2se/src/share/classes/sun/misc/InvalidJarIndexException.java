/*
 * @(#)InvalidJarIndexException.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.lang.LinkageError;

/**
 * Thrown if the URLClassLoader finds the INDEX.LIST file of
 * a jar file contains incorrect information.
 *
 * @author   Zhenghua Li
 * @version  1.6, 12/19/03
 * @since   1.3
 */

public
class InvalidJarIndexException extends RuntimeException {

    /**
     * Constructs an <code>InvalidJarIndexException</code> with no 
     * detail message.
     */
    public InvalidJarIndexException() {
	super();
    }

    /**
     * Constructs an <code>InvalidJarIndexException</code> with the 
     * specified detail message. 
     *
     * @param   s   the detail message.
     */
    public InvalidJarIndexException(String s) {
	super(s);
    }
}
