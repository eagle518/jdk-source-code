/*
 * @(#)NotDirectoryException.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import sun.plugin.converter.*;

public class NotDirectoryException extends java.io.IOException {

    public NotDirectoryException() { super(); }
    public NotDirectoryException(String s) { super(ResourceHandler.getMessage("caption.absdirnotfound") + " " + s); }
}
