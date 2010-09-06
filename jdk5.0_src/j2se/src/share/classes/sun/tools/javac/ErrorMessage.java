/*
 * @(#)ErrorMessage.java	1.21 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

/**
 * A sorted list of error messages
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
@Deprecated
final
class ErrorMessage {
    long where;
    String message;
    ErrorMessage next;

    /**
     * Constructor
     */
    ErrorMessage(long where, String message) {
	this.where = where;
	this.message = message;
    }
}
