/*
 * @(#)Config.java	1.91 04/04/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.io.*;
import java.util.*;
import java.lang.*;
import java.net.URL;
import java.net.URLConnection;
import java.security.Security;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.panel.PlatformSpecificUtils;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.panel.ControlPanel;
/*
 *
 *  Interface to manipulate the deployment properties file
 */
public class Config {

    // Internally the config properties are stored in a standard JDK property
    // file. We do part-object, instead of inheritance to enforce type access
    // to the properties

    private static Properties _defaultProps = null;
    private static Properties _systemProps = null;

    private static Properties _props = null;
    private static Properties _changedProps = null;
    private static Properties _lockedProps = null;


    // basic infrastucture locations:
    private static String _javaHome = null;
    private static String _userHome = null;
    private static String _systemHome = null;
    private static String _osHome = null;

    // the users configuration file itself:
    private static File _userConfigFile = null;

    // Time that we either last read the config file, or last saved it.
    private static long _lastChanged;

    // If true, the properties have changed.
    private static boolean _dirty;

    private static final String PROPERTIES_FILE = "deployment.properties";
    private static final String CONFIG_FILE     = "deployment.config";

    //
    // Private Partial keys:
    //

    private static final String BASE            = "deployment.";
    private static final String USER		= "deployment.user.";
    private static final String SYSTEM		= "deployment.system.";
    private static final String SEC		= "deployment.security.";
    private static final String USEC		= "deployment.user.security.";
    private static final String SSEC		= "deployment.system.security.";
    private static final String PROX		= "deployment.proxy.";
    private static final String JAVAPI		= "deployment.javapi.";
    private static final String JAVAWS		= "deployment.javaws.";


    //
    // Public property keys and their default values:
    //

    // version control

    public static final String VERSION_UPDATED_KEY  = BASE + "version";
    public static final String VERSION_UPDATE_THIS  = "1.5.0";


    // Infrastructure locations:

    public static final String CACHEDIR_KEY	    = USER + "cachedir";
    public static final String CACHEDIR_DEF	    = "$USER_HOME" +
						File.separator + "cache";

    public static final String SYSCACHE_KEY         = SYSTEM + "cachedir";

    public static final String SEC_TLS_KEY  = SEC + "TLSv1";
    public static final boolean SEC_TLS_DEF = false;

    // no default location for System cache - must be configured

    public static final String JAVAWS_CACHE_KEY	= JAVAWS + "cachedir";
    public static final String JAVAPI_CACHE_KEY = JAVAPI + "cachedir";

    public static final String LOGDIR_KEY	    = USER + "logdir";
    public static final String LOGDIR_DEF	    = "$USER_HOME" +
						File.separator + "log";

    public static final String JAVAWS_TRACEFILE_KEY    = JAVAWS + "traceFileName";
    public static final String JAVAWS_TRACEFILE_DEF    = "";

    public static final String JAVAWS_LOGFILE_KEY    = JAVAWS + "logFileName";
    public static final String JAVAWS_LOGFILE_DEF    = "";


    public static final String TMPDIR_KEY	    = USER + "tmp";
    public static final String TMPDIR_DEF	    = "$USER_HOME" +
						File.separator + "tmp";

    public static final String USR_EXTDIR_KEY	    = USER + "extdir";
    public static final String USR_EXTDIR_DEF	    = "$USER_HOME" +
						File.separator + "ext";

    public static final String SYS_EXTDIR_KEY	    = SYSTEM + "extdir";
    // no default location for System extension cache - must be configured

    //
    // Certificate stores and policy files:
    //

    public static final String USEC_POLICY_KEY	    = USEC + "policy";
    public static final String USEC_POLICY_DEF	    =
				"file://$USER_HOME/security/java.policy";

    public static final String USEC_CACERTS_KEY	    = USEC + "trusted.cacerts";
    public static final String USEC_CACERTS_DEF	    =
				"$USER_HOME" + File.separator + "security" + File.separator + "trusted.cacerts";

    public static final String USEC_JSSECERTS_KEY   = USEC + "trusted.jssecacerts";
    public static final String USEC_JSSECERTS_DEF   =
				"$USER_HOME" + File.separator + "security" + File.separator + "trusted.jssecacerts";

    public static final String USEC_TRUSTED_CERTS_KEY = USEC + "trusted.certs";
    public static final String USEC_TRUSTED_CERTS_DEF =
				"$USER_HOME" + File.separator + "security" + File.separator + "trusted.certs";

    public static final String USEC_TRUSTED_JSSE_CERTS_KEY =
				USEC + "trusted.jssecerts";
    public static final String USEC_TRUSTED_JSSE_CERTS_DEF =
				"$USER_HOME" + File.separator + "security" + File.separator + "trusted.jssecerts";

    public static final String USEC_TRUSTED_CLIENT_CERTS_KEY =
				USEC + "trusted.clientauthcerts";
    public static final String USEC_TRUSTED_CLIENT_CERTS_DEF =
				"$USER_HOME" + File.separator + "security" + File.separator + "trusted.clientcerts";

    public static final String SSEC_POLICY_KEY	    = SSEC + "policy";
    // no default deployment system policy file

    public static final String SSEC_CACERTS_KEY	    = SSEC + "cacerts";
    public static final String SSEC_CACERTS_DEF	    =
				"$JAVA_HOME" + File.separator + "lib" + File.separator + "security" + File.separator + "cacerts";

    public static final String SSEC_JSSECERTS_KEY   = SSEC + "jssecacerts";
    public static final String SSEC_JSSECERTS_DEF   =
				"$JAVA_HOME" + File.separator + "lib" + File.separator + "security" + File.separator + "jssecacerts";

    public static final String SSEC_TRUSTED_CERTS_KEY = SSEC + "trusted.certs";
    public static final String SSEC_TRUSTED_CERTS_DEF =
				"$JAVA_HOME" + File.separator + "lib" + File.separator + "security" + File.separator + "trusted.certs";

    public static final String SSEC_TRUSTED_JSSE_CERTS_KEY =
				SSEC + "trusted.jssecerts";
    public static final String SSEC_TRUSTED_JSSE_CERTS_DEF =
				"$JAVA_HOME" + File.separator + "lib" + File.separator + "security" + File.separator + "trusted.jssecerts";

    public static final String SSEC_TRUSTED_CLIENT_CERTS_KEY =
				SSEC + "trusted.clientauthcerts";
    public static final String SSEC_TRUSTED_CLIENT_CERTS_DEF =
				"$JAVA_HOME" + File.separator + "lib" + File.separator + "security" + File.separator + "trusted.clientcerts";

    //
    // Security Access And Control settings:
    //

    public static final String  SEC_ASKGRANT_SHOW_KEY =
				SEC + "askgrantdialog.show";
    public static final boolean SEC_ASKGRANT_SHOW_DEF = true;

    public static final String  SEC_ASKGRANT_NOTCA_KEY =
				SEC + "askgrantdialog.notinca";
    public static final boolean SEC_ASKGRANT_NOTCA_DEF = true;

    public static final String  SEC_USE_BROWSER_KEYSTORE_KEY =
				SEC + "browser.keystore.use";
    public static final boolean SEC_USE_BROWSER_KEYSTORE_DEF = true;

    public static final String  SEC_NOTINCA_WARN_KEY =
				SEC + "notinca.warning";
    public static final boolean SEC_NOTINCA_WARN_DEF = true;

    public static final String  SEC_EXPIRED_WARN_KEY =
				SEC + "expired.warning";
    public static final boolean SEC_EXPIRED_WARN_DEF = true;

    public static final String  SEC_JSSE_HOST_WARN_KEY =
				SEC + "jsse.hostmismatch.warning";
    public static final boolean SEC_JSSE_HOST_WARN_DEF = true;

    public static final String  SEC_TRUSTED_POLICY_KEY =
				SEC + "trusted.policy";
    public static final String  SEC_TRUSTED_POLICY_DEF = "";

    public static final String  SEC_AWT_WARN_WINDOW_KEY =
				SEC + "sandbox.awtwarningwindow";
    public static final boolean SEC_AWT_WARN_WINDOW_DEF = true;

    public static final String  SEC_SANDBOX_JNLP_ENHANCED_KEY =
				SEC + "sandbox.jnlp.enhanced";
    public static final boolean SEC_SANDBOX_JNLP_ENHANCED_DEF = true;

    public static final String  SEC_AUTHENTICATOR_KEY = SEC + "authenticator";
    public static final boolean SEC_AUTHENTICATOR_DEF = true;


    public static final String  REMOTE_JNLP_DOCS_KEY =
				JAVAWS + "home.jnlp.url";
    public static final String REMOTE_JNLP_DOCS_DEF =
        "http://java.sun.com/products/javawebstart";

    public static final String  REMOTE_J2SE_DOCS_KEY = BASE + "home.j2se.url";
    public static final String  REMOTE_J2SE_DOCS_DEF =
        "http://java.sun.com/j2se/";



    //
    // Networking
    //

    public static final int  PROX_TYPE_UNKNOWN   = -1;
    public static final int  PROX_TYPE_NONE      = 0;
    public static final int  PROX_TYPE_MANUAL    = 1;
    public static final int  PROX_TYPE_AUTO      = 2;
    public static final int  PROX_TYPE_BROWSER   = 3;

    public static final String  PROX_TYPE_KEY       = PROX + "type";
    public static final int     PROX_TYPE_DEF       = PROX_TYPE_BROWSER;

    public static final String  PROX_SAME_KEY       = PROX + "same";
    public static final boolean PROX_SAME_DEF       = false;

    public static final String  PROX_LOCAL_KEY      = PROX + "bypass.local";
    public static final boolean PROX_LOCAL_DEF      = false;

    // no default settings for these:
    public static final String  PROX_AUTOCFG_KEY    = PROX + "auto.config.url";

    public static final String  PROX_BYPASS_KEY     = PROX + "bypass.list";

    public static final String  PROX_HTTP_HOST_KEY  = PROX + "http.host";
    public static final String  PROX_HTTP_PORT_KEY  = PROX + "http.port";
    public static final String  PROX_HTTPS_HOST_KEY = PROX + "https.host";
    public static final String  PROX_HTTPS_PORT_KEY = PROX + "https.port";
    public static final String  PROX_FTP_HOST_KEY   = PROX + "ftp.host";
    public static final String  PROX_FTP_PORT_KEY   = PROX + "ftp.port";
    public static final String  PROX_SOX_HOST_KEY   = PROX + "socks.host";
    public static final String  PROX_SOX_PORT_KEY   = PROX + "socks.port";

    public static final String  PROX_OVERRIDE_KEY   = PROX + "override.hosts";
    public static final String  PROX_OVERRIDE_DEF   = "";

    //
    // Cache and Optional Package Repository:
    //

    // cache size "-1" == unlimited; "0" == disabled.
    public static final String  CACHE_MAX_KEY =
				BASE + "cache.max.size";
    public static final String  CACHE_MAX_DEF = "-1";

    public static final String  CACHE_COMPRESSION_KEY =
				BASE + "cache.jarcompression";
    public static final int     CACHE_COMPRESSION_DEF = 0;

    public static final String  CACHE_ENABLED_KEY =
                                JAVAPI + "cache.enabled";
    public static final boolean CACHE_ENABLED_DEF = true;

    public static final String  OPR_ENABLED_KEY =
				BASE + "repository.enabled";
    public static final boolean OPR_ENABLED_DEF = true;

    public static final String  OPR_ASK_SHOW_KEY =
				BASE + "repository.askdownloaddialog.show";
    public static final boolean OPR_ASK_SHOW_DEF = true;

    public static final String  SPLASH_CACHE_INDEX_KEY = JAVAWS + "splash.index";

    //
    // Java Console:
    //

    public static final String CONSOLE_MODE_HIDE     = "HIDE";
    public static final String CONSOLE_MODE_SHOW     = "SHOW";
    public static final String CONSOLE_MODE_DISABLED = "DISABLE";

    public static final String CONSOLE_MODE_KEY      = BASE + "console.startup.mode";
    public static final String CONSOLE_MODE_DEF      = CONSOLE_MODE_HIDE;

    //
    // Tracing and logging
    //
    public static final String  TRACE_MODE_KEY        = BASE + "trace";
    public static final boolean TRACE_MODE_DEF        = false;

    public static final String  TRACE_LEVEL_KEY  = TRACE_MODE_KEY + ".level";

    public static final String  LOG_MODE_KEY          = BASE + "log";
    public static final boolean LOG_MODE_DEF          = false;

    public static final String  LOG_CP_KEY	      = BASE + "control.panel.log";
    public static final boolean LOG_CP_DEF	      = false;

    public static final String  USE_SYSTEM_LF_KEY = BASE + "system.lookandfeel";


    public static final String JPI_TRACE_FILE_KEY = JAVAPI + "trace.filename";
    public static final String JPI_TRACE_FILE_DEF = "";

    public static final String JPI_LOG_FILE_KEY = JAVAPI + "log.filename";
    public static final String JPI_LOG_FILE_DEF = "";

    public static final String SHOW_EXCEPTIONS_KEY  = JAVAPI + "lifecycle.exception";
    public static final boolean SHOW_EXCEPTIONS_DEF = false;

    //
    // Java Plugin JDK and JRE settings:
    //

    public static final int JPI_RUNTIME_TYPE_JRE = 0;
    public static final int JPI_RUNTIME_TYPE_JDK = 1;

    public static final String JPI_RUNTIME_VER_KEY = JAVAPI + "runtime.version";
    public static final String JPI_RUNTIME_VER_DEF = "";

    public static final String JPI_RUNTIME_TYPE_KEY = JAVAPI + "runtime.type";
    public static final int    JPI_RUNTIME_TYPE_DEF = JPI_RUNTIME_TYPE_JRE;

    // no defaults here. for multiple <version>s you can have:
    // {JPI_JRE_KEY, JPI_JDK_KEY}<version>{JPI_JAVA_PATH, JPI_JAVA_ARGS}

    public static final String JPI_JAVA_PATH 	= ".path";
    public static final String JPI_JAVA_ARGS 	= ".args";
    public static final String JPI_JAVA_OSNAME 	= ".osname";
    public static final String JPI_JAVA_OSARCH 	= ".osarch";
    public static final String JPI_JRE_KEY   	= JAVAPI + "jre.";
    public static final String JPI_JDK_KEY   	= JAVAPI + "jdk.";

    //
    // Java Web Start specific properties:
    //
    // JNLP association
    public static final String ASSOCIATION_MODE_KEY = JAVAWS + "associations";
    public static final String ASSOCIATION_MODE_NEVER = "NEVER";
    public static final String ASSOCIATION_MODE_NEW_ONLY = "NEW_ONLY";
    public static final String ASSOCIATION_MODE_ASK_USER = "ASK_USER";
    public static final String ASSOCIATION_MODE_REPLACE_ASK = "REPLACE_ASK";
    public static final String ASSOCIATION_MODE_DEF = ASSOCIATION_MODE_ASK_USER;
    public static final int ASSOCIATION_NEVER = 0;
    public static final int ASSOCIATION_NEW_ONLY = 1;
    public static final int ASSOCIATION_ASK_USER = 2;
    public static final int ASSOCIATION_REPLACE_ASK = 3;
    //  desktop integration:
    public static final int SHORTCUT_NEVER             = 0;
    public static final int SHORTCUT_NO		       = 0;
    public static final int SHORTCUT_ALWAYS            = 1;
    public static final int SHORTCUT_YES               = 1;
    public static final int SHORTCUT_ASK_USER          = 2;
    public static final int SHORTCUT_ASK_IF_HINTED     = 3;
    public static final int SHORTCUT_ALWAYS_IF_HINTED  = 4;

    public static final String SHORTCUT_MODE_NEVER              = "NEVER";
    public static final String SHORTCUT_MODE_ALWAYS             = "ALWAYS";
    public static final String SHORTCUT_MODE_ASK_USER           = "ASK_USER";
    public static final String SHORTCUT_MODE_ASK_IF_HINTED      = "ASK_IF_HINTED";
    public static final String SHORTCUT_MODE_ALWAYS_IF_HINTED   =
							"ALWAYS_IF_HINTED";
    public static final String SHORTCUT_MODE_KEY = JAVAWS + "shortcut";
    public static final String SHORTCUT_MODE_DEF = SHORTCUT_MODE_ASK_IF_HINTED;

    public static final String SHORTCUT_UNINSTALL_KEY = JAVAWS +"uninstall.shortcut";
    public static final boolean SHORTCUT_UNINSTALL_DEF = false;

    // Java Web Start JRE selections:

    public static final String JAVAWS_JRE_PLATFORM_ID	= ".platform";
    public static final String JAVAWS_JRE_PRODUCT_ID	= ".product";
    public static final String JAVAWS_JRE_LOCATION 	= ".location";
    public static final String JAVAWS_JRE_PATH		= ".path";
    public static final String JAVAWS_JRE_OS_ARCH	= ".osarch";
    public static final String JAVAWS_JRE_OS_NAME	= ".osname";
    public static final String JAVAWS_JRE_ISENABLED	= ".enabled";
    public static final String JAVAWS_JRE_ISREGISTERED	= ".registered";

    // no default here = multiple JAVAWS_JRE_KEY<n><each of above attributes>
    public static final String JAVAWS_JRE_KEY = JAVAWS + "jre.";

    public static final String JAVAWS_JRE_INSTALL_KEY = JAVAWS + "installURL";
    public static final String JAVAWS_JRE_INSTALL_DEF =
				"http://java.sun.com/products/autodl/j2se";

    // Java Web Start JRE management:

    public static final String AUTODOWNLOAD_MODE_ALWAYS = "ALWAYS";
    public static final String AUTODOWNLOAD_MODE_PROMPT = "PROMPT";
    public static final String AUTODOWNLOAD_MODE_NEVER  = "NEVER";

    public static final String JAVAWS_JRE_AUTODOWNLOAD_KEY = JAVAWS + "autodownload";
    public static final String JAVAWS_JRE_AUTODOWNLOAD_DEF = AUTODOWNLOAD_MODE_ALWAYS;

    // this is not a key for our property, but rather for a system property
    public static final String JAUTHENTICATOR_SYSTEM_PROP =
					"javaws.cfg.jauthenticator";

    // Browser selection and path:
    public static final String BROWSER_PATH_KEY = BASE + "browser.path";
    public static final String BROWSER_PATH_DEF = "<No Browser Selected>";

    public static final String EXTENDED_BROWSER_ARGS_KEY = BASE + "browser.args";
    public static final String EXTENDED_BROWSER_ARGS_DEF = "-raise -remote openURL(%u,new-window)";

    // Enable/maintain mime-types for Javaws
    public static final String  CAPTURE_MIME_KEY       =
					BASE + "capture.mime.types";
    public static final boolean CAPTURE_MIME_DEF       = false;

    public static final String  UPDATE_MIME_KEY        =
					BASE + "update.mime.types";
    public static final boolean UPDATE_MIME_DEF        = true;

    public static final String  MIME_DEFAULTS_KEY      =
					BASE + "mime.types.use.default";
    public static final boolean MIME_DEFAULTS_MIME_DEF = true;

    // persistance store maxium:

    public static final String JAVAWS_MUFFIN_LIMIT_KEY = JAVAWS + "muffin.max";
    public static final int    JAVAWS_MUFFIN_LIMIT_DEF = 256;  /* in Kb */


    // Check for update Timeout:

    public static final String  JAVAWS_UPDATE_TIMEOUT_KEY =
				JAVAWS + "update.timeout";
    public static final int     JAVAWS_UPDATE_TIMEOUT_DEF = 500;

    public static final String SECURE_PROPS_KEY    = JAVAWS + "secure.properties";
    // note - should be same list as in native: launchFile.c
    private static final String DefaultSecureProperties [] = {
	    "sun.java2d.noddraw",
	    "javax.swing.defaultlf",
	    "javaws.cfg.jauthenticator",
	    "swing.useSystemFontSettings",
	    "swing.metalTheme",
            "http.agent",
            "http.keepAlive" };

   // note: this list MUST corrispond to native secure.c file
    private static String[] secureVmArgs = {
        "-d32",                         /* use 32-bit data model if available */
        "-client",                      /* to select the "client" VM */
        "-server",                      /* to select the "server" VM */
        "-verbose",                     /* enable verbose output */
        "-version",                     /* print product version and exit */
        "-showversion",                 /* print product version and continue */
        "-help",                        /* print this help message */
        "-X",                           /* print help on non-standard options */
        "-ea",                          /* enable assertions */
        "-enableassertions",            /* enable assertions */
        "-da",                          /* disable assertions */
        "-disableassertions",           /* disable assertions */
        "-esa",                         /* enable system assertions */
        "-enablesystemassertions",      /* enable system assertions */
        "-dsa",                         /* disable system assertione */
        "-disablesystemassertions",     /* disable system assertione */
        "-Xmixed",                      /* mixed mode execution (default) */
        "-Xint",                        /* interpreted mode execution only */
        "-Xnoclassgc",                  /* disable class garbage collection */
        "-Xincgc",                      /* enable incremental gc. */
        "-Xbatch",                      /* disable background compilation */
        "-Xprof",                       /* output cpu profiling data */
        "-Xdebug",                      /* enable remote debugging */
        "-Xfuture",                     /* enable strictest checks */
        "-Xrs",                         /* reduce use of OS signals */
	"-XX:+ForceTimeHighResolution", /* use high resolution timer */
	"-XX:-ForceTimeHighResolution"  /* use low resolution (default) */
    };
    // note: this list MUST corrispond to native secure.c file
    private static String[] secureVmPrefixes = {
        "-ea:",                         /* enable assertions for classes ... */
        "-enableassertions:",           /* enable assertions for classes ... */
        "-da:",                         /* disable assertions for classes ... */
        "-disableassertions:",          /* disable assertions for classes ... */
        "-verbose:",                    /* enable verbose output */
        "-Xms",                         /* set initial Java heap size */
        "-Xmx",                         /* set maximum Java heap size */
        "-Xss",                         /* set java thread stack size */
	"-XX:NewRatio",                 /* set Ratio of new/old gen sizes */
	"-XX:NewSize",                  /* set initial size of new generation */
	"-XX:MaxNewSize",               /* set max size of new generation */
	"-XX:PermSize",                 /* set initial size of permanent gen */
	"-XX:MaxPermSize",              /* set max size of permanent gen */
	"-XX:MaxHeapFreeRatio",         /* heap free percentage (default 70) */
	"-XX:MinHeapFreeRatio",         /* heap free percentage (default 40) */
	"-XX:UseSerialGC",              /* use serial garbage collection */
	"-XX:ThreadStackSize",          /* thread stack size (in KB) */
	"-XX:MaxInlineSize",            /* set max num of bytecodes to inline */
	"-XX:ReservedCodeCacheSize",    /* Reserved code cache size (bytes) */
    };

    private static boolean _configOK = false;

    private static Config _config;

    private static String _os;
    private static String _arch;
    private static String _platform;

    private static ControlPanel _ControlPanel = null;

    public static void setControlPanel(ControlPanel cp) {
	_ControlPanel = cp;
    }


    private static final String _javaVersionProperty = 
	System.getProperty("java.version");

    private static final boolean _atLeast14 = (
	!_javaVersionProperty.startsWith("1.2") &&
	!_javaVersionProperty.startsWith("1.3"));

    private static final boolean _atLeast15 = (_atLeast14 &&
	!_javaVersionProperty.startsWith("1.4"));

    public static boolean isJavaVersionAtLeast15() {
        return _atLeast15; 
    }
        
    public static boolean isJavaVersionAtLeast14() {
        return _atLeast14; 
    }


    /**
     * Return a shared platform specific instance of this class.
     */
    public synchronized static Config getInstance() {
	if (_config == null) {
	    _config = ConfigFactory.newInstance();
	}
	return _config;
    }

    private static String _enterprizeConfig = null;


    private static long _start;
    private static long _end;

    /*
     * Do all initialization statically
     */
    static {
	_javaHome = getJavaHome();
	_userHome = Config.getInstance().getPlatformUserHome();
	_systemHome = Config.getInstance().getPlatformSystemHome();
	_osHome = Config.getInstance().getPlatformOSHome();
	_os = System.getProperty("os.name");
        if (_os.startsWith("Win")) {
            _os = "Windows";
        }
        _arch =  System.getProperty("os.arch");
	_platform = System.getProperty("os.platform");
	_configOK = initialize();
    }

    public static void reInit() {
	initialize();
    }

    public static boolean isConfigValid() {
        return _configOK;
    }

    /*
     * com.sun.deploy.Config.initialize()
     *
     * Read in deployment.config file and deployment.properties files,
     * and establish defaults.
     * Responsible for all version update logic.
     *
     */
    private static boolean initialize() {
	boolean retval = true;

	_defaultProps = getDefaultProps();
	_systemProps = new Properties();

	_userConfigFile = new File(_userHome + File.separator +
				   getPropertiesFilename());

	File config = new File(_systemHome + File.separator + CONFIG_FILE);
	if (!config.exists()) {
	    config = new File(_javaHome + File.separator + "lib" +
					  File.separator + CONFIG_FILE);
	}

	if (config.exists()) {
            Properties configProps = loadPropertiesFile (new Properties(), config);
	    if (configProps != null) {
		String mandatory_str = configProps.getProperty(
					"deployment.system.config.mandatory");
		boolean mandatory = ((mandatory_str != null) &&
				     (!mandatory_str.equalsIgnoreCase("false")));
                String urlStr = configProps.getProperty(
					"deployment.system.config");
		_enterprizeConfig = urlStr;
		if (urlStr != null) {
		    if (!tryDownloading(urlStr, _systemProps)) {
		        retval = !mandatory;
			    }
			    }
			}
		    }
	refreshProps();

	versionUpdateCheck();

        setPolicyFiles();

	// (4978307) put all resulting (non jre) properties in system properties
	Properties systemProperties = System.getProperties();
	Enumeration en = _props.propertyNames();
	while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String value = getProperty(key);
	    if (value != null) {
	        systemProperties.put(key, value);
	    }
	}
	System.setProperties(systemProperties);

	return retval;
    }

    /*
     * This method will get the registry settings for APPLET tag
     * handling in browsers installed on the system.
     */
    public static void getBrowserSettings(){
        PlatformSpecificUtils.initBrowserSettings();
    }

    /*
     * This method will save the registry settings for APPLET
     * tag handling in browsers installed on the system.
     */
    public static void setBrowserSettings(){
        PlatformSpecificUtils.applyBrowserSettings();
    }

    private static boolean tryDownloading (String urlStr, Properties props) {
	try {
	    URL url = new URL(urlStr);
	    URLConnection connection = url.openConnection();
	    InputStream is = connection.getInputStream();
	    props.load(is);
	    return true;
	} catch (Exception e) {
	    Trace.ignoredException(e);
	    return false;
        }
    }

    private static void setPolicyFiles() {
        String systemPolicyURL = Config.getSystemSecurityPolicyURL();
        String userPolicyURL = Config.getUserSecurityPolicyURL();

	if (systemPolicyURL != null || userPolicyURL != null) {
            // Add system/user security policy in Deployment infrastructure.
            int numPolicy = 1;
            String policyURL = null;

	    while ((policyURL = Security.getProperty("policy.url." + numPolicy)) != null) {
                numPolicy++;
            }

            // Set system policy
            if (systemPolicyURL != null) {
                Security.setProperty("policy.url." + numPolicy, systemPolicyURL);
                numPolicy++;
            }

            // Set user policy
            if (userPolicyURL != null) {
                Security.setProperty("policy.url." + numPolicy, userPolicyURL);
                numPolicy++;
            }
	}
    }

    public static void refreshProps() {
        Properties userProps = new Properties();
	Properties jreProps = new Properties();
	Properties sysJreProps = new Properties();
        Properties jpiJreProps = new Properties();
        Properties jpiJdkProps = new Properties();

	_lockedProps = new Properties();
	_props = new Properties(_defaultProps);

	if (_userConfigFile.exists()) {
            loadPropertiesFile(userProps, _userConfigFile);
	    _lastChanged = _userConfigFile.lastModified();
	} else {
	    _lastChanged = System.currentTimeMillis();
	}
	Enumeration en = _systemProps.keys();
	while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String value = _systemProps.getProperty(key);
	    if (key.startsWith(JAVAWS_JRE_KEY)) {
		sysJreProps.setProperty(key, value);
	    } else if (key.endsWith(".locked")) {
	        int end = key.length() - ".locked".length();
		key = key.substring(0, end);
		String lockedValue = _systemProps.getProperty(key);	
		if (lockedValue != null) {
		    _lockedProps.setProperty(key, lockedValue);
		}
	    } else if (key.startsWith(BASE)) {
	        _props.setProperty(key, value);
	    }
	}

	en = userProps.keys();
	while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String value =  userProps.getProperty(key);
	    if (key.startsWith(JAVAWS_JRE_KEY)) {
                jreProps.setProperty(key, value);
            } else if(key.startsWith(JPI_JRE_KEY)){
                jpiJreProps.setProperty(key, value);
            } else if(key.startsWith(JPI_JDK_KEY)){
                jpiJdkProps.setProperty(key, value);
	    } else if (key.startsWith(BASE) || key.startsWith("javaplugin")) {
	        _props.setProperty(key, value);
	    }
	}

	en = _lockedProps.keys();
	while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    _props.setProperty(key, _lockedProps.getProperty(key));
	}

        JREInfo.initialize(sysJreProps, jreProps);
        PluginJavaInfo.initialize(jpiJreProps, jpiJdkProps);

	_dirty = false;
	_changedProps = new Properties();
    }

    /*
     * refreshUnchangedProps()
     * this reads new propertys from disk, then re-applies any pending changes
     */
    public static void refreshUnchangedProps() {
        Properties savedChangedProps = _changedProps;
        refreshProps();
        Enumeration enumeration = savedChangedProps.keys();
        while (enumeration.hasMoreElements()) {
            String key = (String) enumeration.nextElement();
            setProperty(key, savedChangedProps.getProperty(key));
        }
    }

    public static String getPropertiesFilename()  {
	return PROPERTIES_FILE;
    }

    public static boolean isDiskNewer() {
	return (_userConfigFile.exists() &&
		(_userConfigFile.lastModified() > _lastChanged));
    }

    // we need this method because Boolean.toString(boolean) is only
    // available in JRE 1.4+
    public static String booleanToString(boolean bool) {
	if (bool == true) {
	    return "true";
	} else {
	    return "false";
	}
    }

    public static void refreshIfNecessary() {
	if (!_dirty && isDiskNewer()) {
	    refreshProps();
	}
    }

    //
    // store() - write out the user's properties file.
    //
    public static void store() {
	if (_dirty && isDiskNewer()) {
	    Properties savedChangedProps = _changedProps;
	    refreshProps();
	    Enumeration enumeration = savedChangedProps.keys();
	    while (enumeration.hasMoreElements()) {
		String key = (String) enumeration.nextElement();
		setProperty(key, savedChangedProps.getProperty(key));
	    }
	}

	Properties p = new Properties();
	Properties jp = new Properties();
        Properties pjp = new Properties();
	Enumeration en = _props.keys();
        while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    if (!isLocked(key)) {
                String value = _props.getProperty(key);  
                String sysVal = _systemProps.getProperty(key); 
                String defVal = _defaultProps.getProperty(key);
                if ((value == null) ||
                    ((sysVal != null) && (!value.equals(sysVal))) ||
                    ((sysVal == null) && (!value.equals(defVal)))) {
                        p.setProperty(key, value);
                }
	    }
	}

	JREInfo[] jres = PluginJavaInfo.getAll();
	for (int i=0; i<jres.length; i++) {
            //if (jres[i] instanceof PluginJavaInfo ){
                // These are Plugin JREs...  Save them to the properties file.
                String version = jres[i].getProduct();
                String path = jres[i].getPath();
                String vmArgs = ((PluginJavaInfo)jres[i]).getVmArgs();

                // Save full entries only (must have version and path!)
		// must be valid path too
                if ( version == null || version.trim().equals("") ||
                     path == null || path.trim().equals("") ||
		     !isValidJPIJavaPath(path)){
                         continue;
                }
                if (((PluginJavaInfo)jres[i]).getProductName().equalsIgnoreCase("jre")){
                    pjp.setProperty(JPI_JRE_KEY + version + JPI_JAVA_PATH, path);
		    if (vmArgs != null) {
			pjp.setProperty(JPI_JRE_KEY + version + JPI_JAVA_ARGS, vmArgs);
		    }
		    if (jres[i].getOSArch() != null) {
			pjp.setProperty(JPI_JRE_KEY + version + JPI_JAVA_OSARCH, jres[i].getOSArch());                      
                    }
                    if (jres[i].getOSName() != null)  {	
			pjp.setProperty(JPI_JRE_KEY + version + JPI_JAVA_OSNAME, jres[i].getOSName());                      
                    }
                }else{
                    pjp.setProperty(JPI_JDK_KEY + version + JPI_JAVA_PATH, path);
		    if (vmArgs != null) {
			pjp.setProperty(JPI_JDK_KEY + version + JPI_JAVA_ARGS, vmArgs);
		    }
                }
            //}else{
        }// end for...
        jres = JREInfo.get();
        for(int i = 0; i < jres.length; i++){
                // These are Javaws JREs...
                if (!jres[i].isSystemJRE()) {
                    if (jres[i].getPlatform() != null) {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_PLATFORM_ID,
                                  jres[i].getPlatform());
                    }
                    if (jres[i].getProduct() != null)  {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_PRODUCT_ID,
                                  jres[i].getProduct());
                    }
                    if (jres[i].getLocation() != null)  {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_LOCATION,
                                  jres[i].getLocation());
                    }
                    if (jres[i].getPath() != null)  {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_PATH,
                                  jres[i].getPath());
                    }
                    if (jres[i].getOSArch() != null)  {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_OS_ARCH,
                                  jres[i].getOSArch());
                    }
                    if (jres[i].getOSName() != null)  {
                        jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_OS_NAME,
                                  jres[i].getOSName());
                    }
                    jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_ISENABLED,
                        	      booleanToString(jres[i].isEnabled()));
                    jp.setProperty(JAVAWS_JRE_KEY + i + JAVAWS_JRE_ISREGISTERED,
                        	      booleanToString(jres[i].isRegistered()));
                }
            //}//end else
	}// end for...
	_userConfigFile.getParentFile().mkdirs();
        try {
            FileOutputStream fos = new FileOutputStream(_userConfigFile);
            try {
                p.store(fos, getPropertiesFilename());
		jp.store(fos, "Java Web Start jre's");
                pjp.store(fos, "Java Plugin jre's");
            } catch (IOException ioe) {
                Trace.println("Exception: "+ ioe);
            }
            fos.flush();
            fos.close();
        } catch (IOException ioe) {}
	_dirty = false;
	_changedProps = new Properties();

	_lastChanged = _userConfigFile.exists() ?
            _userConfigFile.lastModified() : System.currentTimeMillis();
    }

    // From PluginTableModel.java
    public static boolean isValidJPIJavaPath(String path) {
	String java = "java";
        if ( System.getProperty("os.name").indexOf("Windows") != -1 ){
            java = java + ".exe";
        }

        if (path != null) {
            if (path.endsWith(File.separator)){
                path = path + "bin" + File.separator + java;
            }else{
                path = path + File.separator + "bin" + File.separator + java;
            }

            // Check if path exists.
            File f = new File(path);
            return (f.exists());
        }
        return false;
    }

    //
    // storeIfDirty() - write out if anything has changed.
    //
    public static void storeIfDirty() {
	if (_dirty) {
	    store();
	}
    }

    public static Properties getProperties() {
	Properties p = new Properties(_props);
	String key;
	String value;
	// We should convert $USER_HOME, $SYSTEM_HOME, etc before
	// returning the Properties object
	for (Enumeration e = p.propertyNames(); e.hasMoreElements();) {
	    key = (String)e.nextElement();
	    value = p.getProperty(key);
	    if (value != null) {
		p.setProperty(key, replaceVariables(value).trim());
	    }
	}
	return p;
    }

    //
    // isLocked(String key) - is this key a "locked" property
    //
    public static boolean isLocked(String key) {
	return _lockedProps.containsKey(key);
    }


    //
    // infrastucture locations
    //

    public static String getJavaHome() {
	if (_javaHome == null) {
            String ret = System.getProperty("jnlpx.home");
	    if (ret != null) {
		// use jnlpx.home if available
		// jnlpx.home always point to the location of the
		// jre bin directory (where javaws is installed)
		ret = ret.substring(0, ret.lastIndexOf(File.separator));
	    } else {
		ret = System.getProperty("java.home");
	    }
            _javaHome = ret;
	}
	return _javaHome;
    }


    public static String getOSHome() {
	return _osHome;
    }

    public static String getSystemHome() {
	return _systemHome;
    }

    public static String getUserHome() {
	return _userHome;
    }

    //
    // machine OSName, OSArch, OSPlatform, and Locale
    //

    public static String getOSName() {
	return _os;
    }

    public static String getOSArch() {
	return _arch;
    }

    public static String getOSPlatform() {
	return _platform;
    }

    //
    // Public routines to get and set properties of each type
    //

    //
    // getProperty(String key) - All get's go through here
    //
    public static String getProperty(String key) {
        String value = _props.getProperty(key);
	if (value != null) return replaceVariables(value).trim();
	return null;
    }

    //
    // setProperty(String key, String value) - All set's go through here
    //
    public static void setProperty(String key, String value) {
        setProperty(key, value, true);
    }

    public static void setProperty(String key, String value, boolean restore) {
        if (isDiskNewer()) {
            refreshIfNecessary();
        }
        if ((value == null) || (value.length() ==0)) {
            _dirty |= _props.containsKey(key);
            _props.remove(key);
	    _changedProps.remove(key);
        } else {
            if (restore) {
                value = restoreVariables(value);
            }
            if (!value.equals(_props.getProperty(key))) {
                _dirty = true;
                _props.setProperty(key, value);
		_changedProps.setProperty(key, value);
		if (_ControlPanel != null) {
		    _ControlPanel.propertyHasChanged();
		}
            }
        }
    }

    //
    // getIntProperty(String key) returns the integer property, or -1
    // if no value or default exists for key, or it is not a valid int
    //
    public static int getIntProperty(String key) {
        String value = getProperty(key);
        if (value == null) {
            return -1;
        }
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException nfe) {
            return -1;
        }
    }

    //
    // setIntProperty(String key, int value)
    //
    public static void setIntProperty(String key, int value) {
	setProperty(key, Integer.toString(value));
    }

    //
    // Returns the boolean value for the property key
    // This assumes the value is a String,
    // if no value exists for key, or it is not a valid
    // boolean, false is returned.
    //
    public static boolean getBooleanProperty(String key) {
        String value = getProperty(key);
        if (value == null) {
            return false;
        }
        return Boolean.valueOf(value).booleanValue();
    }

    //
    // setBooleanProperty(String key, boolean value)
    //
    public static void setBooleanProperty(String key, boolean value) {
	setProperty(key, booleanToString(value));
    }

    private static String restoreVariables(String value) {
	if (value.indexOf(_javaHome) >= 0) {
	    value = substitute(value, "$JAVA_HOME", _javaHome);
	}
	if (value.indexOf(_userHome) >= 0) {
	    value = substitute(value, "$USER_HOME", _userHome);
	}
	if (value.indexOf(_systemHome) >= 0) {
 	    value = substitute(value, "$SYSTEM_HOME", _systemHome);
	}
	return value;
    }

    private static String replaceVariables(String value) {
	if (value.indexOf("$JAVA_HOME") >= 0) {
	    value = substitute(value, _javaHome, "$JAVA_HOME");
	}
	if (value.indexOf("$USER_HOME") >= 0) {
	    value = substitute(value, _userHome, "$USER_HOME");
	}
	if (value.indexOf("SYSTEM_HOME") >= 0) {
	    value = substitute(value, _systemHome, "$SYSTEM_HOME");
	}
	return value;
    }

    private static String substitute(String val, String s1, String s2) {
	int i = val.indexOf(s2);
        return (i < 0) ? val :
		(val.substring(0, i) + s1 + val.substring(i+s2.length()));
    }

    private static void versionUpdateCheck() {
	String ver = getProperty(VERSION_UPDATED_KEY);
	if (ver == null || ver.compareTo(VERSION_UPDATE_THIS) < 0) {
	    // 1) current config file may have been used in either or
	    //    both webstart or plugin in 1.4.2)
	    Properties p1 = translateMantisProperties(_props);
	    if (p1.getProperty(JAVAWS_CACHE_KEY) == null) {
		String path = _userHome + File.separator + "javaws" +
                                          File.separator + "cache";
		if ((new File(path)).exists()) {
		    p1.setProperty(JAVAWS_CACHE_KEY, path);
		}
	    }

            // 2 try to look for 1.4.2 javaws config file and translate
	    Properties p2 = null;
	    {
	        File home = new File(_userHome);
		String dir = home.getParent() + File.separator +
		    ".deployment" ;
		String path = dir + File.separator + "deployment.properties";
		File propFile = new File(path);
		if (propFile.exists()) {
		    p2 = translateMantisProperties(
                        loadPropertiesFile(new Properties(), propFile));
		    if (p2.getProperty(JAVAWS_CACHE_KEY) == null) {
			String oldDefault = dir + File.separator + "javaws" +
						  File.separator + "cache";
			if ((new File(oldDefault)).exists()) {
			    p2.setProperty(JAVAWS_CACHE_KEY, oldDefault);
			}
		    }
		}
	    }

	    // 3 look for older style javaws config file
	    if (p2 == null) {
		String dir = System.getProperty("user.home") +
		    File.separator + ".javaws";
		String path = dir + File.separator + "javaws.cfg";
		File propFile = new File(path);
		if (propFile.exists()) {
		    p2 = translateOlderProperties(
                        loadPropertiesFile(new Properties(), propFile), dir);
		}
	    }
	    if (p2 != null) {
		setProperties(p2);
	    }
	    if (p1 != null) {
		setProperties(p1);
	    }

            // make sure we have a valid browser path on Unix OS'es
            setProperty(Config.BROWSER_PATH_KEY,
                        Config.getInstance().getBrowserPath());

	    setProperty(VERSION_UPDATED_KEY, VERSION_UPDATE_THIS);

	    // force this to hapen the first time.
	    setBooleanProperty(CAPTURE_MIME_KEY, true);

	    storeIfDirty();
	}
    }

    private static void setProperties(Properties p) {
	Enumeration en = p.keys();
	while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    setProperty(key, p.getProperty(key));
	}
    }

    private static Properties loadPropertiesFile(
                                Properties props, File file) {
        try {
            InputStream is = new FileInputStream(file);
            props.load(is);
            is.close();
            return props;
        } catch (FileNotFoundException fnfe) {
            Trace.println("Cannot find prop file: "+file.getAbsolutePath());
            return null;
        } catch (IOException ioe) {
            Trace.println("IO Execption: "+ ioe);
            return null;
        }
    }

    private static Properties translateMantisProperties(Properties p) {
	Properties out = new Properties();
        Enumeration en = p.keys();
        while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String newKey = null;
	    String value = p.getProperty(key);

	    // here for any translation ...
	    if (key.startsWith("deployment.javaws.proxy")) {
		if (key.equals("deployment.javaws.proxy.httpport")) {
		    newKey = PROX_HTTP_PORT_KEY;
		} else if (key.equals("deployment.javaws.proxy.http")) {
                    newKey = PROX_HTTP_HOST_KEY;
   		} else if (key.equals(
			"deployment.javaws.proxy.httpproxyoverride")) {
                    newKey = PROX_OVERRIDE_KEY;
		}
	    } else if (key.startsWith("deployment.javaws.")) {
		if (key.equals("deployment.javaws.logFileName")) {
		    newKey = JAVAWS_TRACEFILE_KEY;
		} else if (key.equals("deployment.javaws.showConsole")) {
		    if (value.equals("true")) {
			newKey = CONSOLE_MODE_KEY;
			value = CONSOLE_MODE_SHOW;
		    }
		} else if (key.equals("deployment.javaws.updateTimeout")) {
		    newKey = JAVAWS_UPDATE_TIMEOUT_KEY;
		}
	    } else if (key.startsWith("javaplugin")) {
		// leave all plugin values to run with 1.4.2
	        out.setProperty(key, value);
		// and foward port some:
		if (key.equals("javaplugin.cache.disabled")) {
		    newKey = CACHE_ENABLED_KEY;
		    value = (value.equals("true")) ? "false" : "true";
		} else if (key.equals("javaplugin.cache.size")) {
		    newKey = CACHE_MAX_KEY;
		} else if (key.equals("javaplugin.cache.compression")) {
		    newKey = CACHE_COMPRESSION_KEY;
		} else if (key.equals("javaplugin.console")) {
		    newKey = CONSOLE_MODE_KEY;
		    if (value.equals("show")) {
			value = CONSOLE_MODE_SHOW;
		    } else if (value.equals("hide")) {
			value = CONSOLE_MODE_HIDE;
		    } else {
			value = CONSOLE_MODE_DISABLED;
		    }
		} else if (key.equals("javaplugin.exception")) {
		    newKey = SHOW_EXCEPTIONS_KEY;
		}
	    }
	    if (newKey != null) {
	        if (value != null && value.length() > 0) {
	            out.setProperty(newKey, value);
	        }
	    }
	}
	String cachePath = p.getProperty("deployment.javaws.cache.dir");
	if (cachePath != null) {
	    out.setProperty(JAVAWS_CACHE_KEY, cachePath);
	}
	return out;
    }


    private static Properties translateOlderProperties(Properties p, String d) {
	Properties out = new Properties();
        Enumeration en = p.keys();

        while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String value = p.getProperty(key);
	    if (key.startsWith("javaws.cfg.jre.")) {
		continue;
	    } else if (key.startsWith("javaws.cfg.splash")) {
		continue;
	    } else if (key.startsWith("javaws.cfg.player")) {
		continue;
            } else if (key.equals("javaws.cfg.cache.dir")) {
		out.setProperty(JAVAWS_CACHE_KEY, value);
	    }
	    if (key.startsWith("javaws.cfg.")) {
		if (key.equals("javaws.cfg.proxy.httpport")) {
		    key = PROX_HTTP_PORT_KEY;
		} else if (key.equals("javaws.cfg.proxy.http")) {
		    key = PROX_HTTP_HOST_KEY;
		} else if (key.equals("javaws.cfg.proxy.httpproxyoverride")) {
		    key = PROX_OVERRIDE_KEY;
		} else {
		    key = key.replaceFirst("javaws.cfg.", BASE);
		}
	        if (value != null && value.length() > 0) {
	            out.setProperty(key, value);
	        }
	    }
	}
	if (out.getProperty(JAVAWS_CACHE_KEY) == null) {
	    String path = d + File.separator + "cache";
	    if ((new File(path)).exists()) {
		out.setProperty(JAVAWS_CACHE_KEY, path);
	    }
	}
	return out;
    }

    private static Properties getDefaultProps() {
	Properties p = new Properties();

	p.setProperty(JPI_TRACE_FILE_KEY, JPI_TRACE_FILE_DEF);
	p.setProperty(JPI_LOG_FILE_KEY, JPI_LOG_FILE_DEF);
	p.setProperty(ASSOCIATION_MODE_KEY, ASSOCIATION_MODE_DEF);
	p.setProperty(JAVAWS_TRACEFILE_KEY, JAVAWS_TRACEFILE_DEF);
	p.setProperty(JAVAWS_LOGFILE_KEY, JAVAWS_LOGFILE_DEF);
	p.setProperty(SEC_TLS_KEY, ""+SEC_TLS_DEF);
	p.setProperty(CACHEDIR_KEY, CACHEDIR_DEF);
	p.setProperty(LOGDIR_KEY, LOGDIR_DEF);
	p.setProperty(TMPDIR_KEY, TMPDIR_DEF);
	p.setProperty(USR_EXTDIR_KEY, USR_EXTDIR_DEF);
	p.setProperty(USEC_POLICY_KEY, USEC_POLICY_DEF);
	p.setProperty(USEC_CACERTS_KEY, USEC_CACERTS_DEF);
	p.setProperty(USEC_JSSECERTS_KEY, USEC_JSSECERTS_DEF);
	p.setProperty(USEC_TRUSTED_CERTS_KEY, USEC_TRUSTED_CERTS_DEF);
	p.setProperty(USEC_TRUSTED_JSSE_CERTS_KEY,
			USEC_TRUSTED_JSSE_CERTS_DEF);
	p.setProperty(USEC_TRUSTED_CLIENT_CERTS_KEY,
			USEC_TRUSTED_CLIENT_CERTS_DEF);
	p.setProperty(SSEC_CACERTS_KEY, SSEC_CACERTS_DEF);
	p.setProperty(SSEC_JSSECERTS_KEY, SSEC_JSSECERTS_DEF);
	p.setProperty(SSEC_TRUSTED_CERTS_KEY, SSEC_TRUSTED_CERTS_DEF);
	p.setProperty(SSEC_TRUSTED_JSSE_CERTS_KEY,
			SSEC_TRUSTED_JSSE_CERTS_DEF);
	p.setProperty(SSEC_TRUSTED_CLIENT_CERTS_KEY,
			SSEC_TRUSTED_CLIENT_CERTS_DEF);
	p.setProperty(SEC_ASKGRANT_SHOW_KEY, ""+SEC_ASKGRANT_SHOW_DEF);
	p.setProperty(SEC_ASKGRANT_NOTCA_KEY, ""+SEC_ASKGRANT_NOTCA_DEF);
	p.setProperty(SEC_USE_BROWSER_KEYSTORE_KEY, ""+SEC_USE_BROWSER_KEYSTORE_DEF);
	p.setProperty(SEC_NOTINCA_WARN_KEY, ""+SEC_NOTINCA_WARN_DEF);
	p.setProperty(SEC_EXPIRED_WARN_KEY, ""+SEC_EXPIRED_WARN_DEF);
	p.setProperty(SEC_JSSE_HOST_WARN_KEY, ""+SEC_JSSE_HOST_WARN_DEF);
	p.setProperty(SEC_TRUSTED_POLICY_KEY, ""+SEC_TRUSTED_POLICY_DEF);
	p.setProperty(SEC_AWT_WARN_WINDOW_KEY, ""+SEC_AWT_WARN_WINDOW_DEF);
	p.setProperty(SEC_SANDBOX_JNLP_ENHANCED_KEY,
			""+SEC_SANDBOX_JNLP_ENHANCED_DEF);
	p.setProperty(SEC_AUTHENTICATOR_KEY, ""+SEC_AUTHENTICATOR_DEF);
	p.setProperty(PROX_TYPE_KEY, ""+PROX_TYPE_DEF);
        p.setProperty(PROX_SAME_KEY, ""+PROX_SAME_DEF);
        p.setProperty(PROX_LOCAL_KEY, ""+PROX_LOCAL_DEF);
	p.setProperty(PROX_OVERRIDE_KEY, PROX_OVERRIDE_DEF);
	p.setProperty(CACHE_MAX_KEY, ""+CACHE_MAX_DEF);
	p.setProperty(CACHE_COMPRESSION_KEY, ""+CACHE_COMPRESSION_DEF);
	p.setProperty(CACHE_ENABLED_KEY, ""+CACHE_ENABLED_DEF);
	p.setProperty(OPR_ENABLED_KEY, ""+OPR_ENABLED_DEF);
	p.setProperty(OPR_ASK_SHOW_KEY, ""+OPR_ASK_SHOW_DEF);
	p.setProperty(CONSOLE_MODE_KEY, ""+CONSOLE_MODE_DEF);
        p.setProperty(TRACE_MODE_KEY, ""+TRACE_MODE_DEF);
        p.setProperty(LOG_MODE_KEY, ""+LOG_MODE_DEF);
        p.setProperty(LOG_CP_KEY, ""+LOG_CP_DEF);
        p.setProperty(SHOW_EXCEPTIONS_KEY, ""+SHOW_EXCEPTIONS_DEF);
	p.setProperty(JPI_RUNTIME_TYPE_KEY, ""+JPI_RUNTIME_TYPE_DEF);
	p.setProperty(SHORTCUT_MODE_KEY, ""+SHORTCUT_MODE_DEF);
	p.setProperty(SHORTCUT_UNINSTALL_KEY, ""+SHORTCUT_UNINSTALL_DEF);
        p.setProperty(BROWSER_PATH_KEY, ""+BROWSER_PATH_DEF);
        p.setProperty(EXTENDED_BROWSER_ARGS_KEY, ""+EXTENDED_BROWSER_ARGS_DEF);
        p.setProperty(CAPTURE_MIME_KEY, ""+CAPTURE_MIME_DEF);
        p.setProperty(UPDATE_MIME_KEY, ""+UPDATE_MIME_DEF);
        p.setProperty(MIME_DEFAULTS_KEY, ""+MIME_DEFAULTS_MIME_DEF);
        p.setProperty(JAVAWS_JRE_INSTALL_KEY, JAVAWS_JRE_INSTALL_DEF);
        p.setProperty(JAVAWS_JRE_AUTODOWNLOAD_KEY, JAVAWS_JRE_AUTODOWNLOAD_DEF);
	p.setProperty(JAVAWS_MUFFIN_LIMIT_KEY, ""+JAVAWS_MUFFIN_LIMIT_DEF);
	p.setProperty(JAVAWS_UPDATE_TIMEOUT_KEY, ""+JAVAWS_UPDATE_TIMEOUT_DEF);

	p.setProperty(REMOTE_JNLP_DOCS_KEY, REMOTE_JNLP_DOCS_DEF);
	p.setProperty(REMOTE_J2SE_DOCS_KEY, REMOTE_J2SE_DOCS_DEF);

	// system property can override default jauthenticator property
	String auth = System.getProperty(JAUTHENTICATOR_SYSTEM_PROP);
	if (auth != null) {
	    String value = "" + (!auth.equalsIgnoreCase("all") &&
			!auth.equalsIgnoreCase("true"));
	    p.setProperty (SEC_AUTHENTICATOR_KEY, value);
	}

	return p;
    }

    public static void printProps() {
        Trace.println("\n_defaultProps:");
        printP(_defaultProps);

        Trace.println("\n_systemProps:");
	printP(_systemProps);

	Trace.println("\n_props:");
	printP(_props);
    }


    private static void printP(Properties p) {
        Enumeration en = p.keys();
        while (en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    String value = p.getProperty(key);
	    Trace.println("  "+key+" : "+value);
	}
	Trace.println("");
    }


    public static String getUserTrustedCertificateFile() {
	return getProperty(USEC_TRUSTED_CERTS_KEY);
    }

    public static String getSystemTrustedCertificateFile() {
	return getProperty(SSEC_TRUSTED_CERTS_KEY);
    }

    public static String getUserTrustedHttpsCertificateFile() {
	return getProperty(USEC_TRUSTED_JSSE_CERTS_KEY);
    }

    public static String getSystemTrustedHttpsCertificateFile() {
	return getProperty(SSEC_TRUSTED_JSSE_CERTS_KEY);
    }

    public static String getUserRootCertificateFile() {
	return getProperty(USEC_CACERTS_KEY);
    }

    public static String getSystemRootCertificateFile() {
	return getProperty(SSEC_CACERTS_KEY);
    }

    public static String getUserSSLRootCertificateFile() {
	return getProperty(USEC_JSSECERTS_KEY);
    }

    public static String getSystemSSLRootCertificateFile() {
	return getProperty(SSEC_JSSECERTS_KEY);
    }

    public static String getUserClientAuthCertFile() {
	return getProperty(USEC_TRUSTED_CLIENT_CERTS_KEY);
    }

    public static String getSystemClientAuthCertFile() {
	return getProperty(SSEC_TRUSTED_CLIENT_CERTS_KEY);
    }

    public static String getUserCookieFile() {
	return _userHome + File.separator + "security" + File.separator + "cookie.txt";
    }

    public static String getCacheDirectory() {
	return getProperty(CACHEDIR_KEY);
    }

    public static String getPluginCacheDir() {
	String overrideLocation = getProperty(JAVAPI_CACHE_KEY);
	if (overrideLocation != null) { return overrideLocation; }
	return getCacheDirectory() + File.separator + "javapi";
    }

    public static String getTempCacheDir() {
	return getCacheDirectory() + File.separator + "tmp";
    }

    public static String getSystemCacheDirectory() {
	return getProperty(SYSCACHE_KEY);
    }

    public static void setCacheDirectory(String dir) {
	setProperty(CACHEDIR_KEY, dir, true);
    }

    public static String getLogDirectory() {
	return getProperty(LOGDIR_KEY);
    }

    public static String getTempDirectory() {
	return getProperty(TMPDIR_KEY);
    }

    public static String getUserExtensionDirectory() {
	return getProperty(USR_EXTDIR_KEY);
    }

    public static String getSystemExtensionDirectory() {
	return getProperty(SYS_EXTDIR_KEY);
    }

    public static String getUserSecurityPolicyURL() {
	return getProperty(USEC_POLICY_KEY);
    }

    public static String getSystemSecurityPolicyURL() {
	return getProperty(SSEC_POLICY_KEY);
    }

    public static boolean useSystemLookAndFeel() {
	String value = getProperty(USE_SYSTEM_LF_KEY);
	if (value != null) {
	    return Boolean.valueOf(value).booleanValue();
	}

	// so to default to true on gnome and windows:
	return getInstance().systemLookAndFeelDefault();
    }

    //
    // Returns the command to launch Java.
    // On Unix: {java_home}/bin/java
    // On Windows: {java_home}/bin/javaw.exe
    //
    public static String getJavaCommand(){
        String command = System.getProperty("java.home");

        if (!command.endsWith(File.separator)) {
            command = command + File.separator;
        }

        command = command + "bin" + File.separator +
                    Config.getInstance().getPlatformSpecificJavaName();

        return command;
    }

    //
    // Returns the command used to start up a new version of Java Web Start.
    //
    public static String getJavawsCommand() {
        String path = getJavaHome();

        if (!path.endsWith(File.separator)) {
            path = path + File.separator;
        }
        path = path + "bin" + File.separator + "javaws" +
	    Config.getInstance().getPlatformExtension();
        return path;
    }

    public static String getJavawsCacheDir() {
	String overrideLocation = getProperty(JAVAWS_CACHE_KEY);
	if (overrideLocation != null) { return overrideLocation; }
	return getCacheDirectory() + File.separator + "javaws";
    }

    public static String getSplashDir() {
	return getJavawsCacheDir() + File.separator + "splash";
    }

    public static String getSplashIndex() {
	return getSplashDir() + File.separator + "splash.xml";
    }

    public static void setSplashCache() {
	// last arg false so it writes it in config file even if its default
	setProperty(SPLASH_CACHE_INDEX_KEY, getSplashIndex(), false);
    }

    public static String[] getSecureProperties() {

        ArrayList list = new ArrayList(4);
        for (int i=0; i<DefaultSecureProperties.length;
                list.add(DefaultSecureProperties[i++]));
        String ConfigKeys = getProperty (SECURE_PROPS_KEY);
        if (ConfigKeys != null) {
            StringTokenizer st = new StringTokenizer(ConfigKeys, ",");
            while (st.hasMoreTokens()) {
                list.add(st.nextToken());
            }
        }
        return (String []) list.toArray(new String[0]);
    }

    public static boolean isSecureVmArg(String arg) {
	for (int i=0; i<secureVmArgs.length; i++) {
	    if (arg.equals(secureVmArgs[i])) { return true; }
	}
	for (int i=0; i<secureVmPrefixes.length; i++) {
	    if (arg.startsWith(secureVmPrefixes[i])) { return true; }
	}
	return false;
    }

    public static List getProxyOverrideList() {
	return null;
    }

    public static void setShortcutValue(int value) {
	String str;
	switch (value) {
	    case SHORTCUT_NEVER:
		str = SHORTCUT_MODE_NEVER;
	    	break;
	    case SHORTCUT_ALWAYS:
		str = SHORTCUT_MODE_ALWAYS;
		break;
	    default:
	    case SHORTCUT_ASK_USER:
		str = SHORTCUT_MODE_ASK_USER;
		break;
	    case SHORTCUT_ASK_IF_HINTED:
		str = SHORTCUT_MODE_ASK_IF_HINTED;
		break;
	    case SHORTCUT_ALWAYS_IF_HINTED:
		str = SHORTCUT_MODE_ALWAYS_IF_HINTED;
		break;
	}
	setProperty(SHORTCUT_MODE_KEY, str);
    }

    public static int getShortcutValue() {
	String str = getProperty(SHORTCUT_MODE_KEY);
	if (str.equals(SHORTCUT_MODE_NEVER)) {
	    return SHORTCUT_NEVER;
	}
	if (str.equals(SHORTCUT_MODE_ALWAYS)) {
	    return SHORTCUT_ALWAYS;
	}
	if (str.equals(SHORTCUT_MODE_ASK_USER)) {
	    return SHORTCUT_ASK_USER;
	}
	if (str.equals(SHORTCUT_MODE_ASK_IF_HINTED)) {
	    return SHORTCUT_ASK_IF_HINTED;
	}
	if (str.equals(SHORTCUT_MODE_ALWAYS_IF_HINTED)) {
	    return SHORTCUT_ALWAYS_IF_HINTED;
	}
        return SHORTCUT_NEVER;
    }
    public static void setAssociationValue(int value) {
	String str;
	switch (value) {
	    case ASSOCIATION_NEVER:
		str = ASSOCIATION_MODE_NEVER;
	    	break;
	    case ASSOCIATION_NEW_ONLY:
		str = ASSOCIATION_MODE_NEW_ONLY;
		break;
	    default:
	    case ASSOCIATION_ASK_USER:
		str = ASSOCIATION_MODE_ASK_USER;
		break;
	    case ASSOCIATION_REPLACE_ASK:
		str = ASSOCIATION_MODE_REPLACE_ASK;
		break;
	}
	setProperty(ASSOCIATION_MODE_KEY, str);
    }

    public static int getAssociationValue() {
	String str = getProperty(ASSOCIATION_MODE_KEY);
	if (str.equals(ASSOCIATION_MODE_NEVER)) {
	    return ASSOCIATION_NEVER;
	}
	if (str.equals(ASSOCIATION_MODE_NEW_ONLY)) {
	    return ASSOCIATION_NEW_ONLY;
	}
	if (str.equals(ASSOCIATION_MODE_ASK_USER)) {
	    return ASSOCIATION_ASK_USER;
	}
	if (str.equals(ASSOCIATION_MODE_REPLACE_ASK)) {
	    return ASSOCIATION_REPLACE_ASK;
	}
        return ASSOCIATION_NEVER;
    }

    public static long getCacheSizeMax() {
	long size = -1;
	long factor;
	String str = getProperty(CACHE_MAX_KEY);
	if (str != null && str.length() > 0) {

           if (str.endsWith("M") || str.endsWith("m")) {
                // Size is in MegaBytes
                factor = 1024*1024;
                str = str.substring(0, str.length() - 1);
            } else if (str.endsWith("K") || str.endsWith("k")) {
                // Size is in KiloBytes
                factor = 1024;
                str = str.substring(0, str.length() - 1);
            } else {
                // Size is in bytes
                factor = 1;
            }
            try {
                // Parse the size
                size = factor * Long.valueOf(str).longValue();
            } catch (NumberFormatException e) {
		// 100 meg if error
		size = 100 * 1024 * 1024;
            }
	}
	return size;
    }


    //
    // non static methods :
    // these default implementations can be overwritten with
    // platform specific implementations
    //

    // platform specific implementation
    public String getPlatformOSHome() {
        return null;
    }

    // platform specific implementation
    public String getPlatformSystemHome() {
        return null;
    }

    // platform specific implementation
    public String getPlatformUserHome() {
	return null;
    }

    // platform specific implementation
    public String getSystemJavawsPath() {
	return getJavawsCommand();
    }

    // platform specific implementation
    public String toExecArg(String path) {
	// default implementation returns the path
	return path;
    }

    // platform specific implementation
    public String getLibraryPrefix() {
	return null;
    }

    // platform specific implementation
    public String getPlatformSpecificJavaName() {
        return null;
    }

    // platform specific implementation
    public String getPlatformExtension() {
        return null;
    }

    // platform specific implementation
    public String getDebugJavaPath(String path) {
        return path;
    }

    

    // platform specific implementation
    public String getLibrarySufix() {
	return null;
    }

    // platform specific implementation
    public boolean useAltFileSystemView() {
	return false;
    }

    // platform specific implementation
    public boolean showDocument(String url) {
	return false;
    }

    // platform specific implementation
    public String getBrowserPath() {
        return null;
    }

    // platform specific implementation
    public String getMozillaUserProfileDirectory() {
        return null;
    }

    // platform specific implementation
    public int installShortcut(String path, String appName,
	String description, String appPath, String args,
	String directory, String iconPath) {
	return 0;
    }

    // platform specific implementation
    public boolean isLocalInstallSupported() {
	// overridded on unix
	return true;
    }

    // platform specific implementation
    public int addRemoveProgramsAdd(String jnlpFile, String appName, boolean sysCache) {
	return 0;
    }

    // platform specific implementation
    public int addRemoveProgramsRemove(String appName) {
	return 0;
    }

    // platform specific implementation
    public boolean systemLookAndFeelDefault() {
	return true;
    }

    // platform specific implementation
    public String getSessionSpecificString() {
	return "";
    }

    /**
     * Show Error dialog for setting default VM for MSIE
     */
    private static void showIExplorerErrorDialog()
    {
	DialogFactory.showErrorDialog(
		ResourceManager.getMessage("browser.settings.fail.ie.text"),
		ResourceManager.getMessage("browser.settings.fail.caption"));
    }

    /**
     * Show Error dialog for setting default VM for Mozilla
     */
    private static void showMozillaErrorDialog()
    {
        DialogFactory.showErrorDialog(
		ResourceManager.getMessage("browser.settings.fail.moz.text"),
		ResourceManager.getMessage("browser.settings.fail.caption"));
    }

    /**
     * Show Success dialog for setting default VM
     */
    private static void showSuccessDialog()
    {
	DialogFactory.showInformationDialog(
		ResourceManager.getMessage("browser.settings.success.text"),
		ResourceManager.getMessage("browser.settings.success.caption"));
    }

    /**
     * Show Alert dialog for setting default VM
     */
    private static boolean showAlertDialog()
    {
	int userChoice;
	userChoice = DialogFactory.showConfirmDialog(
		ResourceManager.getMessage("browser.settings.alert.text"));
	return (userChoice == javax.swing.JOptionPane.YES_OPTION) ? true:false;
    }

    public static String getEnterprizeString() {
	return _enterprizeConfig;
    }
    public static boolean isDebugMode() {
	String debugMode = System.getProperty("deploy.debugMode");
	if (debugMode != null && debugMode.equalsIgnoreCase("true")) {
	    return true;
	} 
	return false;
    }
    public static boolean isDebugVMMode() {
	String debugVMMode = System.getProperty("deploy.useDebugJavaVM");
	if (debugVMMode != null && debugVMMode.equalsIgnoreCase("true")) {
	    return true;
	} 
	return false;
    }
    
}



