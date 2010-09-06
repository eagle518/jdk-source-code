/*
 * @(#)GetIntegerAction.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.action;

/**
 * A convenience class for retrieving the integer value of a system property
 * as a privileged action.
 *
 * <p>An instance of this class can be used as the argument of
 * <code>AccessController.doPrivileged</code>.
 *
 * <p>The following code retrieves the integer value of the system
 * property named <code>"prop"</code> as a privileged action. Since it does
 * not pass a default value to be used in case the property
 * <code>"prop"</code> is not defined, it has to check the result for
 * <code>null</code>: <p>
 *
 * <pre>
 * Integer tmp = (Integer)java.security.AccessController.doPrivileged
 *     (new sun.security.action.GetIntegerAction("prop"));
 * int i;
 * if (tmp != null) {
 *     i = tmp.intValue();
 * }
 * </pre>
 *
 * <p>The following code retrieves the integer value of the system
 * property named <code>"prop"</code> as a privileged action, and also passes
 * a default value to be used in case the property <code>"prop"</code> is not
 * defined: <p>
 *
 * <pre>
 * int i = ((Integer)java.security.AccessController.doPrivileged(
 *                         new GetIntegerAction("prop", 3))).intValue();
 * </pre>
 *
 * @author Roland Schemers
 * @version 1.10, 12/19/03
 * @see java.security.PrivilegedAction
 * @see java.security.AccessController
 * @since JDK1.2
 */

public class GetIntegerAction implements java.security.PrivilegedAction {
    private String theProp;
    private int defaultVal;
    private boolean defaultSet = false;

    /**
     * Constructor that takes the name of the system property whose integer
     * value needs to be determined.
     *
     * @param theProp the name of the system property.
     */
    public GetIntegerAction(String theProp) {
	this.theProp = theProp;
    }

    /**
     * Constructor that takes the name of the system property and the default
     * value of that property.
     *
     * @param theProp the name of the system property.
     * @param defaulVal the default value.
     */
    public GetIntegerAction(String theProp, int defaultVal) {
        this.theProp = theProp;
        this.defaultVal = defaultVal;
	this.defaultSet = true;
    }

    /**
     * Determines the integer value of the system property whose name was
     * specified in the constructor.
     *
     * <p>If there is no property of the specified name, or if the property
     * does not have the correct numeric format, then an <code>Integer</code>
     * object representing the default value that was specified in the
     * constructor is returned, or <code>null</code> if no default value was
     * specified.
     *
     * @return the <code>Integer</code> value of the property.
     */
    public Object run() {
        Integer value = Integer.getInteger(theProp);
        if ((value == null) && defaultSet)
	    return new Integer(defaultVal);
	return value;
    }
}
