/*
 * @(#)JarException.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jar;

import java.io.IOException;

public
class JarException extends IOException {
    public JarException() {
	super();
    }

    public JarException(String s) {
	super(s);
    }
}
