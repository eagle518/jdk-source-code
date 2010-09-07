/*
 * @(#)AppletViewer.java	1.249 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A recipient to show a Java Applet or a JavaBeans component in.
 * This class implements all platform independent behaviour of showing
 * an applet in one version of the Java Activator.
 *
 * @version 	1.3
 *
 */

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Frame;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.PopupMenu;
import java.awt.MenuItem;
import java.awt.RenderingHints;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FilenameFilter;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ObjectInputStream;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.MalformedURLException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.HashMap;
import java.util.Set;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.Provider;
import java.security.Security;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.applet.AppletEvent;
import sun.applet.AppletListener;
import sun.applet.AppletPanel;
import sun.applet.AppletClassLoader;
import sun.applet.AppletThreadGroup;
import sun.awt.AppContext;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.javascript.JSContext;
import sun.plugin.cache.JarCacheUtil;
import sun.plugin.cache.JarCacheVersionException;
import sun.plugin.cache.CacheUpdateHelper;
import com.sun.deploy.net.proxy.DynamicProxyManager;
import sun.plugin.security.PluginClassLoader;
import sun.plugin.services.BrowserService;
import sun.plugin.perf.PluginRollup;
import sun.plugin.util.UserProfile;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.ErrorDelegate;
import sun.plugin.util.GrayBoxPainter;
import sun.net.www.protocol.jar.URLJarFile;
import com.sun.java.browser.net.ProxyService;
import com.sun.java.browser.net.ProxyServiceProvider;
import com.sun.deploy.config.Config;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.ConsoleHelper;
import com.sun.deploy.util.UpdateCheck;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.MemoryCache;
import com.sun.deploy.util.DeployAWTUtil;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;

public class AppletViewer extends AppletPanel implements WindowListener
{
    private GrayBoxPainter grayBoxPainter = new GrayBoxPainter(this);
    private GrayBoxListener grayBoxListener = null;
    private String customBoxMessage = null;
    
    private boolean dumpPerf = false;
    private boolean loading_first_time=true;
    private boolean preloading = false;
    private volatile boolean stopped = false;
    private volatile boolean stopLoadJar = false;

    private static Frame dummyFrame = new Frame();
    private static String APPCONTEXT_APPLETCONTEXT_KEY = "AppletContextKey";
    
    /*
     * are we initialized
     */
    private static boolean initialized = false;
    
    /**
     * Some constants...
     */
    public static String theVersion = "1.1";

    private URL documentURL = null;
    private String documentURLString = null;
    protected URL baseURL = null;
    
    // Parameters handler
    protected java.util.HashMap atts = new java.util.HashMap();
    
    private ClassLoaderInfo cli = null;
    
    // Exception dialogboxes enable/disable status
    private static boolean fShowException = false;

    public static void loadPropertiesFiles() 
    {        
        DeployPerfUtil.put("START - Java   - JVM - AppletViewer.loadPropertiesFiles");

	try
	{
	    File temp = new File(UserProfile.getPropertyFile());
	    // ensure the props folder can be made
	    temp.getParentFile().mkdirs();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}

        DeployPerfUtil.put("END   - Java   - JVM - AppletViewer.loadPropertiesFiles");
    }

    /**
     *	activex control load time is recorded for 
     *	performance analysis
     */	
    public static void setStartTime(long time) {
	String perfProp = System.getProperty("sun.perflog");
	//set the startup time if the performance logger is enabled
	if(perfProp != null ) {
	    try {
		Class perfLogger = Class.forName("sun.misc.PerformanceLogger");
		if (perfLogger != null) {
		    Class [] params = new Class[2];
		    params[0] = String.class;
		    params[1] = Long.TYPE;
		    Method setMethod = perfLogger.getMethod("setStartTime", params);
		    if(setMethod != null) {
			Object [] args = new Object[2];
			args[0] = "Java Plug-in load time";
			args[1] = new Long(time);
			setMethod.invoke(null, args);
		    }
		}
	    } catch(Exception exc) {
		//exc.printStackTrace();
		//Ignore exceptions
	    }
	}
    }

    /**
     * Prepare the enviroment for executing applets.
     */
    public static void initEnvironment(int browserType, long startTime){
	if (initialized)
            return;

	setStartTime(startTime);
	initEnvironment(browserType);
    }
    
    /**
     * Prepare the enviroment for executing applets.
     */
    public static void initEnvironment(int browserType)
    {
        // if we are already initialized, just return
        if (initialized)
            return;
        
        initialized=true;

        DeployPerfUtil.put("START - Java   - ENV - AppletViewer.initEnvironment");

	// to make sure plugin thread group is child of main thread group
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - PluginSysUtil.getPluginThreadGroup");
	PluginSysUtil.getPluginThreadGroup();
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - PluginSysUtil.getPluginThreadGroup");

        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - ServiceManager.setService");
        com.sun.deploy.services.ServiceManager.setService(browserType);
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - ServiceManager.setService");

	// This is a hack to fix the memory leak problem in Swing.
	// Apparently, when these classes are loaded, it will create
	// Component objects that are stored in static variables.
	// However, each Component object bound to the caller's
	// AppContext. Thus, if these classes are first used by
	// any applet, the applet's AppContext will never be freed.
	// We fix this by loading these classes in the main
	// AppContext, so we won't leak anything.
	//
	try
	{
	    Class a = javax.swing.ImageIcon.class;
	}
	catch (Throwable e)
	{
    
	}

        
        try
        {
            // Initialize the ClassFileTransformer for JDK 1.1
            // class file.
            DeployPerfUtil.put("START - Java   - ENV - " +
			       "AppletViewer.initEnvironment - JDK11ClassFileTransformer.init");
            sun.plugin.security.JDK11ClassFileTransformer.init();
            DeployPerfUtil.put("END   - Java   - ENV - " +
			       "AppletViewer.initEnvironment - JDK11ClassFileTransformer.init");
        }
        catch (Throwable e)
        {
            e.printStackTrace();
        }

        DeployPerfUtil.put("START - Java   - ENV - AppletViewer.initEnvironment - load deploy.properties");

	// Load deployment configuration properties
        Properties configProps = Config.getProperties();

        // Load System props (includes deployment props now)
        Properties props = new Properties(System.getProperties());
        
        // Define a number of standard properties
        props.put("acl.read", "+");
        props.put("acl.read.default", "");
        props.put("acl.write", "+");
        props.put("acl.write.default", "");
        
        // Standard browser properties
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
        
        // Install new protocol handler
        String pkgs = (String) props.getProperty("java.protocol.handler.pkgs");
        if (pkgs != null)
            props.put("java.protocol.handler.pkgs", pkgs + "|sun.plugin.net.protocol|com.sun.deploy.net.protocol");
        else
            props.put("java.protocol.handler.pkgs", "sun.plugin.net.protocol|com.sun.deploy.net.protocol");

        // Set allow default user interaction in HTTP/HTTPS
        DeployPerfUtil.put("START - Java   - ENV - " +
		       "AppletViewer.initEnvironment - URLConnection.setDefaultAllowUserInteraction");
        java.net.URLConnection.setDefaultAllowUserInteraction(true);
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - URLConnection.setDefaultAllowUserInteraction");

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

	// Add new system property for proxy authentication
	props.put("http.auth.serializeRequests", "true");
	
	// Initialize tracing environment
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - JavaRunTime.initTraceEnvironment");
        sun.plugin.JavaRunTime.initTraceEnvironment();
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - JavaRunTime.initTraceEnvironment");

        // Look in the user choices if we should open the Java Console
        String consoleStartup = Config.getProperty(Config.CONSOLE_MODE_KEY);

        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - display JavaConsole");

        if( (Config.CONSOLE_MODE_SHOW).equalsIgnoreCase(consoleStartup))
        {
            //This is when we want full size Java Console.
            sun.plugin.JavaRunTime.showJavaConsole(true);
        }
        else if( (Config.CONSOLE_MODE_DISABLED).equalsIgnoreCase(consoleStartup))
        {
            // We do not want the Console at all, so don't start it!

        }
        else
        {
	    // Hide Java Console
	    BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();

	    // Start console in hidden state if console cannot be
	    // accessible through browser menu. Otherwise, delay 
	    // console loading until users access the menu
	    //
	    if (service.isConsoleIconifiedOnClose())
	    {
		// Start console in hidden/iconified state
		sun.plugin.JavaRunTime.showJavaConsole(false);
	    }
        }  

        sun.plugin.JavaRunTime.installConsoleTraceListener();

        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - display JavaConsole");
                
        //Check whether the exception dialogboxes are allowed
        if ("true".equalsIgnoreCase(props.getProperty(Config.SHOW_EXCEPTIONS_KEY, "false"))) {
            fShowException = true;
        }
        
        // Set the default connection timeout value
        // to 120 seconds (2 minutes)
        String timeout = (String) props.getProperty("sun.net.client.defaultConnectTimeout", "120000");
            
        // Connection timeout not already set, so set it
        props.put("sun.net.client.defaultConnectTimeout", timeout);

        DeployPerfUtil.put("START - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - install extension package");
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
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - install extension package");

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
            DeployPerfUtil.put("START - Java   - ENV - " +
			       "AppletViewer.initEnvironment - enable proxy/web server authentication");
            java.net.Authenticator.setDefault(new com.sun.deploy.security.DeployAuthenticator());
            DeployPerfUtil.put("END   - Java   - ENV - " + 
			       "AppletViewer.initEnvironment - enable proxy/web server authentication");
        }

        // Install a property list.
        System.setProperties(props);

        DeployPerfUtil.put("END   - Java   - ENV - AppletViewer.initEnvironment - load deploy.properties");

        System.out.println("");
        
        // Reset proxy selector
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - DeployProxySelector.reset");
	com.sun.deploy.net.proxy.DeployProxySelector.reset();
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - DeployProxySelector.reset");

	// Reset cookie selector
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - DeployCookieSelector.reset");
	com.sun.deploy.net.cookie.DeployCookieSelector.reset();    
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - DeployCookieSelector.reset");

	// Reset offline manager
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - DeployOfflineManager.reset");
	com.sun.deploy.net.offline.DeployOfflineManager.reset();    
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - DeployOfflineManager.reset");
	
	System.out.println("");
        
        // init cache
        // this is to make sure the static block of com.sun.deploy.cache.Cache
        // gets executed in the correct thread group; so that the cache 
        // clean-up thread will get created in the correct thread group
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - DeployCacheHandler.reset");

        Class cache = com.sun.deploy.cache.Cache.class;

	// Set deploy cache handler        
	com.sun.deploy.cache.DeployCacheHandler.reset();

        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - DeployCacheHandler.reset");

        System.out.println("");
        
        // Install progress monitor for progress bar support
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - ProgressMonitor.setDefault");
	sun.net.ProgressMonitor.setDefault(new sun.plugin.util.ProgressMonitor());
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - ProgressMonitor.setDefault");
	
        // Install security manager, so everyone feels
        // warm and fuzzy.  Hold me, the way I need to be held.
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - install security manager");
        sun.plugin.security.ActivatorSecurityManager security = new sun.plugin.security.ActivatorSecurityManager();
        System.setSecurityManager(security);
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - install security manager");

        // REMIND: Create and install a socket factory!

        // All initialization is done, show help information in Java Console.
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - ConsoleHelper.displayHelp");
        System.out.println(ConsoleHelper.displayHelp());
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - ConsoleHelper.displayHelp");

        // validate system cache directory
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - Config.validateSystemCacheDirectory");
        Config.validateSystemCacheDirectory();      
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - Config.validateSystemCacheDirectory");

        // Set the interface to call back to plugin.
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - URLJarFile.setCallBack");
        URLJarFile.setCallBack(new PluginURLJarFileCallBack());       
        DeployPerfUtil.put("END   - Java   - ENV - " + 
		           "AppletViewer.initEnvironment - URLJarFile.setCallBack");

	// Prompt User for JavaUpdate Enabling, if Windows platform 
 	if (System.getProperty("os.name").indexOf("Windows") != -1)
 	{
            DeployPerfUtil.put("START - Java   - ENV - " +
			       "AppletViewer.initEnvironment - show update message");
 	    UpdateCheck.showDialog();
            DeployPerfUtil.put("END   - Java   - ENV - " +
			       "AppletViewer.initEnvironment - show update message");
 	}

	// Upgrade old (1.4.2/5.0) plugin cache to 6.0 format
	// this needs to be run in the plugin system thread group, because
	// it will show the cache upgrade progress dialog
        DeployPerfUtil.put("START - Java   - ENV - " +
		           "AppletViewer.initEnvironment - upgrade cache");
	try {
	    DeploySysRun.executePrivileged(new DeploySysAction() {                
                public Object execute() {

		    if (Config.getBooleanProperty(Config.JAVAPI_UPDATE_KEY)) { 
			if (CacheUpdateHelper.updateCache()) { 
			    Config.setBooleanProperty(Config.JAVAPI_UPDATE_KEY, false); 
			    Config.storeIfDirty(); 
			}
		    }

		    return null;
		}
	    }, null);

	} catch (Exception e) {
	    Trace.printException(e);
	}
        DeployPerfUtil.put("END   - Java   - ENV - " +
		           "AppletViewer.initEnvironment - upgrade cache");

	try {
            sun.awt.DesktopBrowse.setInstance(new sun.awt.DesktopBrowse() {
                public void browse(URL url) {
	            AppletContext ac = (AppletContext) (
                        AppContext.getAppContext().get(
                            APPCONTEXT_APPLETCONTEXT_KEY));
                    if (ac != null) {
                        ac.showDocument(url);
                    }
                }
            });
        } catch (Throwable t) {
            com.sun.deploy.util.Trace.ignored(t);
        }

        DeployPerfUtil.put("END   - Java   - ENV - AppletViewer.initEnvironment");
    }	
    
    /**
     * Construct a new applet viewer.
     * Restricted to subclasses for security reasons,
     */
    public AppletViewer()
    {
        // Don't set it to light gray!  User could specify color, then gray will blink in
        //the beginning - not pretty.
        //setBackground(Color.lightGray);
    }

    private static class AppletEventListener implements AppletListener
    {
        public void appletStateChanged(AppletEvent evt) {
	    AppletViewer src = (AppletViewer) evt.getSource();
            switch (evt.getID()) {
                case AppletPanel.APPLET_RESIZE: {
                    if(src != null)
                    {
                        Object obj = src.getViewedObject();
                        if (obj instanceof Component)
                        {
                            src.setSize(src.getSize());
                            ((Component)obj).setSize(src.getSize());
                            src.validate();
                        }
                    }
		    break;
                }
		case AppletPanel.APPLET_LOADING_COMPLETED: {
		    src.notifyLoadingDone();
		    break;
		}
            }
        }
    }
    


    private AppletEventListener appletEventListener = new AppletEventListener();
    
    private Object syncInit = new Object();
    private boolean bInit = false;

    /**
     * Init the applet, called once to start the download of the applet
     */
    public void appletInit()
    {
        if (! Config.isConfigValid()) {
            UIFactory.showErrorDialog(null, 
	                ResourceManager.getString("launcherrordialog.brief.message.applet"),
                        ResourceManager.getString("enterprize.cfg.mandatory.applet",
                                                   Config.getEnterprizeString()),
                        ResourceManager.getString("error.default.title.applet"));
            //notify loading is aborted 
	    notifyLoadingDone();
            return;
        }

	//Do not initialize applet if classloader creation fails because of 
        //not able to get codebase
	if(createClassLoader()) {
	    initApplet();
	} else {
	    assert (status == AppletPanel.APPLET_DISPOSE
                 || status == AppletPanel.APPLET_ERROR
                 || status == AppletPanel.APPLET_QUIT);

	    notifyLoadingDone();
	}
    }
    
    private final static String VERSION_TAG = "version=";
    
    private HashMap jarVersionMap = new HashMap();
    private HashMap preloadJarMap = new HashMap();
    private ArrayList newStyleJarList = new ArrayList();
    
    private final static String PRELOAD = "preload";
    
    /*
     * initialize the jar version/preload map
     *
     */
    private void initJarVersionMap() {
        int i = 1;
        String archive_tag_value = getParameter("archive_" + i);
        String tokenString;
        if (archive_tag_value != null) {
            // process new style tags
            while (archive_tag_value != null) {               
                // process the archive tag string                               
                // parse the string and extract tokens separated by comma
                StringTokenizer archiveTok = new StringTokenizer(archive_tag_value, ",", false);
                String jarName = null;
                String version = null;
                boolean preloadJar = false;
                while(archiveTok.hasMoreTokens()){
                    tokenString = archiveTok.nextToken().trim();
                    
                    // first argument must be jar name
                    if (jarName == null) {
                        jarName = tokenString;
                    } else if (tokenString.toLowerCase().startsWith(VERSION_TAG)) {
                        version = tokenString.substring(VERSION_TAG.length());
                    } else if (tokenString.toLowerCase().equals(PRELOAD)) {
                        preloadJar = true;
                    }
                }
                
                if (jarName != null) {
                    if (preloadJar) {
                        preloadJarMap.put(jarName, version);
                    }
                    jarVersionMap.put(jarName, version);
                    newStyleJarList.add(jarName);
                }
                i++;
                archive_tag_value = getParameter("archive_" + i);
            }
        } else {
            // process old style tags
            String jpi_archive = getParameter("cache_archive");
            String jpi_version = getParameter("cache_version");
            String jpi_archive_ex = getParameter("cache_archive_ex");
            try {
                // use old style tags, cache_archive, cache_version and cache_archive_ex
                jarVersionMap = JarCacheUtil.getJarsWithVersion(jpi_archive, jpi_version,
                        jpi_archive_ex);
            } catch (Exception ex) {
                Trace.printException(ex, ResourceManager.getMessage("cache.error.text"),
                        ResourceManager.getMessage("cache.error.caption"));
            }
            
            // Figure out which JAR files still need to be loaded.
            if (jpi_archive_ex != null) {
                StringTokenizer jarTok = new StringTokenizer(jpi_archive_ex, ",", false);
                while (jarTok.hasMoreTokens()) {
                    String elem = jarTok.nextToken().trim();
                    int optionIndex = elem.indexOf(';');
                    if(optionIndex != -1) {
                        String optionString = elem.substring(optionIndex);
                        if(optionString.toLowerCase().indexOf(PRELOAD) != -1) {
                            String jarFileName = elem.substring(0, optionIndex);
                            preloadJarMap.put(jarFileName, null);
                        }
                    }
                }
            }
        }
    }
    
    private void storeJarVersionMapInAppContext() {
        // iterate the jarVesrionMap and store into Applet AppContext      
        Iterator iter = jarVersionMap.keySet().iterator();
        while(iter.hasNext()) {
            String jarName = (String)iter.next();
            String jarVersion = (String)jarVersionMap.get(jarName);
            URL url = null;
            try {
                // this will resolve a/abc/../ to a/ in the url
                url = new URL(getCodeBase(), jarName);
            } catch (MalformedURLException mue) {
                // should not happen
                com.sun.deploy.util.Trace.ignoredException(mue);
            }
            if (url != null) {
                AppContext.getAppContext().put(Config.getAppContextKeyPrefix() +
                        url.toString(), jarVersion);
            }
        }
    }

    /**
     * Create ClassLoader, ThreadGroup and AppContext
     * This method is used to get or create ClassLoader 
     * for an applet codebase and increament ClassLoaderInfo's
     * references count
     *
     * This method is called early in creating embedded frame
     *
     * @return AppletClassLoader
     *
     */
    public AppletClassLoader getAppletClassLoader() {
        AppletClassLoader loader = null;

        URL cbUrl = getCodeBase();
        if (cbUrl == null) {
            System.err.println("ERROR: unexpectedly couldn't get the codebase");
            return null;
        }

        //Create the applet class loader and force it to create its thread group
        ClassLoaderInfo info =
             ClassLoaderInfo.find(cbUrl, getClassLoaderCacheKey());
        if (info == null) {
            System.err.println("ERROR: unexpectedly couldn't get the ClassLoaderInfo for the codebase/cache key");
        } else {
            //set AppletViewer's field cli
            cli = info;

            //create or get class loader, threadgroup, AppContext
            //and addReference() in ClassloaderInfo
            loader = info.grabClassLoader();

            URL documentBase = getDocumentBase();

            if (documentBase == null ||
                    documentBase.getProtocol().equalsIgnoreCase("file") == false ||
                    URLUtil.isUNCFileURL(documentBase)) {
                // do not allow recursive codebase direcotry read if:
                // 1.  documentBase is null
                // 2.  documentBase is not file protocol url
                // 3.  documentBase url is UNC file url
                loader.disableRecursiveDirectoryRead();
            }

        }

        return loader;
    }
   
    /**
     * Create the classloader and threadgroup if they have not been 
     * created.
     *
     * Create Applet thread 
     */
    public boolean createClassLoader()
    {   
        DeployPerfUtil.put("START - Java   - ENV - AppletViewer.createClassLoader");

        // Add applet event listener
        addAppletListener(appletEventListener);
        
        initJarVersionMap();
     
	URL cbUrl = getCodeBase();
	if(cbUrl == null) {
	    return false;
	}
        
        try{
         
            if(!jarVersionMap.isEmpty())
                verifyJarVersions(cbUrl, getClassLoaderCacheKey(), jarVersionMap);
            
            
        } catch(Exception ex) {
            Trace.printException(ex, ResourceManager.getMessage("cache.error.text"),
            ResourceManager.getMessage("cache.error.caption"));
        }

        // Add applet into AppletContext
        appletContext.addAppletPanelInContext(this);
        
        // We must synchronized the class because
        // super.init() will create classloader -
        // and we need the creation to be synchronized
        //
        synchronized(AppletViewer.class)
        {
            super.init();
        }

        DeployPerfUtil.put("END   - Java   - ENV - AppletViewer.createClassLoader");

	return true;
    }

    private void verifyJarVersions(URL codeBase, String key, HashMap jarFileVersionMap)
		    throws IOException, JarCacheVersionException {
    	boolean markClassLoader = false;
	//If versions are specified for all the jar files, try to mark
	//the cached files 
	Iterator iter = jarFileVersionMap.keySet().iterator();
	while(iter.hasNext()) {
	    String jarFileName = (String)iter.next();
	    String jarFileVersion = (String)jarFileVersionMap.get(jarFileName);
	    URL url = new URL(codeBase, jarFileName);

            Trace.msgNetPrintln("cache.version_checking", new Object[] {jarFileName, jarFileVersion});
            // only check versioned resource
            if (jarFileVersion != null) {

                String cacheVersion = Cache.getCacheEntryVersion(url, null);

                // if there's a cached jar file, check if the version of the cached jar satisfies the required version
	        if (cacheVersion != null && cacheVersion.compareTo(jarFileVersion) < 0) {

                    // there is no cached jar that matches the required
                    // version
                    markClassLoader = true;
                }
            }
	}

	//If one of the jar file has been modified, clear the classloader since 
	//it is no longer correct to use it.
	if(markClassLoader == true) {
	    ClassLoaderInfo.markNotCachable(codeBase, key);
	}
    }	

    /*
     * Override super class createAppletThread to not grab the class loader
     * reference since we now grab it when creating embedded frame
     */
    protected synchronized void createAppletThread() {

        String nm = "applet-" + getCode();

        //get the threadgroup for this applet panel
        //the threadgroup was created when create the embedded frame
        ThreadGroup appletGroup = loader.getThreadGroup();

        handler = new Thread(appletGroup, this, "thread " + nm);

        // set the context class loader for this thread
        AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    handler.setContextClassLoader(loader);
                    return null;
                }
            });
        handler.start();

    }

    /*
     * Helper method to set the AppletClassLoader in the super class.
     */
    public void setLoader(AppletClassLoader acl) {
	loader = acl;
    }

    /**
     * Load & Initialize the applet
     */
    public void initApplet()
    {
        DeployPerfUtil.put("START - Java   - ENV - AppletViewer.initApplet");

        // After super.init() is called, applet handler thread is
        // created. Added applet thread group into tracing.
        //
        Thread t = getAppletHandlerThread();

	// Setup animation in graybox  (only the first time)
	if (!bInit) {
            // Set up the error delegate
            grayBoxPainter.setErrorDelegate(new ErrorDelegate() {
                    public void handleReloadApplet() {
                        ClassLoaderInfo.clearClassLoaderCache();
                        getAppletContext().showDocument(getDocumentBase());
                    }

                    public String getCodeBase() {
                        return AppletViewer.this.getCodeBase().toString();
                    }

                    public void addJarFilesToSet(Set/*<String>*/ jarSet) {
                        String jars = getJarFiles();
                        if (jars != null){
                            // Jar files are separated by "," 
                            StringTokenizer jarTok = 
                                new StringTokenizer(jars, ",", false);
                            int jarCount = jarTok.countTokens();

                            for(int i=0; i<jarCount; i++) {
                                String jarFileName = jarTok.nextToken().trim();
                                jarSet.add(jarFileName);                            
                            }
                        }
                    }
                });

	    // See if user specified special image to be shown 
	    String user_image = getParameter("image");
	    if (user_image != null) {
	        try {
		    URL customImageURL = new URL(getCodeBase(), user_image);
                    boolean centerImage = Boolean.valueOf(getParameter("centerimage")).booleanValue();
                    grayBoxPainter.setBoxBorder(getParameter("boxborder"));
		    grayBoxPainter.setCustomImageURL(customImageURL, centerImage);
	        } catch (MalformedURLException e) {
		    e.printStackTrace();
	        } 	    
	    }     
	        
	    // Get a list of jar files
	    grayBoxPainter.setProgressFilter(getCodeBase(), getJarFiles());
	    grayBoxPainter.beginPainting(t.getThreadGroup());
    
	    // Add mouse listener                
	    grayBoxListener = new GrayBoxListener(this, customBoxMessage);
            addMouseListener(grayBoxListener);
        } else {
	    grayBoxPainter.resumePainting();
	}

        Trace.msgPrintln("applet.progress.load");
        sendEvent(sun.applet.AppletPanel.APPLET_LOAD);
    }

    public void sendAppletInit() {

        Trace.msgPrintln("applet.progress.init");
        sendEvent(sun.applet.AppletPanel.APPLET_INIT);
        
        // Mark viewer as initialized
        synchronized(syncInit)
        {
            bInit = true;
        }

        DeployPerfUtil.put("END   - Java   - ENV - AppletViewer.initApplet");
    }

    /**
     * Tell loader thread to stop loading
     * Override
     */
      public void stopLoading() {
          stopLoadJar = true;

          if (status == APPLET_LOAD) {
              Trace.msgPrintln("applet.progress.stoploading");
	      super.stopLoading();
          }
      }

    /**
     * Start the applet.
     */
    public void appletStart()
    {
        // Do nothing if viewer hasn't been initialized
        synchronized(syncInit)
        {
            if (bInit == false)
                return;
        }
	if (stopped) {
	    // when we first start, we need to wait till showAppletStatus()
	    // to turn off animation.  On a restart, we will never get 
	    // to showAppletStatus with (status >= APPLET_START)
	    // so we suspend here.
            final GrayBoxPainter gbp = grayBoxPainter;
            suspendGrayBoxPainting(gbp);
	    stopped = false;
	}

        Trace.msgPrintln("applet.progress.start");

	try {
            DeployPerfUtil.write(new PluginRollup(), false);
	    Trace.println("completed perf rollup", TraceLevel.BASIC);
	    dumpPerf = true;
	}
	catch (IOException ioe) {
	    // ignore exception
	}

        sendEvent(sun.applet.AppletPanel.APPLET_START);
    }
    
    /**
     * Stop the applet.
     */
    public void appletStop()
    {
	stopped = true;

        // Do nothing if viewer hasn't been initialized
        synchronized(syncInit)
        {
            if (bInit == false)
                return;
        }
        
        if (grayBoxPainter != null)
        {
            //may be called on main thread in Netscape plugin
            final GrayBoxPainter gbp = grayBoxPainter;
            suspendGrayBoxPainting(gbp);
        }

        if (status==APPLET_LOAD)
        {
            stopLoading();
        }

        Trace.msgPrintln("applet.progress.stop");
        sendEvent(sun.applet.AppletPanel.APPLET_STOP);
    }

    private void suspendGrayBoxPainting(final GrayBoxPainter gbp) {
        //suspendPainting need get Awt Treelock
        //it may block the main thread. Use invokeLater

        //let's look for the tartget component
        Component target = null;
        if (getApplet() != null) {
            //use applet
            target = (Component)getApplet();
        } else {
            if (getParent() != null) {
                //panel has added to frame. use frame
                target = (Component)getParent();
            } else {
                //use panel. AppContext is main
                target = (Component) this;
            }
        }
        DeployAWTUtil.invokeLater(target,
            new Runnable() {
                public void run() {
                    gbp.suspendPainting();
                }
            }
        );
    }

    /* used for sync loader and main thread 
     */
    private boolean loadingDone = false;
    private final Object syncLoading = new Object();

    public void notifyLoadingDone() {
        synchronized (syncLoading) {
            loadingDone = true;
            syncLoading.notify();
        }
    }
		   
    public void waitForLoadingDone(long timeout) {
        try {
            synchronized (syncLoading) {
                while (!loadingDone) {
                    syncLoading.wait(timeout);
                }
            }
        } catch (InterruptedException e) {
            //Trace.printException(e);
        } catch (Exception e) {
            Trace.printException(e);
        }
    }

    /**
     * Notification that the applet is being closed
     *
     */
    public void appletDestroy()
    {
	stopped = true;

        // Destroy gray box painter
        if (grayBoxPainter != null)
        {
            //this is called on the shutdown thread
            //it is ok to not use invokeLater
            grayBoxPainter.finishPainting();
            grayBoxPainter = null;
        }

	// Remove graybox listener
	if (grayBoxListener != null)
	{
    	    removeMouseListener(grayBoxListener);
	    grayBoxListener = null;
	}

	Trace.msgPrintln("applet.progress.destroy");
	sendEvent(APPLET_DESTROY);

	Trace.msgPrintln("applet.progress.dispose");
	sendEvent(APPLET_DISPOSE);
    }

    //cleanup when plugin object is not initialized
    //Applet panel has not add reference to classloaderinfo
    //and has not created applet thread
    public void miniCleanup() {
        // Remove applet event listeners
        removeAppletListener(appletEventListener);
        appletEventListener = null;

        // Remove applet from the context first, so it won't be called
        if (appletContext != null) {
            appletContext.removeAppletPanelFromContext(this);
            appletContext = null;
        }
    }

    public void cleanup() {
        miniCleanup();
         
        Trace.msgPrintln("applet.progress.joining");
        joinAppletThread();
        Trace.msgPrintln("applet.progress.joined");
    }

    public void joinAppletThread() {
        Thread appletThread = getAppletHandlerThread();

        if (appletThread != null) {
            try {
                appletThread.join(1000);
            } catch (Exception e) {
                Trace.printException(e);
            }

            //if the applet thread is still alive after timeout, interrupt it
            if (appletThread.isAlive()) {
                appletThread.interrupt();
            }   
        }
    }

    public void release() {
        if (cli == null ) {
            cli = ClassLoaderInfo.find(getCodeBase(), getClassLoaderCacheKey());
        }

        AppletClassLoader loader = cli.getLoader();
        ThreadGroup group = (loader == null)? null : loader.getThreadGroup();

        PluginClassLoader pcl = null;
        AppContext context = null;

        // Additional synchronization on ClassLoaderInfo needed
        // to prevent reference count from being incremented in
        // e.g. ClassLoaderInfo.grabClassLoader() during
        // PluginClassLoader.resetAppContext() because the method may 
        // mark ClassLoader's AppContext and ThreadGroup to be destroyed 
        synchronized (cli) {
            // Release classloader reference
            Trace.msgPrintln("applet.progress.findinfo.0");
            int refCount = cli.removeReference();
            Trace.msgPrintln("applet.progress.findinfo.1");

            if (refCount > 0) {
                //do nothing
                return;
            }

            if (loader != null && loader instanceof PluginClassLoader) {
                pcl = (PluginClassLoader)loader;

                Trace.msgPrintln("applet.progress.quit");
                context = pcl.resetAppContext();
            }
        }

        //dispose AppContext outside the synchronized block
        pcl.release(context);

        long startTime = System.currentTimeMillis();
        while (group != null && group.activeCount() > 0
                       && System.currentTimeMillis() - startTime < 5000) {
            group.stop();
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) { }
        }

        cli = null;    
    }

    /**
     * Pre-refresh the applet
     */
    public void preRefresh()
    {
	// Clear cache files and jars in memory
        MemoryCache.clearLoadedResources();

	// Mark the classloader as non-cachable
        if (cli != null)    
	{
	    cli.markNotCachable(getCodeBase(), getClassLoaderCacheKey());
	}
    }
    
    /**
     * Get an applet parameter.
     */
    public String getParameter(String name) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        
        synchronized(atts)
        {
            String value = (String) atts.get(name);
            
            if (value != null)
                value = trimWhiteSpaces(value);
            
            return value;
        }
    }
    
    /**
     * Set an applet parameter.
     */
    public void setParameter(String name, Object value) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        
        synchronized(atts)
        {
            atts.put(name, trimWhiteSpaces(value.toString()));
        }
    }
    
    /**
     * Trim whitespaces
     */
    private String trimWhiteSpaces(String str)
    {
        if (str == null)
            return str;
        
        StringBuffer buffer = new StringBuffer();
        
        for (int i=0; i < str.length(); i++)
        {
            char c = str.charAt(i);
            
            // Skip over whitespaces
            if (c == '\n' || c == '\f' || c == '\r' || c == '\t')
                continue;
            else
                buffer.append(c);
        }
        
        // Trim whitespaces on both ends of the strings
        return buffer.toString().trim();
    }
    
    private boolean docbaseInit = false;
    private Object docBaseSyncObj = new Object();
    
    /**
     * Get the document url.
     */
    public void setDocumentBase(String url)
    {
        if (docbaseInit == false)
        {
	    // Canonicalize URL if necessary
            String urlString = URLUtil.canonicalize(url);
                      
	    // delay constructing the URL - 
	    // URL constructed later in geDocumentBase 
	    documentURLString = canonicalizeDocumentURL(urlString); 

            docbaseInit = true;
            
            // Notify all the pending getDocumentBase() calls
            synchronized(docBaseSyncObj)
            {
                docBaseSyncObj.notifyAll();
            }
        }
    }
    
    /**
     * Canonicalize URL.
     */
    public String canonicalizeDocumentURL(String url) {
        
        int fromIndex=-1,lastIndex;
        
        // Strip off "#" and "?" from URL
        int fragmentIndex = url.indexOf('#');
        int queryIndex = url.indexOf('?');
        
        if(queryIndex != -1 && fragmentIndex != -1)
        {
            fromIndex = Math.min(fragmentIndex, queryIndex);
        }
        else if(fragmentIndex != -1)
        {
            fromIndex = fragmentIndex;
        }
        else if(queryIndex != -1)
        {
            fromIndex = queryIndex;
        }
        
        // Strip off the end of the URL
        String strippedURL;
        
        if (fromIndex == -1)
            strippedURL = url;
        else
            strippedURL = url.substring(0, fromIndex);
        
        
        // Replace "|" character with ":" 
        StringBuffer urlBuffer = new StringBuffer(strippedURL);
        int index = urlBuffer.toString().indexOf("|");
        if (index >= 0)
        {
            urlBuffer.setCharAt(index, ':'); 
        }
        
        if (fromIndex != -1)
	    urlBuffer.append(url.substring(fromIndex));
       
	return urlBuffer.toString();
			     
    }
    
    
    /**
     * Get the document url.
     */
    public URL getDocumentBase()
    {
        Object syncObject = new Object();
        
        synchronized(syncObject)
        {
            if (docbaseInit == false)
            {
                BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
                
                if (service.isNetscape() && service.getBrowserVersion() >= 5.0f)
                {
                    // If we are running in Netscape 6.x, we MUST wait for setDocumentBase()
                    // to get the document base.
                    try
                    {
                        synchronized(docBaseSyncObj)
                        {
                            while (docbaseInit == false)
                                docBaseSyncObj.wait(0);
                        }
                    }catch (InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
                else
                {
                    // Otherwise, obtain the document base directly through JSObject
                    //
                    JSContext jsCtx = (JSContext) getAppletContext();
                    
                    try
                    {
                        JSObject win = (JSObject) jsCtx.getJSObject();
                        if (win == null)
                            throw new JSException("Unable to obtain Window object");
                        
                        JSObject doc = (JSObject) win.getMember("document");
                        if (doc == null)
                            throw new JSException("Unable to obtain Document object");
                        
                        String url = (String) doc.getMember("URL");
                        
			// Canonicalize URL in case the URL is in some
			// weird form only recognized by the browsers
			//
                        String docBase = URLUtil.canonicalize(url);
                                            
			// initialize policy up-front
			AccessController.doPrivileged(new PrivilegedAction() {
			    public Object run() {
				return java.security.Policy.getPolicy();
			    }
			});

                        documentURL = new URL(canonicalizeDocumentURL(docBase));
                    }
                    catch (Throwable e)
                    {
			Trace.println(e.getMessage(), TraceLevel.BASIC);
			return null;
                    }

                    docbaseInit = true;
                }
            }
        }
        
	if (documentURL == null) {
	    // documentURL not constructed yet -
	    // first initialize policy.
 
	    AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    return java.security.Policy.getPolicy();
		}
	    });
 
            try {
		if (documentURLString == null) {
		    throw new IllegalStateException("documentURLString null");
		}
		documentURL = new URL(documentURLString);
	    } catch (Throwable e) {
		e.printStackTrace();
	    }
	}

        return documentURL;
    }
    
    protected boolean codeBaseInit = false;
    
    
    /**
     * Get the base url.
     */
    public URL getCodeBase()
    {
        Object syncObject = new Object();
        
        synchronized(syncObject)
        {
            if (codeBaseInit == false)
            {
                String att = getParameter("java_codebase");

                if (att == null)
                    att = getParameter("codebase");

		URL docBase = getDocumentBase();
		if(docBase == null)return null;

		if (att != null) 
		{
		    if (!att.equals(".") && !att.endsWith("/")) 
		    {
			att += "/";
		    }

		    // Canonicalize URL in case the URL is in some
		    // weird form only recognized by the browser
		    //
		    att = URLUtil.canonicalize(att);

		    try {
			baseURL = new URL(docBase, att);
		    } catch (MalformedURLException e) {
			// ignore exception
		    }
		}

		if (baseURL == null) 
		{
                    String urlString = docBase.toString();
                    int i = urlString.indexOf('?'); 
                    if (i > 0) { 
                        urlString = urlString.substring(0, i); 
                    } 
                    i = urlString.lastIndexOf('/');
		    if (i > -1 && i < urlString.length() - 1) {
			try 
			{
			    // Canonicalize URL to ensure it is well-formed
			    baseURL = new URL(URLUtil.canonicalize(urlString.substring(0, i + 1)));
			} 
			catch (MalformedURLException e) 
			{
			    // ignore exception
			}
		    }

		    // when all is said & done, baseURL shouldn't be null
		    if (baseURL == null)
			baseURL = docBase;
		}
                
                codeBaseInit = true;
            }
        }
        
        return baseURL;
    }
    
    /**
     * Get the width.
     */
    public int getWidth() {
        String w = getParameter("width");
        if (w != null) {
            return Integer.valueOf(w).intValue();
        }
        return 0;
    }
    
    /**
     * Get the height.
     */
    public int getHeight() {
        String h = getParameter("height");
        if (h != null) {
            return Integer.valueOf(h).intValue();
        }
        return 0;
    }
    
    /**
     * Get initial_focus
     */
    public boolean hasInitialFocus() {    
        // 6234219: Do not set initial focus on an applet 
        // during startup if applet is targeted for 
        // JDK 1.1/1.2. [stanley.ho]
        if (isJDK11Applet() || isJDK12Applet())
            return false;
                
        String initialFocus = getParameter("initial_focus");

        if (initialFocus != null) {
            if (initialFocus.toLowerCase().equals("false")) 
                return false;
        }

        if (Config.getInstance().isNativeModalDialogUp())
            return false;

        return true;
    }

    public void updateHostIPFile(String codebase) {
        Cache.updateHostIPFile(codebase);
    }
    
    /**
     * Get the code parameter
     */
    public String getCode() {
        // Support HTML 4.0 style of OBJECT tag.
        //
        // <OBJECT classid=java:sun.plugin.MyClass .....>
        // <PARAM ....>
        // </OBJECT>
        //
        // In this case, the CODE will be inside the classid
        // attribute.
        //
        String moniker = getParameter("classid");
        String code = null;
        
        if (moniker != null)
        {
            int index = moniker.indexOf("java:");
            
            if (index > -1)
            {
                code = moniker.substring(5 + index);
                
                if (code != null || !code.equals(""))
                    return code;
            }
        }
        
        code = getParameter("java_code");
        if (code==null)
            code=getParameter("code");
        return code;
    }
    
    /**
     * Return true if the applet requires legacy lifecycle
     */
    public boolean isLegacyLifeCycle()
    {
        String lifecycle = getParameter("legacy_lifecycle");
        
        if (lifecycle != null && lifecycle.equalsIgnoreCase("true"))
            return true;
        else
            return false;
    }
    
    
    private String classLoaderCacheKey = null;
    
    /**
     * Return the caching key for classloader
     */
    public String getClassLoaderCacheKey()
    {
        // Fixed #4516442 - Switch for classloader policy.
        //
        // If classic classloader policy is set to true
        //
        String param = getParameter("classloader-policy");
        
        if (param != null && param.equals("classic"))
            return super.getClassLoaderCacheKey();
        
        // This method is responsible for determining
        // the classloader sharing policy.
        //
        // Classloader is shared between applets if
        // and only if
        //
        // 1. codebase's values are the same
        // 2. list of jar files are the same
        //
        if (classLoaderCacheKey == null)
        {
            StringBuffer buffer = new StringBuffer();
            
            buffer.append(getCodeBase());
            
            String jarFilesList = getJarFiles();
            
            if (jarFilesList != null) {
                buffer.append(",");
                buffer.append(jarFilesList);
            }
            
            classLoaderCacheKey = buffer.toString();
        }
        
        return classLoaderCacheKey;
    }
    
    private synchronized static String getJarsInCacheArchiveEx(String cacheArchiveEx){
        if (cacheArchiveEx == null) {
            return null;
        }
        
        String jarFiles = "";
        StringTokenizer jarTok = new StringTokenizer(cacheArchiveEx, ",", false);
        int jarCount = jarTok.countTokens();
        
        for(int i=0;i<jarCount;i++) {
            String elem = jarTok.nextToken().trim();
            int index = elem.indexOf(";");
            if(index != -1) {
                String jarFileName = elem.substring(0, index);
                jarFiles += jarFileName;
                jarFiles += (i != jarCount-1)?",":"";
            }
        }
        
        return jarFiles;

    }
    
    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     */
    public String getJarFiles() {
        
        StringBuffer result = null;
        
        // jar listed under new archive_* tag
        if (newStyleJarList.isEmpty() == false) {         
            // use new style tags archive_*
            Iterator jarItr = newStyleJarList.iterator();
            while(jarItr.hasNext()) {
                if (result == null) {
                    result = new StringBuffer();
                }
                String jarFileName = (String)jarItr.next();
                result.append(jarFileName);
                if (jarItr.hasNext()) {
                    result.append(",");
                }
            }
            return addJarFileToPath(result == null ? null : result.toString(),
                    null);
        }
        
        // old style tags
        // Figure out the list of all required JARs.
        String archive = getParameter("archive");
        String java_archive = getParameter("java_archive");
        String jpi_archive = getParameter("cache_archive");
        String jpi_archive_ex = getParameter("cache_archive_ex");
        String archive_ex = null;
        
        if(jpi_archive_ex != null) {
            int index = jpi_archive_ex.indexOf(";");
            if(index != -1) {
                archive_ex = getJarsInCacheArchiveEx(jpi_archive_ex);
            } else {
                archive_ex = jpi_archive_ex;
            }
        }
        
        return addJarFileToPath( archive_ex,
                addJarFileToPath(jpi_archive,
                addJarFileToPath(java_archive, archive)));
    }
    
    private String addJarFileToPath(String original, String jar)
    {
        if(original == null && jar == null)
        {
            return null;
        }
        
        if(original == null && jar != null)
        {
            return jar;
        }
        
        if(original != null && jar == null)
        {
            return original;
        }
        
        return original + "," + jar;
    }
    
   /*
    *  Allow pre-loading of local .jar files in plug-in lib/app directory
    *  These .jar files are loaded with the PluginClassLoader so they
    *  run in the applet's sandbox thereby saving developers the trouble
    *  of writing trusted support classes.
    *  The ClassLoaderInfo cli should be locked.
    */
    private void loadLocalJarFiles(PluginClassLoader loader, String basePath)
    {
        File dir = new File(basePath);

        if (dir.exists())  
	{
            String[] jarList = dir.list(new FilenameFilter()  
	    {
                public boolean accept(File f, String s)  {
                    return(s.endsWith(".jar"));
                }
            });
            
            for (int i = 0; i < jarList.length; i++)  
	    {
		try 
		{
		    URL localJarUrl = (new File(basePath + File.separator + jarList[i])).toURI().toURL(); 

		    loader.addLocalJar(localJarUrl);
		}
		catch (MalformedURLException e)
		{
		    e.printStackTrace();
		}
            }
        }
    }  
    
    
    protected void setupAppletAppContext() {
        storeJarVersionMapInAppContext();
	AppContext.getAppContext().put(Config.APPCONTEXT_APP_NAME_KEY, getName());
	AppContext.getAppContext().put(APPCONTEXT_APPLETCONTEXT_KEY, 
            getAppletContext());
    }
    
    /*
     * We overload our parent loadJarFiles so tht we can avoid
     * reloading JAR files that are already loaded.
     *						KGH Mar 98
     */
    protected void loadJarFiles(AppletClassLoader loader)
    throws IOException, InterruptedException
    {
        if (loading_first_time)
        {
            loading_first_time=false;
   
            try{
                //pre-load jar files are loaded first                
                preloading = true;
                JarCacheUtil.preload(getCodeBase(), preloadJarMap);
                preloading = false;
                
            } catch(Exception ex) {
                Trace.printException(ex, ResourceManager.getMessage("cache.error.text"),
                ResourceManager.getMessage("cache.error.caption"));
            }
        }
        
        // Get a list of jar files
        String archive = getJarFiles();

        //save the cli on the stack. In case, the main thread destroyPlugin 
        //set the cli to null
        ClassLoaderInfo loaderInfo = cli;

        if (loaderInfo == null ) {
            return;
        }
        
        try
        {
            // Prevent two applets trying to load JARS from the same
            // classloader at the same time.
            loaderInfo.lock();
            
            if (!loaderInfo.getLocalJarsLoaded() && (loader instanceof PluginClassLoader) )
            {
		String fSep = File.separator;
		String appletDir = System.getProperty("java.home") + fSep + 
				    "lib" + fSep + "applet";
		loadLocalJarFiles((PluginClassLoader) loader, appletDir);

		//Add the jar files from system wide untrusted directory
		if(Config.getOSName().equalsIgnoreCase("Windows")) {
		    String untrustDir = Config.getSystemHome() + fSep + "Lib" + 
					fSep + "Untrusted";
		    loadLocalJarFiles((PluginClassLoader) loader, untrustDir);
		}
                loaderInfo.setLocalJarsLoaded(true);
            }
            
            // If there are no JARs, this is easy.
            if (archive == null)
            {
                return;
            }
            
            // Figure out which JAR files still need to be loaded.
            StringTokenizer st = new StringTokenizer(archive, ",", false);
            while(st.hasMoreTokens())
            {
                String tok = st.nextToken().trim();
                if (loaderInfo.hasJar(tok))
                {
                    continue;
                }
                
                loaderInfo.addJar(tok);
            }
            
            //do the actual jar loading here
            String jarFiles = getJarFiles();
            st = new StringTokenizer(jarFiles, ",", false);
            while(!stopLoadJar && st.hasMoreTokens()) {
                //stopLoadJar can only stop adding jar to ucp 
                //need find where the real downloading happens

                String tok = st.nextToken().trim();
                try {
                    ((PluginClassLoader)loader).addJar(tok);
                } catch (IllegalArgumentException e) {
                    continue;
                }
            }
        }
        finally
        {
            // Other people can load JARs now.
            loaderInfo.unlock();

            // reset stopLoadJar to false, other people can load JARs
            stopLoadJar = false;
        }
    }
    
    
  /**
   * Return the value of the object param
   */
    public String getSerializedObject() {
        String object = getParameter("java_object");
        if (object==null)
            object=getParameter("object");// another name?
        return object;
    }
    
    public Applet getApplet() {
        Applet applet = super.getApplet();
        if (applet!=null) {
            if (applet instanceof BeansApplet)
                return null;
            else
                return applet;
        }
        return null;
    }
    
    
    /*
     * return the viewed object
     */
    public Object getViewedObject() {
        Applet applet = super.getApplet();
        if (applet instanceof BeansApplet)
            return ((BeansApplet) applet).bean;
        else
            return applet;
    }
    
    
    /**
     * Set the applet context. For now this is
     * also implemented by the AppletPanel class.
     */
    public void setAppletContext(AppletContext appletContext) {
        
        if (appletContext==null)
            throw new IllegalArgumentException("AppletContext");
        
        if (this.appletContext != null)
            this.appletContext.removeAppletPanelFromContext(this);
        
        this.appletContext = (PluginAppletContext) appletContext;
    }
    
    /**
     * Get the applet context. For now this is
     * also implemented by the AppletPanel class.
     */
    public AppletContext getAppletContext() {
        return appletContext;
    }
    

    /**
     * Implement getParent() method, return null when caller doesn't have
     * AllPermission, which prevent caller get to AppletContext level.
     */
    /*public Container getParent()
    {
        Container con = null;
     
        try {
            Permission perm = new AllPermission();
            AccessControlContext acc = AccessController.getContext();
            acc.checkPermission(perm);
            con = super.getParent();
        }
        catch(Throwable e) {
            con = null;
        }
     
        return con;
    }*/
    
    /*
     * See if user specified the background color for the applet's gray box
     * or a foreground color and set these values for the applet.
     */
    public void setColorAndText()
    {
        Color val = null;
        /*
         * See if user specified any colors
         *      BOXBGCOLOR - background color for the gray box
         *      BOXFGCOLOR - foreground color for the gray box
         *      BOXMESSAGE - user-defined message for the applet viewer.
         * If not - use defaults - light gray for background, black for foreground,
         * and purple for the progress bar color.
         */
        String boxBGColorStr = getParameter("boxbgcolor");
        if (boxBGColorStr != null){
            val = createColor("boxbgcolor", boxBGColorStr);
            
	    /*
	     * If user specified valid color, set background color.
	     */
	    if (val != null)
		grayBoxPainter.setBoxBGColor(val);
        }

        setBackground(grayBoxPainter.getBoxBGColor());
        
        String boxFGColorStr = getParameter("boxfgcolor");
        if(boxFGColorStr != null){
            val = createColor("boxfgcolor", boxFGColorStr);
            
            /*
             * If user specified valid color, set foreground color.
             */
            if (val != null)
		grayBoxPainter.setBoxFGColor(val);
	}
        
        customBoxMessage = getParameter("boxmessage");

	if (customBoxMessage != null)
	    grayBoxPainter.setWaitingMessage(customBoxMessage);
	else
	    grayBoxPainter.setWaitingMessage(getWaitingMessage());

    }
    
    /*
     * Create color
     * @param html_tag - name of the html tag to use in tracing message
     * @param str - string representation of color.  It should consist three digits
     *              separated by commas, or one of standard colors.
     *
     * Returns color or null(if number of components for RGB color was incorrect,
     * or if standard color was spelled incorrectly).
     */
    private Color createColor(String html_tag, String str)
    {
        if (str != null && str.indexOf(",") != -1)
        {
            // This is the RGB format.  Tokenize the string.
            StringTokenizer st = new StringTokenizer(str, ",");
            if (st.countTokens()==3)
            {
                // We've got three components for the color.
                int i=0;
                int red=0, green=0, blue=0;
                while (st.hasMoreTokens())
                {
                    String token = (String)st.nextElement();
                    switch(i){
                        case 0:
                        {
                            if(! token.trim().equals(""))
                                red = new Integer(token.trim()).intValue();
                            break;
                        }
                        case 1:
                        {
                            if (! token.trim().equals(""))
                                green = new Integer(token.trim()).intValue();
                            break;
                        }
                        case 2:
                        {
                            if(! token.trim().equals(""))
                                blue = new Integer(token.trim()).intValue();
                            break;
                        }
                    }
                    i++;
                }
                return new Color(red, green, blue);
            }
            else
            {
                Trace.msgPrintln("applet_viewer.color_tag", new Object[] {html_tag}, TraceLevel.BASIC);
                return null;
            }
            
        }
        else if(str != null)
        {
            //Check & decode if the color is in hexadecimal color format (i.e. #808000)
            try {
                return Color.decode(str);
            }
            catch (NumberFormatException e) {
                // ignore exception
            }
            
            //This is a string representation of color
            if(str.equalsIgnoreCase("red"))
                return Color.red;
            if(str.equalsIgnoreCase("yellow"))
                return Color.yellow;
            if(str.equalsIgnoreCase("black"))
                return Color.black;
            if(str.equalsIgnoreCase("blue"))
                return Color.blue;
            if(str.equalsIgnoreCase("cyan")
            || str.equalsIgnoreCase("aqua"))
                return Color.cyan;
            if(str.equalsIgnoreCase("darkGray"))
                return Color.darkGray;
            if(str.equalsIgnoreCase("gray"))
                return Color.gray;
            if(str.equalsIgnoreCase("lightGray")
            || str.equalsIgnoreCase("silver"))
                return Color.lightGray;
            // green is Java std #00ff00 not w3 HTML std.
            // w3 HTML std - lime is #00ff00 & green is #008000
            if(str.equalsIgnoreCase("green")
            || str.equalsIgnoreCase("lime"))
                return Color.green;
            if(str.equalsIgnoreCase("magenta")
            || str.equalsIgnoreCase("fuchsia"))
                return Color.magenta;
            if(str.equalsIgnoreCase("orange"))
                return Color.orange;
            if(str.equalsIgnoreCase("pink"))
                return Color.pink;
            if(str.equalsIgnoreCase("white"))
                return Color.white;
            if(str.equalsIgnoreCase("maroon"))
                return new Color(128, 0, 0);
            if(str.equalsIgnoreCase("purple"))
                return new Color(128, 0, 128);
            if(str.equalsIgnoreCase("navy"))
                return new Color(0, 0, 128);
            if(str.equalsIgnoreCase("teal"))
                return new Color(0, 128, 128);
            if(str.equalsIgnoreCase("olive"))
                return new Color(128, 128, 0);
        }
        
        /*
         * Misspelling?
         */
        return null;
    }
    
    /**
     * Paint this panel while visible and loading an applet to entertain
     * the user.  paint() is called by AWT to repaint this component.
     *
     * @param g the graphics context
     */
    public void paint(Graphics g)
    {
        Dimension d = getSize();
        
        if (d.width > 0 && d.height > 0 &&
        (status == APPLET_LOAD || status == APPLET_INIT || status == APPLET_ERROR))
        {
            paintForegrnd(g);
        }
        else
        {
            super.paint(g);
        }
    }

	// Fix bug #4671775. If the foreground color is not set, set to white
	// to avoid NullPointerException inside Java2D code.
	public Color getForeground() {
		Color c = super.getForeground();
		if(null == c)
			c = Color.BLACK;

		return c;
	}
    
    /**
     * Paint this panel while visible and loading an applet to entertain
     * the user.  This method will be called by update() when repainting by request
     * from GrayBoxPainter and by paint() when repainting by request from AWT.
     *
     * @param g the graphics context
     */
    public void paintForegrnd(Graphics g)
    {
    	if (grayBoxPainter != null)
	    grayBoxPainter.paintGrayBox(this, g);    
    }
    
    public String getWaitingMessage() 
    {
	if (status == APPLET_ERROR)
	{
	    return getMessage("failed");
	}
	else
	{
	    MessageFormat mf = new MessageFormat(getMessage("loading"));
	    return mf.format(new Object[] {getHandledType()});
	}
    }
    
    
    /*
     * <p>
     * Load an applet from a serialized stream. This is likely to happen
     * when the user uses the Back/Forward buttons
     * </p>
     *
     * @param is Input stream of the serialized applet
     */
    protected void load(java.io.InputStream is) {
        this.is = is;
    }
    
    protected Applet createApplet(AppletClassLoader loader) throws ClassNotFoundException,
    IllegalAccessException, IOException, InstantiationException, InterruptedException {
        
        // If we are not in a reload...
        if (is==null)
            return super.createApplet(loader);
        
        ObjectInputStream ois =
        new AppletObjectInputStream(is, loader);
        Object serObject = ois.readObject();
        Applet applet = (Applet) serObject;
        doInit = false; // skip over the first init
        if (Thread.interrupted()) {
            try {
                status = APPLET_DISPOSE; // APPLET_ERROR?
                applet = null;
                // REMIND: This may not be exactly the right thing: the
                // status is set by the stop button and not necessarily
                // here.
                showAppletStatus("death");
            } finally {
                Thread.currentThread().interrupt(); // resignal interrupt
            }
            return null;
        }
        is = null;
        return applet;
    }
    /*
     * @return the applet name
     */
    public String getName()  {
        
        String name = getParameter("name");
        if (name!=null)
            return name;
        
        // Remove .class extension
        name = getCode();
        if (name != null){
            int index = name.lastIndexOf(".class");
            if (index != -1)
                name = name.substring(0, index);
        } else{
            // Remove .object extension
            name = getSerializedObject();
            
            if (name != null) {
                int index = name.lastIndexOf(".ser");
                if (index != -1)
                    name = name.substring(0, index);
            }
        }
        
        return name;
    }
    
    public static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
    
    /**
     * @return the java component displayed by this viewer class
     */
    protected String getHandledType() {
        return getMessage("java_applet");
    }
    
    private AppletStatusListener statusListener = null;
    
    public void addAppletStatusListener(AppletStatusListener listener)
    {
        statusListener = listener;
    }
    
    public void removeAppletStatusListener(AppletStatusListener listener)
    {
        statusListener = null;
    }
    
    public void setStatus(int status) {
        this.status = status;
    }
    
    public void showAppletLog(String msg) {
        super.showAppletLog(msg);
    }
    
    public boolean isStopped() {
	return stopped;
    }

    public void showAppletStatus(String msg) {
        
        if (msg != null && !msg.equals("") && !msg.equals("\n"))
        {
            String name = getName();
	    MessageFormat mf = new MessageFormat(getMessage("status_applet"));
           
	    if (appletContext != null) { 
                if (name != null && !msg.equals(""))
                    appletContext.showStatus(mf.format(new Object[] {name, msg}));
                else
                    appletContext.showStatus(mf.format(new Object[] {msg, ""}));
            }

            // We no longer need the gray box processer if the applet has started
            if (grayBoxPainter != null)
            {
		if (status == APPLET_ERROR)
		{
		    grayBoxPainter.showLoadingError();
		} else if (status >= APPLET_START) {
		    grayBoxPainter.suspendPainting();
		}
	    }
				                
            if (statusListener != null)
                statusListener.statusChanged(status);            
        }
    }

    //override other showAppletStatus methods in superclass to avoid NPE
    protected void showAppletStatus(String msg, Object arg) {
        if (appletContext != null) {
            super.showAppletStatus(msg, arg);
        }
        
        //status is APPLET_ERROR
        if (grayBoxPainter != null && status == APPLET_ERROR) {
            grayBoxPainter.showLoadingError();
        }
    }

    protected void showAppletStatus(String msg, Object arg1, Object arg2) {
        if (appletContext != null) {
            super.showAppletStatus(msg, arg1, arg2);
        }
        
        //status is APPLET_ERROR
        if (grayBoxPainter != null && status == APPLET_ERROR) {
            grayBoxPainter.showLoadingError();
        }
    }       
    
    public void setDoInit(boolean doInit) {
        this.doInit = doInit;
    }
    
    private volatile PluginAppletContext appletContext;
    
    
    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String getMessage(String key)
    {
        return ResourceManager.getMessage(key);
    }
    
    
    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String[] getMessageArray(String key)
    {
        return ResourceManager.getMessageArray(key);
    }
    
    private java.io.InputStream is;
    
    /**
     * This method actually creates an AppletClassLoader.
     *
     * It can be override by subclasses (such as the Plug-in)
     * to provide different classloaders. This method should be
     * called only when running inside JDK 1.2.
     */
    protected AppletClassLoader createClassLoader(final URL codebase) {
        return ClassLoaderInfo.find(codebase, getClassLoaderCacheKey()).getLoader();
    }
    
    /**
     * Called by the AppletPanel to provide
     * feedback when an exception has happened.
     */
    protected void showAppletException(Throwable exc)
    {
        super.showAppletException(exc);
        
        Trace.msgPrintln("exception", new Object[]{exc.toString()}, TraceLevel.BASIC);        
        
        if (fShowException)
        {
            Trace.printException(exc);
        }

	if (grayBoxPainter != null)
	{
	    grayBoxPainter.showLoadingError();
	}
    }
    
    public void showStatusText(String msg)
    {
        // Message should be displayed in the status bar
        //
        if (appletContext != null) {
            appletContext.showStatus(msg);
        }
    }
     
    /*
     * Update graphics.
     *
     * @param g - the graphics context.
     */
    public void update(Graphics g)
    {
        Dimension d = getSize();
        
        if (d.width > 0 && d.height > 0 &&
        (status == APPLET_LOAD || status == APPLET_INIT || status == APPLET_ERROR))
        {
            paintForegrnd(g);
        }
        else
        {
            super.update(g);
        }
    }
        
    /**
     * <p> Obtain the loading status of the applet.
     * </p>
     *
     * @return loading status.
     */
    public int getLoadingStatus()
    {
        return status;
    }
    
    
    // To satisfy the WindowEventListener interface for
    // accessibility to work
    public void windowActivated(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowClosing(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}

    static class GrayBoxListener implements MouseListener
    {
	private String msg = null;
	private AppletViewer av;

	GrayBoxListener(AppletViewer av, String msg)
	{
	    this.msg = msg;
	    this.av = av;
	}


	/*
	 * When pointing at the applet - display status message (or user-defined
	 * message) in browser status field.
	 */
	public void mouseEntered(MouseEvent e) {
	    if (msg != null)
		av.showStatusText(msg);
            else
                av.showStatusText(av.getWaitingMessage());
	}
    
	public void mouseExited(MouseEvent e) {}
	public void mousePressed(MouseEvent e) {}
	public void mouseReleased(MouseEvent e) {}   
	public void mouseClicked(MouseEvent e) {}
    }  

}
