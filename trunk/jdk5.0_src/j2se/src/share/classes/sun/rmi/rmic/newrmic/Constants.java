/*
 * @(#)Constants.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.rmic.newrmic;

/**
 * Constants potentially useful to all rmic generators.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 *
 * @version 1.2, 03/12/19
 * @author Peter Jones
 **/
public final class Constants {

    private Constants() { throw new AssertionError(); }

    /*
     * fully-qualified names of types used by rmic
     */
    public static final String REMOTE = "java.rmi.Remote";
    public static final String EXCEPTION = "java.lang.Exception";
    public static final String REMOTE_EXCEPTION = "java.rmi.RemoteException";
    public static final String RUNTIME_EXCEPTION = "java.lang.RuntimeException";
}
