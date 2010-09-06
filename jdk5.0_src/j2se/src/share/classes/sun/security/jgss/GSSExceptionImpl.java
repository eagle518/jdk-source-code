/*
 * @(#)GSSExceptionImpl.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss; 
 
import org.ietf.jgss.*; 

/**
 * This class helps overcome a limitation of the org.ietf.jgss.GSSException
 * class that does not allow the thrower to set a string corresponding to
 * the major code.
 */
class GSSExceptionImpl extends org.ietf.jgss.GSSException {

    private static final long serialVersionUID = 4251197939069005575L;

    private String majorMessage;

    /**
     * A constructor that takes the majorCode as well as the message that
     * corresponds to it.
     */
    public GSSExceptionImpl(int majorCode, String majorMessage) {
	super(majorCode);
	this.majorMessage = majorMessage;
    }

    /**
     * Returns the message that was embedded in this object, otherwise it
     * returns the default message that an org.ietf.jgss.GSSException
     * generates.
     */
    public String getMessage() {
	if (majorMessage != null)
	    return majorMessage;
	else
	    return super.getMessage();
    }

}
