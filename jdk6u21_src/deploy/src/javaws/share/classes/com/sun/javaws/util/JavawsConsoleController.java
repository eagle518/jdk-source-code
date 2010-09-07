/*
 * @(#)JavawsConsoleController.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.javaws.util;

import com.sun.javaws.Globals;
import com.sun.deploy.util.ConsoleController;
import com.sun.deploy.util.ConsoleWindow;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import javax.swing.SwingUtilities;
import java.util.logging.Logger;
/*
 * JavawsConsoleController is a class for controlling 
 * various aspects of the javaws console window behavior.
 *
 */
public class JavawsConsoleController implements ConsoleController
{
    private static ConsoleWindow console = null;
    private static JavawsConsoleController jcc = null;

    public JavawsConsoleController() {
    }

    public static JavawsConsoleController getInstance() {
	if (jcc == null) {
	    if (Globals.isJavaVersionAtLeast14()) {
		jcc = new JavawsConsoleController14();
	    } else {
		jcc = new JavawsConsoleController();
	    }
	}
	return jcc;
    }

    public void setLogger(Logger lg) {
	return;
    }

    public void setConsole(ConsoleWindow cw) {
	if (console == null) {
	    console = cw;
	}
    }

    public ConsoleWindow getConsole() {
	return console;
    }

    /**
     * Return true if console window should be iconified on close.
     */
    public boolean isIconifiedOnClose() {
	return false;
    }

    /**
     * Return true if double buffering should be used.
     */
    public boolean isDoubleBuffered() {
	return true;
    }

    /**
     * Return true if dump stack command is supported.
     */
    public boolean isDumpStackSupported() {
	return false;
    }
    
    /**
     * Dump thread stack.
     *
     * @return The output of the thread stack dump.
     */
    public String dumpAllStacks() {
	return null;
    }

    /**
     * Return main thread group.
     */
    public ThreadGroup getMainThreadGroup() {
	return Thread.currentThread().getThreadGroup();
    }
              
    /**
     * Return true if security policy reload is supported.
     */
    public boolean isSecurityPolicyReloadSupported() {
	return false;
    }

    /**
     * Reload security policy.
     */
    public void reloadSecurityPolicy() {
	return;
    }

    /**
     * Return true if proxy config reload is supported.
     */
    public boolean isProxyConfigReloadSupported()
    {
	return true;
    }

    /**
     * Reload proxy config.
     */
    public void reloadProxyConfig()
    {
	com.sun.deploy.net.proxy.DynamicProxyManager.reset();
    }

    /**
     * Return true if dump classloader is supported.
     */
    public boolean isDumpClassLoaderSupported() {
	return false;
    }
    
    /**
     * Dump classloader list.
     *
     * @return The output of the classloader list.
     */
    public String dumpClassLoaders() {
	return null;
    }

    /**
     * Return true if clear classloader is supported.
     */
    public boolean isClearClassLoaderSupported() {
	return false;
    }

    /**
     * Clear classloader list.
     */
    public void clearClassLoaders() {
	return;
    }

    /**
     * Return true if logging is supported.
     */
    public boolean isLoggingSupported() {
	return false;	
    }
     
    /**
     * Toggle logging supported.
     *
     * @return true if logging is enabled.
     */
    public boolean toggleLogging() {
	return false;
    }

    /**
     * Return true if JConv is supported.
     */
    public boolean isJCovSupported() {
	return false;
    }

    /**
     * Dump JCov data.
     *
     * @return true if JCov data is dumped successfully.
     */
    public boolean dumpJCovData() {
	return false;
    }

    /**
     * Returns product name.
     */
    public String getProductName() {
	return ResourceManager.getString("product.javaws.name", Globals.JAVAWS_VERSION);
    }

    /**
     * Invoke runnable object in proper AWT event dispatch thread.
     */
    public void invokeLater(Runnable runnable) {
	SwingUtilities.invokeLater(runnable);
    }

    public void notifyConsoleClosed() {
        // If we used the system tray icon for Java Web Start we'd
        // update its state here
    }

    public static void showConsoleIfEnable() {
	if (Config.getProperty(Config.CONSOLE_MODE_KEY).equals(
             Config.CONSOLE_MODE_SHOW)) {
	    console.showConsole(true);
        }
    }
    
    public static void setTitle(String prefixKey, String titleSuffix) {
        if (console != null) {
            console.setTitle(
                    ResourceManager.getMessage(prefixKey) + titleSuffix);
        }
    }
}
