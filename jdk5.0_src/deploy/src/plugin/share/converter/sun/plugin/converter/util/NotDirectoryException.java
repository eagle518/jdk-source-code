/*
 * @(#)NotDirectoryException.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import sun.plugin.converter.*;

public class NotDirectoryException extends java.io.IOException {

    public NotDirectoryException() { super(); }
    public NotDirectoryException(String s) { super(ResourceHandler.getMessage("caption.absdirnotfound") + " " + s); }
}
