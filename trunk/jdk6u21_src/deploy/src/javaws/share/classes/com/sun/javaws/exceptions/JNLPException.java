/*
 * @(#)JNLPException.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.jnl.LaunchDesc;

/** Root exception for all exceptions thrown by
 *  this JNLP Client
 */
abstract public class JNLPException extends Exception {
    
    // Static reference to the default LaunchDescriptor. This is
    // used in the Error Message if the particular exception does
    // not overwrite it
    static private LaunchDesc _defaultLaunchDesc = null;
    
    // Reference to a exception specific LaunchDesc
    private LaunchDesc _exceptionLaunchDesc = null;
    
    // Specifies what kind of exception it is
    private String _categoryMsg = null;
    
    // Specifies the underlying exception that caused this condition, if any
    private Throwable _wrappedException = null;
    
    /** Construct a JNLP exception */
    public JNLPException(String category) {
        this(category, null, null);
    }
    
    /** Construct a JNLP exception */
    public JNLPException(String category, LaunchDesc ld) {
        this(category, ld, null);
    }
    
    /** Construct a JNLP exception */
    public JNLPException(String category, Throwable exception) {
        this(category, null, exception);
    }
    
    /** Construct a JNLP exception */
    public JNLPException(String category, LaunchDesc ld, Throwable wrappedException) {
        super();
        _categoryMsg = category;
        _exceptionLaunchDesc = ld;
        _wrappedException = wrappedException;
    }
    
    /** Sets the default exception */
    static public void setDefaultLaunchDesc(LaunchDesc ld) { _defaultLaunchDesc = ld; }
    
    /** Get the default LaunchDesc */
    static public LaunchDesc getDefaultLaunchDesc() {
        return _defaultLaunchDesc;
    }
    
    /** Returns the localized error message for the exception. This is overwritten
     *  to call get getRealMessage to force compile time errors if the subclass does
     *  not implemet it
     */
    public String getMessage() { return getRealMessage(); }
    
    public String getBriefMessage() { return null; }

    /** Must be specified by subclass */
    protected abstract String getRealMessage();
    
    /** Get LaunchDesc that was processed when this exception happened. This
     *  method defaults to the default if the exception does not write its own
     */
    public LaunchDesc getLaunchDesc() {
        return (_exceptionLaunchDesc != null) ? _exceptionLaunchDesc : _defaultLaunchDesc;
    }
    
    /** Returns the source of the LaunchDesc. This might be overwritten in subclasses */
    public String getLaunchDescSource() {
        LaunchDesc ld = getLaunchDesc();
        if (ld == null) return null;
        return ld.getSource();
    }
    
    /** Get category for exception */
    public String getCategory() { return _categoryMsg; }
    
    /** Get the expeception that caused this exception to be thrown */
    public Throwable getWrappedException() { return _wrappedException ; }
    
    /** Output */
    public String toString() {
        return "JNLPException[category: " + _categoryMsg +
            " : Exception: " + _wrappedException +
            " : LaunchDesc: " + _exceptionLaunchDesc + " ]"; };
}


