/*
 * @(#)iiopnameURLContextFactory.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.iiopname;

import com.sun.jndi.url.iiop.iiopURLContextFactory;

/**
 * An iiopname URL context factory.
 * It just uses the iiop URL context factory but is needed here
 * so that NamingManager.getURLContext() will find it.
 * 
 * @author Rosanna Lee
 * @version 1.6 03/12/19
 */
final public class iiopnameURLContextFactory extends iiopURLContextFactory {
}
