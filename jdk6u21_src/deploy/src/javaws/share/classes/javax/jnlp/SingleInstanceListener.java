/*
 * @(#)SingleInstanceListener.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;


/**
 * <code>SingleInstanceListener</code> is a interface which should be
 * implemented by a JNLP application if they wish to support
 * single instance behaviour.
 *
 * @since 1.5
 */

public interface SingleInstanceListener {

 /**  
   *  This method should be implemented by the application to
   *  handle the single instance behaviour - how should the application
   *  handle the arguments when another instance of the application is
   *  invoked with params.
   *
   *  @param params   Array of parameters for the application main
   *                    (arguments supplied in the jnlp file)
   *
   */

    void newActivation(String[] params);

}
