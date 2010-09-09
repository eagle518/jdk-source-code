/*
 * @(#)PrincipalImpl.java	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.acl;

import java.security.*;

/**
 * This class implements the principal interface.
 *
 * @author 	Satish Dharmaraj
 */
public class PrincipalImpl implements Principal {

    private String user;

    /**
     * Construct a principal from a string user name.
     * @param user The string form of the principal name.
     */
    public PrincipalImpl(String user) {
	this.user = user;
    }

    /**
     * This function returns true if the object passed matches 
     * the principal represented in this implementation
     * @param another the Principal to compare with.
     * @return true if the Principal passed is the same as that 
     * encapsulated in this object, false otherwise
     */
    public boolean equals(Object another) {
	if (another instanceof PrincipalImpl) {
	    PrincipalImpl p = (PrincipalImpl) another;
	    return user.equals(p.toString());
	} else
	  return false;
    }
    
    /**
     * Prints a stringified version of the principal.
     */
    public String toString() {
	return user;
    }

    /**
     * return a hashcode for the principal.
     */
    public int hashCode() {
	return user.hashCode();
    }

    /**
     * return the name of the principal.
     */
    public String getName() {
	return user;
    }

}







