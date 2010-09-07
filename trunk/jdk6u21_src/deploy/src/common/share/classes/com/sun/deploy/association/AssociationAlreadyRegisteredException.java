/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


/**
 * Thrown by methods when trying to add an association already existed in the system.
 * @version 1.0
 */
public class AssociationAlreadyRegisteredException extends AssociationException {
  
    /**
     * Constructs an AssociationAlreadyRegisteredException with no detail 
     * message.
     */
    public AssociationAlreadyRegisteredException() {
        super();
    }
  
    /**
     * Constructs an AssociationAlreadyRegisteredException with the specified detail 
     * message.
     * 
     * @param msg the detail message pertaining to this exception.
     */
    public AssociationAlreadyRegisteredException(String msg) {
        super(msg);
    }
}
