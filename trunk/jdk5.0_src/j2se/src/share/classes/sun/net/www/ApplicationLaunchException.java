/*
 * @(#)ApplicationLaunchException.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www;

/**
 * An exception thrown by the MimeLauncher when it is unable to launch
 * an external content viewer.
 *
 * @version     1.10, 12/19/03
 * @author      Sunita Mani
 */

public class ApplicationLaunchException extends Exception {
    public ApplicationLaunchException(String reason) {
	super(reason);
    }
}
