/*
 * @(#)BinaryExceptionHandler.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * A single exception handler.  This class hangs off BinaryCode.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */

public class BinaryExceptionHandler {
    public int startPC;		
    public int endPC;
    public int handlerPC;
    public ClassDeclaration exceptionClass;

    BinaryExceptionHandler(int start, int end, 
			   int handler, ClassDeclaration xclass) {
	startPC = start;
	endPC = end;
	handlerPC = handler;
	exceptionClass = xclass;
    }
}
