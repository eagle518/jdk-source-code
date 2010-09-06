/*
 * @(#)Constants.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.rmic;

import sun.tools.java.Identifier;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public interface Constants extends sun.tools.java.Constants {

    /*
     * Identifiers potentially useful for all Generators
     */
    public static final Identifier idRemote =
	Identifier.lookup("java.rmi.Remote");
    public static final Identifier idRemoteException =
	Identifier.lookup("java.rmi.RemoteException");
}
