/*
 * @(#)ConsoleController.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/*
 * ConsoleController is an interface for controlling 
 * various aspects of the console window behavior.
 *
 * @version 1.0
 * @author Stanley Man-Kit Ho
 */


public interface ConsoleController
{
    /**
     * Return true if console window should be iconified on close.
     */
    public boolean isIconifiedOnClose();

    /**
     * Return true if double buffering should be used.
     */
    public boolean isDoubleBuffered();

    /**
     * Return true if dump stack command is supported.
     */
    public boolean isDumpStackSupported();
    
    /**
     * Dump thread stack.
     *
     * @return The output of the thread stack dump.
     */
    public String dumpAllStacks();

    /**
     * Return main thread group.
     */
    public ThreadGroup getMainThreadGroup();
              
    /**
     * Return true if security policy reload is supported.
     */
    public boolean isSecurityPolicyReloadSupported();

    /**
     * Reload security policy.
     */
    public void reloadSecurityPolicy();

    /**
     * Return true if proxy config reload is supported.
     */
    public boolean isProxyConfigReloadSupported();

    /**
     * Reload proxy config.
     */
    public void reloadProxyConfig();

    /**
     * Return true if dump classloader is supported.
     */
    public boolean isDumpClassLoaderSupported();
    
    /**
     * Dump classloader list.
     *
     * @return The output of the classloader list.
     */
    public String dumpClassLoaders();    

    /**
     * Return true if clear classloader is supported.
     */
    public boolean isClearClassLoaderSupported();

    /**
     * Clear classloader list.
     */
    public void clearClassLoaders();

    /**
     * Return true if logging is supported.
     */
    public boolean isLoggingSupported();
     
    /**
     * Toggle logging supported.
     *
     * @return true if logging is enabled.
     */
    public boolean toggleLogging();

    /**
     * Return true if JConv is supported.
     */
    public boolean isJCovSupported();

    /**
     * Dump JCov data.
     *
     * @return true if JCov data is dumped successfully.
     */
    public boolean dumpJCovData();

    /**
     * Returns product name.
     */
    public String getProductName();

    /**
     * Invoke runnable object in proper AWT event dispatch thread.
     */
    public void invokeLater(Runnable runnable);

    /**
     * Notifies the ConsoleController that the console window has been
     * closed through an outside operation (clicking the system close
     * button or the "close" button at the bottom of the window).
     */
    public void notifyConsoleClosed();
}
