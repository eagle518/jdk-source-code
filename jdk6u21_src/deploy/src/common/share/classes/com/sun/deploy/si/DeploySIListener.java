/*
 * 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.si;


/**
 * <code>DeploySIListener</code> is same as 
 *    javax.jnlp.SingleInstanceListener
 */

public interface DeploySIListener {

 /**
   *  This method should be implemented by the application to
   *  handle the single instance behaviour - how should the application
   *  handle the arguments when another instance of the application is
   *  invoked with params.
   *
   *  @param[] params   Array of parameters for the application main
   *                    (arguments supplied in the jnlp file)
   *
   */

    void newActivation(String[] params);
    Object getSingleInstanceListener();
}

