/*
 * @(#)JavaScriptPermission.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

import java.security.BasicPermission;

/**
 * <P> JavaScriptPermission is for encapsulating a JavaScript 
 * permission which is passed through LiveConnect.
 * </P>
 *
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */


public final class JavaScriptPermission extends BasicPermission {

    /**
     * Creates a new JavaScriptPermission with the specified name.
     * The name is the symbolic name of the JavaScriptPermission. An asterisk
     * may appear at the end of the name, following a ".", or by itself, to
     * signify a wildcard match.
     *
     * @param name the name of the JavaScriptPermission
     */

    public JavaScriptPermission(String name)
    {
	super(name);
    }

    /**
     * Creates a new JavaScriptPermission object with the specified name.
     * The name is the symbolic name of the JavaScriptPermission, and the
     * actions String is currently unused and should be null.  This
     * constructor exists for use by the <code>Policy</code> object
     * to instantiate new Permission objects.
     *
     * @param name the name of the JavaScriptPermission
     * @param actions should be null.
     */

    public JavaScriptPermission(String name, String actions)
    {
	super(name, actions);
    }
}
