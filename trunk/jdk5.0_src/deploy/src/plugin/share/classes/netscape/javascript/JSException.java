/*
 * @(#)JSException.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.javascript;


/** 
 * <p> Emulate netscape.javascript.JSException so applet can access the JavaScript
 * Document Object Model in the browser. JSException is thrown whenever there is
 * an error when JSObject is accessed.
 * </p>
 */
public class JSException extends RuntimeException {

    // Exception type supported by JavaScript 1.4 in Navigator 5.0.
    //
    public static final int EXCEPTION_TYPE_EMPTY = -1;
    public static final int EXCEPTION_TYPE_VOID = 0;
    public static final int EXCEPTION_TYPE_OBJECT = 1;
    public static final int EXCEPTION_TYPE_FUNCTION = 2;
    public static final int EXCEPTION_TYPE_STRING = 3;
    public static final int EXCEPTION_TYPE_NUMBER = 4;
    public static final int EXCEPTION_TYPE_BOOLEAN = 5;
    public static final int EXCEPTION_TYPE_ERROR = 6;

    /** 
     * <p> Construct a JSException object. 
     * </p>
     */
    public JSException() {
	this(null);
    }

    /** 
     * <p> Construct a JSException object. 
     * </p>
     *
     * @param s The detail message.
     */
     public JSException(String s)  {
	this(s, null, -1, null, -1);
    }

    
    /** 
     * <p> Construct a JSException object. 
     * </p>
     *
     * @param s The detail message.
     * @param filename The URL of the file where the error occurred, if possible.
     * @param lineno The line number if the file, if possible.
     * @param source The string containing the JavaScript code being evaluated.
     * @param tokenIndex The index into the source string where the error occurred.
     */
    public JSException(String s, String filename, int lineno, String source, 
		       int tokenIndex)  {
	super(s);
	this.message = s;
	this.filename = filename;
	this.lineno = lineno;
	this.source = source;
	this.tokenIndex = tokenIndex;	
	this.wrappedExceptionType = EXCEPTION_TYPE_EMPTY;
    }

    /**
     * <P> Constructs a JSException with a wrapped JavaScript exception 
     * object. This constructor is used by JavaScript 1.4 in Navigator 5.0.
     * </P>
     * 
     * @param wrappedExceptionType Type of the wrapped JavaScript exception.
     * @param wrappedException JavaScript exception wrapper.
     */
    public JSException(int wrappedExceptionType, Object wrappedException) {
	this();
	this.wrappedExceptionType = wrappedExceptionType;
	this.wrappedException = wrappedException;
    }

    /** 
     * <p> The detail message. </p> 
     */
    protected String message = null;

    /**
     * <p> The URL of the file where the error occurred, if possible. </p>
     */
    protected String filename = null;

    /** 
     * <p> The line number if the file, if possible. </p>
     */
    protected int lineno = -1;

    /** 
     * <p> The string containing the JavaScript code being evaluated. </p>
     */
    protected String source = null;

    /** 
     * <p> The index into the source string where the error occurred. </p>
     */
    protected int tokenIndex = -1;

    /**
     * <p> Type of the wrapped JavaScript exception. </p>
     */
    private int wrappedExceptionType = -1;

    /**
     * <p> JavaScript exception wrapper. </p>
     */
    private Object wrappedException = null;

    /**
     * <P> getWrappedExceptionType returns the int mapping of the
     * type of the wrappedException Object. 
     * </P>
     * 
     * @return int JavaScript exception type.
     */
    public int getWrappedExceptionType() {
	return wrappedExceptionType;
    }

    /**
     * <P> getWrappedException returns the wrapped JavaScript exception.
     * </P>
     *
     * @return Object JavaScript exception wrapper.
     */
    public Object getWrappedException() {
	return wrappedException;
    }
}
