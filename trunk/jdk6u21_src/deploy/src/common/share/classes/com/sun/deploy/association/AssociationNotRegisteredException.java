/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


/**
 * Thrown by methods when trying to remove an association not existed in the system.
 * @version 1.0
 */
public class AssociationNotRegisteredException extends AssociationException {
  
    /**
     * Constructs an AssociationNotRegisteredException with no detail message.
     */
    public AssociationNotRegisteredException() {
        super();
    }
  
    /**
     * Constructs an AssociationNotRegisteredException with the specified detail 
     * message.
     * 
     * @param msg the detail message pertaining to this exception.
     */
    public AssociationNotRegisteredException(String msg) {
        super(msg);
    }
}
