/*
 * @(#)BadTokenException.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.xml;

/**
 * Exception thrown if a parse error occured when interpreting
 * the launch descriptor
 */

public class BadTokenException extends Exception {
    private int    _line;
    private String _source;
    private String _message;

    public BadTokenException(String source, int line) {
        this(null, source, line);
    }

    public BadTokenException(String message, String source, int line) {
        _line    = line;
        _source  = source;
        _message = message;
    }

    public int getLine() { return _line; }
    public String getSource() { return _source; }
    public String getReason() { return _message; }

    public String toString() {
        String result;

        if (_message != null) {
            result = _message + " Exception parsing xml at line " + _line;
        }
        else {
            result = "Exception parsing xml at line " + _line;
        }

	return (result);
    }
}


