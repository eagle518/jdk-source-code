/*
 * @(#)ErrorConsumer.java	1.14 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

/**
 * Allows for easier parsing of errors and warnings from the compiler
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
@Deprecated 
public
interface ErrorConsumer {
	public void pushError(String errorFileName, 
							int line, 
							String message,
							String referenceText, String referenceTextPointer);
};
