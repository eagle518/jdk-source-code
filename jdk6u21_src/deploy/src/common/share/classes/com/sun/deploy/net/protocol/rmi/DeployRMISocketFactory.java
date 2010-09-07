/*
 * @(#)DeployRMISocketFactory.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.protocol.rmi;

import sun.rmi.transport.proxy.RMIMasterSocketFactory;
import sun.rmi.transport.proxy.RMIHttpToPortSocketFactory;
import sun.rmi.transport.proxy.RMIHttpToCGISocketFactory;


/**
 * RMIPluginSocketFactory attempts to create a socket connection to the
 * specified host using successively less efficient mechanisms
 * until one succeeds.  If the host is successfully connected to,
 * the factory for the successful mechanism is stored in an internal
 * hash table keyed by the host name, so that future attempts to
 * connect to the same host will automatically use the same
 * mechanism.
 */
public class DeployRMISocketFactory extends RMIMasterSocketFactory {

    /**
     * Create a RMIPluginSocketFactory object.  Establish order of
     * connection mechanisms to attempt on createSocket, if a direct
     * socket connection fails.
     */
    public DeployRMISocketFactory()
    {
        altFactoryList.addElement(new RMIHttpToPortSocketFactory());
        altFactoryList.addElement(new RMIHttpToCGISocketFactory());
    }
}

