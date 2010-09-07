/*
 * @(#)NotFileException.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

public class NotFileException extends java.io.IOException {

    public NotFileException() { super(); }
    public NotFileException(String s) { super(s); }
}
