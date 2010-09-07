/*
 * @(#)Applet2Environment.java	1.32 10/03/31
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.applet.AppletContext;
import java.lang.reflect.Method;
import java.net.URL;
import java.util.Properties;
import java.io.File;
import java.security.Policy;
import java.security.AccessController;
import java.security.PrivilegedAction;

import sun.awt.AppContext;
import sun.net.www.protocol.jar.URLJarFile;

import sun.plugin.JavaRunTime;
import sun.plugin.PluginURLJarFileCallBack;
import sun.plugin.cache.CacheUpdateHelper;
import sun.plugin.services.BrowserService;
import sun.plugin.util.PluginSysUtil;
import com.sun.deploy.Environment;
import com.sun.deploy.config.Config;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.ui.DialogHook;
import com.sun.deploy.ui.JavaTrayIcon;
import com.sun.deploy.ui.JavaTrayIconController;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.ConsoleController14;
import com.sun.deploy.util.ConsoleHelper;
import com.sun.deploy.util.SecurityBaseline;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.UpdateCheck;
import com.sun.javaws.util.JavawsConsoleController;

import sun.plugin2.util.SystemUtil;

/** Initializes the environment in the current JVM for executing
    applets using the deployment code. */

public class Applet2Environment {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    private static boolean initialized = false;
    private static final String theVersion = "1.1";

    /** The controller is an optional argument which allows overriding
        of the default PluginConsoleController. <P>

        The defaultContext is an optional argument passed to the
        Applet2BrowserService.install() method which allows for
        example the Java Console's resetting of the TrustDecider to
        work; this method is called outside the scope of any running
        applet. The delegation path implies that the correct
        underlying code (fetching of the various browser certificate
        stores) will be called regardless. (FIXME: this is one of the
        areas that should be rethought.)
    */
    public static void initialize(String jvmArgs,
                                  boolean redirectStdio,
                                  boolean printTraceMessages,
                                  final ConsoleController14 controller,
                                  Applet2ExecutionContext defaultContext,
                                  DialogHook dialogHook) {
        synchronized (Applet2Environment.class) {
            if (initialized)
                return;

            long t0 = DeployPerfUtil.put(0, "Applet2Environment.initialize() - BEGIN");

            // Set the "javaplugin.version" property to make the
            // deployment code (in particular the Environment class)
            // think we're running the Java Plug-In. Note that these
            // system properties must be set very early, in particular
            // before calling JavaRunTime.initEnvironment().
            System.setProperty("javaplugin.version", SecurityBaseline.getCurrentVersion());
            System.setProperty("javaplugin.nodotversion", SecurityBaseline.getCurrentNoDotVersion());

            // Set the javaplugin.vm.options system property, mainly
            // for testing purposes
            if (jvmArgs != null) {
                System.setProperty("javaplugin.vm.options", jvmArgs);
            }

            // Initialization fragments from AppletViewer.initEnvironment()
            DeploySysRun.setOverride(new PluginSysUtil());

            //DeployPerfUtil.put("Applet2Environment.initialize() - before Trace.redirectStdioStderr()");

            if (redirectStdio) {
                try {
                    com.sun.deploy.util.Trace.redirectStdioStderr();
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - before Trace.addTraceListener()");

            if (printTraceMessages) {
                Trace.addTraceListener(new com.sun.deploy.util.TraceListener() {
                        public void print(String msg) {
                            System.out.println(msg);
                        }
                    });
            }

            if (!Environment.isJavaPlugin()) {
                new InternalError("\n****************************************************************\n" +
                                  "ERROR: the javaplugin.version system property wasn't picked up\n" +
                                  "by the com.sun.deploy.Environment class. This probably happened\n" +
                                  "because of a change to the initialization order in PluginMain\n" +
                                  "where the deployment classes are being initialized too early.\n" +
                                  "This will break jar cache versioning, and possibly other things.\n" +
                                  "Please undo your recent changes and rethink them.\n" +
                                  "****************************************************************").printStackTrace();
                // Continue execution, because otherwise we won't pop up the Java Console
                // and won't see this error, which will probably make it harder to debug
                // However, be resilient
                Environment.reset();
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - before PluginSysUtil.getPluginThreadGroup()");

            // to make sure plugin thread group is child of main thread group
            PluginSysUtil.getPluginThreadGroup();

            //DeployPerfUtil.put("Applet2Environment.initialize() - before Applet2BrowserService.install()");

            Applet2BrowserService.install(defaultContext);

            //DeployPerfUtil.put("Applet2Environment.initialize() - before Class.forName()");

            // This is a workaround for a memory leak in Swing
            // (6482575) the fix for which isn't present before 6u10
            if (System.getProperty("java.version").compareTo("1.6.0_10") < 0) {
                try {
                    Class.forName("javax.swing.ImageIcon");
                } catch (Throwable e) {
                }
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - before JDK11ClassFileTransformer.init()");

            try {
                // Initialize the ClassFileTransformer for JDK 1.1
                // class file.
                sun.plugin.security.JDK11ClassFileTransformer.init();
            } catch (Throwable e) {
                e.printStackTrace();
            }

            //long t1 = DeployPerfUtil.put(0, "Applet2Environment.initialize() - post property setup (1.0)");

            // Load deployment configuration properties
            Properties configProps = Config.getProperties();

            // Load System props (includes deployment props now)
            Properties props = new Properties(System.getProperties());
        
            //DeployPerfUtil.put("Applet2Environment.initialize() - post property setup (1.1)");

            // Define a number of standard properties
            props.put("acl.read", "+");
            props.put("acl.read.default", "");
            props.put("acl.write", "+");
            props.put("acl.write.default", "");

            // Standard browser properties
            props.put("browser", "sun.plugin");
            props.put("browser.version", theVersion);
            props.put("browser.vendor", "Sun Microsystems, Inc.");

            // Set HTTP User-Agent
            props.put("http.agent", "Mozilla/4.0 (" + System.getProperty("os.name") + " " + System.getProperty("os.version") + ")");

            // turn on error stream buffering
            props.put("sun.net.http.errorstream.enableBuffering", "true");

            // Define which packages can NOT be accessed by applets
            props.put("package.restrict.access.sun", "true");
            props.put("package.restrict.access.com.sun.deploy", "true");

            // Define which JSS packages can NOT be accessed by applets
            props.put("package.restrict.access.org.mozilla.jss", "true");

            //
            // This is important to set the netscape package access to "false".
            // Some applets running in IE and NS will access
            // netscape.javascript.JSObject sometimes. If we set this
            // restriction to "true", these applets will not run at all.
            // However, if we set it to "false", the applet may continue
            // to run by catching an exception.
            props.put("package.restrict.access.netscape", "false");

            // Define which packages can NOT be extended by applets
            props.put("package.restrict.definition.java", "true");
            props.put("package.restrict.definition.sun", "true");
            props.put("package.restrict.definition.netscape", "true");
            props.put("package.restrict.definition.com.sun.deploy", "true");

            // Define which JSS packages can NOT be extended by applets
            props.put("package.restrict.definition.org.mozilla.jss", "true");

            // Define which properties can be read by applets.
            // A property named by "key" can be read only when its twin
            // property "key.applet" is true.  The following ten properties
            // are open by default.  Any other property can be explicitly
            // opened up by the browser user setting key.applet=true in
            // ~/.java/properties.   Or vice versa, any of the following can
            // be overridden by the user's properties.
            props.put("java.version.applet", "true");
            props.put("java.vendor.applet", "true");
            props.put("java.vendor.url.applet", "true");
            props.put("java.class.version.applet", "true");
            props.put("os.name.applet", "true");
            props.put("os.version.applet", "true");
            props.put("os.arch.applet", "true");
            props.put("file.separator.applet", "true");
            props.put("path.separator.applet", "true");
            props.put("line.separator.applet", "true");
        

            // Set "trustProxy" to true so that plug-in will trust the proxy to perform
            // DNS lookup and return materials to plug-in from trusted hosts. Specifically,
            // if the client is behind the firewall, SocketPermission.implies will work
            // correctly if that flag is set to true. See java.net.SocketPermission code
            // for more details.
            props.put("trustProxy", "true");
            
            if (Config.installDeployRMIClassLoaderSpi()) {
                props.put("java.rmi.server.RMIClassLoaderSpi",
                        "sun.plugin2.applet.JNLP2RMIClassLoaderSpi");
            }

            // Install new protocol handler
            String pkgs = (String) props.getProperty("java.protocol.handler.pkgs");
            if (pkgs != null)
                props.put("java.protocol.handler.pkgs", pkgs + "|sun.plugin.net.protocol|com.sun.deploy.net.protocol");
            else
                props.put("java.protocol.handler.pkgs", "sun.plugin.net.protocol|com.sun.deploy.net.protocol");

            //DeployPerfUtil.put("Applet2Environment.initialize() - post property setup (1.2)");

            // Set allow default user interaction in HTTP/HTTPS
            java.net.URLConnection.setDefaultAllowUserInteraction(true);

            // Set default SSL handshaking protocols to SSLv3 and SSLv2Hello
            // because some servers may not be able to handle TLS. #46268654.
            //
            // Set only if users hasn't set it manually.
            //
            // Added options to enable/disable SSLv2/SSLv3/TLSv1 in Control Panel.  
            //
            if (props.get("https.protocols") == null){
                StringBuffer protocolsStr = new StringBuffer();
                if (Config.getBooleanProperty(Config.SEC_TLS_KEY)){
                    protocolsStr.append("TLSv1");
                }
                if (Config.getBooleanProperty(Config.SEC_SSLv3_KEY)){
                    if (protocolsStr.length() != 0){
                        protocolsStr.append(",");
                    }
                    protocolsStr.append("SSLv3");        
                }
                if (Config.getBooleanProperty(Config.SEC_SSLv2_KEY)){
                    if (protocolsStr.length() != 0){
                        protocolsStr.append(",");
                    }
                    protocolsStr.append("SSLv2Hello");
                }

                props.put("https.protocols", protocolsStr.toString()); 
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - post property setup (1.3)");

            // Add new system property for proxy authentication
            props.put("http.auth.serializeRequests", "true");

            //DeployPerfUtil.put(t1, "Applet2Environment.initialize() - post property setup (1.X)");

            new Thread(new Runnable() {
                    public void run() {
                        // Set up the console controller for the trace environment first
                        sun.plugin.JavaRunTime.initTraceEnvironment(controller);

                        // Look in the user choices if we should open the Java Console
                        String consoleStartup = Config.getProperty(Config.CONSOLE_MODE_KEY);

                        if ((Config.CONSOLE_MODE_SHOW).equalsIgnoreCase(consoleStartup)) {
			    JavawsConsoleController jcc = JavawsConsoleController.getInstance();
			    // if there's a java console created by Java Web Start, dispose it
			    // before creating one from Java Plug-in because the java console
			    // from Java Plug-in has classloader related options
			    if ((jcc != null) && (jcc.getConsole() != null)) {
				jcc.getConsole().dispose();
			    }
			    //This is when we want full size Java Console.
			    sun.plugin.JavaRunTime.showJavaConsole(true);

                        } else if( (Config.CONSOLE_MODE_DISABLED).equalsIgnoreCase(consoleStartup)) {
                            // We do not want the Console at all, so don't start it!
                        } else {
                            // Hide Java Console
                            BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();

                            // Start console in hidden state if console cannot be
                            // accessible through browser menu. Otherwise, delay 
                            // console loading until users access the menu
                            //
                            if (service.isConsoleIconifiedOnClose()) {
                                // Start console in hidden/iconified state
                                sun.plugin.JavaRunTime.showJavaConsole(false);
                            }
                        }  

                        try {
                            JavaTrayIcon.install(new JavaTrayIconController() {
                                    public boolean isJavaConsoleVisible() {
                                        return JavaRunTime.isJavaConsoleVisible();
                                    }

                                    public void showJavaConsole(boolean visible) {
                                        JavaRunTime.showJavaConsole(visible);
                                    }
                                });
                        } catch (Throwable t) {
                            t.printStackTrace();
                        }

                        // show help information in Java Console.
                        sun.plugin.JavaRunTime.appendStringToConsole(ConsoleHelper.displayHelp());

                        // Hook up console trace listener so
                        // System.out output goes to the console
                        sun.plugin.JavaRunTime.installConsoleTraceListener();
                    }
                }).start();

            //DeployPerfUtil.put("Applet2Environment.initialize() - post initTraceEnvironment()");

            // Set the default connection timeout value
            // to 120 seconds (2 minutes)
            String timeout = (String) props.getProperty("sun.net.client.defaultConnectTimeout", "120000");

            // Connection timeout not already set, so set it
            props.put("sun.net.client.defaultConnectTimeout", timeout);

            try {
                Class c = Class.forName("sun.misc.ExtensionDependency");
                if (c != null) {
                    Class[] parms = new Class[1];
                    parms[0] = Class.forName("sun.misc.ExtensionInstallationProvider");
                    java.lang.reflect.Method m = c.getMethod("addExtensionInstallationProvider", parms);
                    if (m != null) {
                        Object[] args = new Object[1];
                        args[0] = new sun.plugin.extension.ExtensionInstallationImpl();
                        m.invoke(null, args);
                    } else {
                        Trace.msgPrintln("optpkg.install.error.nomethod");
                    }
                } else {
                    Trace.msgPrintln("optpkg.install.error.noclass");
                }
            } catch(Throwable e) {
                Trace.printException(e);
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - post sun.misc.ExtensionDependency");

            // Remove Proxy Host & Port
            props.remove("proxyHost");
            props.remove("proxyPort");
            props.remove("http.proxyHost");
            props.remove("http.proxyPort");
            props.remove("https.proxyHost");
            props.remove("https.proxyPort");
            props.remove("ftpProxyHost");
            props.remove("ftpProxyPort");
            props.remove("ftpProxySet");
            props.remove("gopherProxyHost");
            props.remove("gopherProxyPort");
            props.remove("gopherProxySet");
            props.remove("socksProxyHost");
            props.remove("socksProxyPort");
        
            // Enable proxy/web server authentication
            if ("true".equalsIgnoreCase(props.getProperty("javaplugin.proxy.authentication",
                                                          "true"))) {
                java.net.Authenticator.setDefault(new com.sun.deploy.security.DeployAuthenticator());
            }

            // Install a property list.
            System.setProperties(props);

            //DeployPerfUtil.put("Applet2Environment.initialize() - post property setup (2)");

            // Reset offline manager
            com.sun.deploy.net.offline.DeployOfflineManager.reset();    
        
            //DeployPerfUtil.put("Applet2Environment.initialize() - post DeployOfflineManager.reset()");

	    // Before creating DeployCacheHandler, initializing the security system
	    // by calling java.security.Policy.getPolicy().
	    // This is to prevent calling into createTempFile recursively
            AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return Policy.getPolicy();
                }
            } );

            // init cache
            // this is to make sure the static block of com.sun.deploy.cache.Cache
            // gets executed in the correct thread group; so that the cache 
            // clean-up thread will get created in the correct thread group
            Class cache = com.sun.deploy.cache.Cache.class;
            //DeployPerfUtil.put("Applet2Environment.initialize() - post DeployCacheHandler static init");

            try {
                // Set deploy cache handler        
                com.sun.deploy.cache.DeployCacheHandler.reset();
            } catch (Throwable t) {
                // The ResponseCache class was only introduced in JDK 5, so
                // this failure is expected on 1.4.2
                // FIXME: not sure if we need to implement any backward compatibility mechanism here
            }

            //DeployPerfUtil.put("Applet2Environment.initialize() - post DeployCacheHandler.reset()");

            // Install security manager
            Applet2SecurityManager security = new Applet2SecurityManager();
            System.setSecurityManager(security);

            //DeployPerfUtil.put("Applet2Environment.initialize() - post Applet2SecurityManager()");

            // Info: Create and install a socket factory is done in PluginMain()

            // All initialization is done ..

            // validate system cache directory
            Config.validateSystemCacheDirectory();      

            // Set the interface to call back to plugin.
            URLJarFile.setCallBack(new PluginURLJarFileCallBack());

            // Install hooks for notification of display of system dialogs
            UIFactory.setDialogHook(dialogHook);

            //DeployPerfUtil.put("Applet2Environment.initialize() - post UIFactory.setDialogHook()");

            // Prompt User for JavaUpdate Enabling, if Windows platform 
            if (System.getProperty("os.name").indexOf("Windows") != -1) {
                UpdateCheck.showDialog();
                //DeployPerfUtil.put("Applet2Environment.initialize() - post UpdateCheck.showDialog()");
            }

            // Upgrade old (1.4.2/5.0) plugin cache to 6.0 format
            // this needs to be run in the plugin system thread group, because
            // it will show the cache upgrade progress dialog
            if (Config.getBooleanProperty(Config.JAVAPI_UPDATE_KEY)) { 
                //DeployPerfUtil.put("Applet2Environment.initialize() - pre CacheUpdateHelper.updateCache()");
                try {
                    DeploySysRun.execute(new DeploySysAction() {                
                            public Object execute() throws Exception {
                                if (CacheUpdateHelper.updateCache()) { 
                                    Config.setBooleanProperty(Config.JAVAPI_UPDATE_KEY, false); 
                                    Config.storeIfDirty(); 
                                }
                                return null;
                            }
                        });
                } catch (Exception e) {
                    Trace.printException(e);
                }
                //DeployPerfUtil.put("Applet2Environment.initialize() - post CacheUpdateHelper.updateCache()");
            }

            try {
                sun.awt.DesktopBrowse.setInstance(new sun.awt.DesktopBrowse() {
                        public void browse(URL url) {
                            AppletContext ac = (AppletContext) (
                                AppContext.getAppContext().get(
                                    Applet2Manager.APPCONTEXT_APPLETCONTEXT_KEY));
                            if (ac != null) {
                                ac.showDocument(url);
                            }
                        }
                    });
            } catch (Throwable t) {
                Trace.ignored(t);
            }

            DeployPerfUtil.put(t0, "Applet2Environment.initialize() - END");
        }
    }
}
