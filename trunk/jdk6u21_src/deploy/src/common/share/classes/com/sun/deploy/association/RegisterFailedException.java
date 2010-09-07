/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


/**
 * Thrown by methods when registering/unregistering an association failed.
 * @version 1.0
 */
public class RegisterFailedException extends AssociationException {
  
    /**
     * Constructs a RegisterFailedException with no detail message.
     */
    public RegisterFailedException() {
        super();
    }
  
    /**
     * Constructs a RegisterFailedException with the specified detail 
     * message.
     * @param msg the detail message pertaining to this exception.
     */
    public RegisterFailedException(String msg) {
        super(msg);
    }
}
