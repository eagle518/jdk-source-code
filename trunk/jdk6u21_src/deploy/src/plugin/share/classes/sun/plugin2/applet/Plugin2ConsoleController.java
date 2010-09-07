/*
 * @(#)Plugin2ConsoleController.java	1.9 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.io.*;
import java.security.Policy;
import java.util.logging.Level;
import java.util.logging.Logger;

import sun.plugin.WJcovUtil;
import sun.plugin.services.BrowserService;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.UserProfile;
import com.sun.deploy.cache.MemoryCache;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.security.CertificateHostnameVerifier;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.security.X509DeployTrustManager;
import com.sun.deploy.ui.JavaTrayIcon;
import com.sun.deploy.util.ConsoleController14;
import com.sun.deploy.util.ConsoleHelper;
import com.sun.deploy.util.LoggerTraceListener;
import com.sun.deploy.util.SecurityBaseline;
import com.sun.deploy.util.Trace;

import sun.plugin2.applet.Applet2ClassLoaderCache;
import sun.plugin2.applet.Applet2ManagerCache;

/** This class replaces the PluginConsoleController to avoid calling
    into unused functionality in the old plug-in. We needed to avoid
    subclassing PluginConsoleController because it made calls in its
    constructor we weren't prepared for (casting the result of
    ServiceManager.getService() to a BrowserService). */

public class Plugin2ConsoleController implements ConsoleController14 {
    private Applet2ClassLoaderCache cache;
    private Applet2ManagerCache instanceCache;

    private boolean onWindows = false;
    private boolean isMozilla = false;
    private boolean initializedIconifiedOnClose = false;
    private boolean iconifiedOnClose = false;
    private Logger logger = null;

    public Plugin2ConsoleController(Applet2ClassLoaderCache cache,
                                    Applet2ManagerCache instanceCache) {
        this.cache = cache;
        this.instanceCache = instanceCache;

	try {
            // Determine the platform that we are on
            //
            String osName = (String) java.security.AccessController.doPrivileged(
                                new sun.security.action.GetPropertyAction("os.name"));

            if (osName.indexOf("Windows") != -1)
                onWindows = true;

            String workaround = (String) java.security.AccessController.doPrivileged(
                                    new sun.security.action.GetPropertyAction("mozilla.workaround", "false"));

            if (workaround != null && workaround.equalsIgnoreCase("true"))
                isMozilla = true;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Return true if console window should be iconified on close.
     */
    public boolean isIconifiedOnClose()
    {
        if (!initializedIconifiedOnClose) {
            BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
            iconifiedOnClose = service.isConsoleIconifiedOnClose();
        }

        return iconifiedOnClose;
    }

    /**
     * Return true if double buffering should be used.
     */
    public boolean isDoubleBuffered()
    {
        /* WORKAROUND>>>
         * There seems to be a major problem with double buffering in swing and Netscape 6.
         * We turn off buffering until the problem with Netscape 6 or Swing can be resolved
         */
        return !(onWindows == false && isMozilla == true);
    }


    /**
     * Return true if dump stack command is supported.
     */
    public boolean isDumpStackSupported()
    {
        return true;
    }
    
    /**
     * Dump thread stack.
     *
     * @return The output of the thread stack dump.
     */
    public String dumpAllStacks()
    {
        return ConsoleHelper.dumpAllStacks();
    }

    /**
     * Return main thread group.
     */
    public ThreadGroup getMainThreadGroup()
    {
        return PluginSysUtil.getPluginThreadGroup().getParent();
    }
              
    /**
     * Return true if security policy reload is supported.
     */
    public boolean isSecurityPolicyReloadSupported()
    {
        return true;
    }

    /**
     * Reload security policy.
     */
    public void reloadSecurityPolicy()
    {
        Policy policy = Policy.getPolicy();
        policy.refresh();
    }

    /**
     * Return true if proxy config reload is supported.
     */
    public boolean isProxyConfigReloadSupported()
    {
        // This doesn't make sense in this implementation
        return false;
    }

    /**
     * Reload proxy config.
     */
    public void reloadProxyConfig()
    {
    }

    /**
     * Return true if dump classloader is supported.
     */
    public boolean isDumpClassLoaderSupported()
    {
        return true;
    }
    
    /**
     * Dumps the list of class loaders in the class loader cache.
     */
    public String dumpClassLoaders()
    {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream ps = new PrintStream(baos);
        if (cache != null) {
            cache.dump(ps);
        } else {
            ps.println("No class loader cache installed.");
        }
        return new String(baos.toByteArray());
    }

    /**
     * Return true if clear classloader is supported.
     */
    public boolean isClearClassLoaderSupported()
    {
        return true;
    }

    /**
     * Clears the class loaders in the class loader cache.
     */
    public void clearClassLoaders()
    {
        // We can't call super.clearClassLoaders() because it
        // references ClassLoaderInfo, which we don't use or support

        // Clear cache files and jars in memory
        MemoryCache.clearLoadedResources();
        
        if (cache != null) {
            // Clear the class loader cache
            cache.clear();
        }

        if (instanceCache != null) {
            // Clear the legacy lifecycle instance cache
            instanceCache.clear();
        }

        // We MUST reset the trust decider so newer classes
        // with different cert may be loaded
        TrustDecider.reset();

        // Reset trust manager for HTTPS
        X509DeployTrustManager.reset();

        // Reset the certificate hostname verifier for HTTPS
        CertificateHostnameVerifier.reset();
    }

    /**
     * Return true if logging is supported.
     */
    public boolean isLoggingSupported()
    {
        return true;
    }

    public void setLogger(Logger l) {
        logger = l;
    }

    public Logger getLogger() {
        return logger;
    }

    /**
     * Toggle logging supported.
     *
     * @return true if logging is enabled.
     */
    public boolean toggleLogging()
    {
        if (logger == null) {
            File logDir = new File(UserProfile.getLogDirectory());
            File logFile = Trace.createTempFile(Config.PLUGIN_OUTPUTFILE_PREFIX, Config.OUTPUTFILE_LOG_SUFFIX, logDir); 
            LoggerTraceListener ltl = new LoggerTraceListener("sun.plugin", logFile.getPath());
            logger = ltl.getLogger();
        }

        Level level = logger.getLevel();
 
        if (level == Level.OFF)
            level = Level.ALL;
        else
            level = Level.OFF;
           
        logger.setLevel(level);

        return (level == Level.ALL);
    }

    /**
     * Return true if JConv is supported.
     */
    public boolean isJCovSupported()
    {
        boolean ret = false;

        if (onWindows) {
            String runjcovOption = System.getProperty("javaplugin.vm.options");
            ret = (runjcovOption != null && runjcovOption.indexOf("-Xrunjcov") != -1);
        }

        return ret;
    } 

    /**
     * Dump JCov data.
     *
     * @return true if JCov data is dumped successfully.
     */
    public boolean dumpJCovData()
    {
        return WJcovUtil.dumpJcovData();
    }

    /**
     * Returns product name.
     */
    public String getProductName()
    {
        return ResourceManager.getString("product.javapi.name", SecurityBaseline.getCurrentVersion());
    }

    /**
     * Invoke runnable object in proper AWT event dispatch thread.
     */
    public void invokeLater(Runnable runnable)
    {
        PluginSysUtil.invokeLater(runnable);
    }

    public void notifyConsoleClosed() {
        JavaTrayIcon.notifyConsoleClosed();
    }
}
