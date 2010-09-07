/*
 * @(#)SingleInstanceService.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 * <code>SingleInstanceService</code> allow applications launched under Java 
 * Web Start to register themselves as singletons, and to be passed in new 
 * parameter sets when user attempts to launch new instances of them. 
 *
 * @since 1.5
 */


public interface SingleInstanceService {
 /** 
   *  Adds the specified SingleInstanceListener to handle how the application/applet
   *  should behave when another instance of the same application/applet is invoked.
   *     
   *  If SingleInstanceListener sil is null, no exception is thrown and no
   *  action is performed.
   *
   *  @param  sil  the SingleInstanceListener object to be added
   *
   */

    public void addSingleInstanceListener(javax.jnlp.SingleInstanceListener sil);

 /** 
   *  Removes the specified SingleInstanceListener.  This method performs
   *  no function, nor does it throw an exception, if the listener specified
   *  by the argument was not previously added to the application/applet.
   *  If listener sil is null, no exception is thrown and no action is
   *  performed.
   *
   *  It is recommended that if an application/applet registered any
   *  SingleInstanceListener(s), it should call this method to remove all 
   *  listeners upon exit of the application/applet.
   *
   *  @param  sil  the SingleInstanceListener object to be removed
   *
   */
    public void removeSingleInstanceListener(javax.jnlp.SingleInstanceListener sil);

}
