/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


/**
 * Base class for all exception classes for association service. 
 * 
 * @version 1.0
 */
public class AssociationException extends Exception {
  
    /**
     * Constructs an AssociationException with no detail message.
     */
    public AssociationException() {
        super();
    }
  
    /**
     * Constructs an AssociationException with the specified detail message.
     * 
     * @param msg the detail message pertaining to this exception.
     */
    public AssociationException(String msg) {
        super(msg);
    }
}
