/*
 * @(#)ExtensionInstallationException.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/*
 * Exception when installation of an extension has failed for 
 * any reason
 *
 * @author  Jerome Dochez
 * @version 1.7, 12/19/03
 */

public class ExtensionInstallationException extends Exception {

    /*
     * <p>
     * Construct a new exception with an exception reason
     * </p>
     */
    public ExtensionInstallationException(String s) {
	super(s);
    }
}
