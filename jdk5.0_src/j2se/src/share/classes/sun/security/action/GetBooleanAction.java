/*
 * @(#)GetBooleanAction.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.action;

/**
 * A convenience class for retrieving the boolean value of a system property
 * as a privileged action.
 *
 * <p>An instance of this class can be used as the argument of
 * <code>AccessController.doPrivileged</code>.
 *
 * <p>The following code retrieves the boolean value of the system
 * property named <code>"prop"</code> as a privileged action: <p>
 *
 * <pre>
 * boolean b = ((Boolean)java.security.AccessController.doPrivileged(
 *                         new GetBooleanAction("prop"))).booleanValue();
 * </pre>
 *
 * @author Roland Schemers
 * @version 1.9, 12/19/03
 * @see java.security.PrivilegedAction
 * @see java.security.AccessController
 * @since JDK1.2
 */

public class GetBooleanAction implements java.security.PrivilegedAction {
    private String theProp;

    /**
     * Constructor that takes the name of the system property whose boolean
     * value needs to be determined.
     *
     * @param theProp the name of the system property.
     */
    public GetBooleanAction(String theProp) {
	this.theProp = theProp;
    }

    /**
     * Determines the boolean value of the system property whose name was
     * specified in the constructor.
     *
     * @return the <code>Boolean</code> value of the system property.
     */
    public Object run() {
	return new Boolean(Boolean.getBoolean(theProp));
    }
}
