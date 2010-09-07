/*
 * @(#)UnixDomainSocketException.java	1.3 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.socket;

public class UnixDomainSocketException extends UnixSocketException {
    
    public static UnixDomainSocketException createUnixDomainSocketException(String msg, int errno) {
        return new UnixDomainSocketException(msg, errno);
    }

    public UnixDomainSocketException(String msg, int errno) {
        super(msg, errno);
    }

    public UnixDomainSocketException(String msg) {
        super(msg);
    }

    public UnixDomainSocketException() {
        super();
    }
}

