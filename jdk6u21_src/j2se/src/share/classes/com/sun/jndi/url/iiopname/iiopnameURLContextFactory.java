/*
 * @(#)iiopnameURLContextFactory.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.iiopname;

import com.sun.jndi.url.iiop.iiopURLContextFactory;

/**
 * An iiopname URL context factory.
 * It just uses the iiop URL context factory but is needed here
 * so that NamingManager.getURLContext() will find it.
 * 
 * @author Rosanna Lee
 * @version 1.8 10/03/23
 */
final public class iiopnameURLContextFactory extends iiopURLContextFactory {
}
