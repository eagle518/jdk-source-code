/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
