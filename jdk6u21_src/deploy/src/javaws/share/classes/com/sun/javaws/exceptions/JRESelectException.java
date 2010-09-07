/*
 * @(#)JRESelectException.java	
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

import com.sun.javaws.jnl.JREDesc;

/**
 * Exception thrown when for any reason Java Web Start cannot run
 */

public class JRESelectException extends ExitException {
    private JREDesc _jreDesc ;
    private String  _jvmArgs ;

    public JRESelectException(JREDesc jreDesc, String jvmArgs) {
        this(jreDesc, jvmArgs, null);
    }

    public JRESelectException(JREDesc jreDesc, String jvmArgs, Throwable throwable) {
        super(throwable, ExitException.JRE_MISMATCH);
        _jreDesc  = jreDesc;
        _jvmArgs  = jvmArgs;
    }
    
    public JREDesc    getJREDesc()   { return _jreDesc; }
    public String     getJVMArgs()   { return _jvmArgs; }

    public String toString() {
        String str = "JRESelectException[ jreDesc: " + _jreDesc + "; jvmArgs: " + _jvmArgs + " ]"; 
        if (getException() != null) { 
            str += getException().toString();
        }
        return str;
    }
}


