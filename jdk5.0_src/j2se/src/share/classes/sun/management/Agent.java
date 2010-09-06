/*
 * @(#)Agent.java	1.12 04/06/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.text.MessageFormat;
import java.util.Properties;
import java.util.Enumeration;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.lang.management.ManagementFactory;
import java.lang.reflect.Method;

import javax.management.remote.JMXConnectorServer;

import sun.management.snmp.AdaptorBootstrap;
import sun.management.jmxremote.ConnectorBootstrap;
import static sun.management.AgentConfigurationError.*;

/**
 * This Agent is started by the VM when -Dcom.sun.management.snmp
 * or -Dcom.sun.management.jmxremote is set. This class will be
 * loaded by the system class loader.
 */
public class Agent {
    // management properties
    private static Properties mgmtProps;
    private static ResourceBundle messageRB;

    private static final String DEFAULT_AGENT_CLASS =
        "sun.management.Agent";
    private static final String CONFIG_FILE =
        "com.sun.management.config.file";
    private static final String SNMP_PORT =
        "com.sun.management.snmp.port";
    private static final String JMXREMOTE =
	"com.sun.management.jmxremote";
    private static final String JMXREMOTE_PORT =
        "com.sun.management.jmxremote.port";
    private static final String ENABLE_THREAD_CONTENTION_MONITORING =
        "com.sun.management.enableThreadContentionMonitoring";


    public static void premain(String args) throws Exception {

        // initialize management properties 
        Properties props = getManagementProperties();
        if (props == null) {
            return;
        }

        String snmpPort = props.getProperty(SNMP_PORT);
	String jmxremote = props.getProperty(JMXREMOTE);
        String jmxremotePort = props.getProperty(JMXREMOTE_PORT);

        // Enable optional monitoring functionality if requested
        final String enableThreadContentionMonitoring =
            props.getProperty(ENABLE_THREAD_CONTENTION_MONITORING);
        if (enableThreadContentionMonitoring != null) {
            ManagementFactory.getThreadMXBean().
                setThreadContentionMonitoringEnabled(true);
        }

        try {
            if (snmpPort != null) {
                AdaptorBootstrap.initialize(snmpPort, props);
            }

	    /*
	     * If the jmxremote.port property is set then we start the
	     * RMIConnectorServer for remote M&M.	
	     * 
	     * If the jmxremote or jmxremote.port properties are set then
	     * we start a RMIConnectorServer for local M&M. The address
	     * of this "local" server is exported as a counter to the jstat
	     * instrumentation buffer.
	     */
            if (jmxremote != null || jmxremotePort != null) {
		if (jmxremotePort != null) {
                    ConnectorBootstrap.initialize(jmxremotePort, props);
		}
	  	JMXConnectorServer cs = ConnectorBootstrap.startLocalConnectorServer();
		ConnectorAddressLink.export(cs.getAddress().toString());
            }
        } catch (AgentConfigurationError e) {
            error(e.getError(), e.getParams());
        } catch (Exception e) {
            error(e);
        }
    }
 
    public static Properties loadManagementProperties() {
        Properties props = new Properties();
    
        // Load the management properties from the config file
        readConfiguration(props);

        // management properties can be overridden by system properties 
        // which take precedence
        props.putAll(System.getProperties());

        return props;
    }

    public static synchronized Properties getManagementProperties() {
        if (mgmtProps == null) {
            String configFile = System.getProperty(CONFIG_FILE);
            String snmpPort = System.getProperty(SNMP_PORT);
	    String jmxremote = System.getProperty(JMXREMOTE);
            String jmxremotePort = System.getProperty(JMXREMOTE_PORT);

            if (configFile == null && snmpPort == null && 
                jmxremote == null && jmxremotePort == null) {
                // return if out-of-the-management option is not specified
                return null;
            }
            mgmtProps = loadManagementProperties();
        }
        return mgmtProps;
    }

    // read config file and initialize the properties
    private static void readConfiguration(Properties p) {
        String fname = System.getProperty("com.sun.management.config.file");
        if (fname == null) {
            String home = System.getProperty("java.home");
            if (home == null) {
                throw new Error("Can't find java.home ??");
            }
            StringBuffer defaultFileName = new StringBuffer(home);
            defaultFileName.append(File.separator).append("lib");
            defaultFileName.append(File.separator).append("management");
            defaultFileName.append(File.separator).append("management.properties");
            // Set file name
            fname = defaultFileName.toString();
        }
        final File configFile = new File(fname);
        if (!configFile.exists()) {
            error(CONFIG_FILE_NOT_FOUND, fname);
        }

        InputStream in = null;
        try {
            in = new FileInputStream(configFile);
            BufferedInputStream bin = new BufferedInputStream(in);
            p.load(bin);
        } catch (FileNotFoundException e) {
            error(CONFIG_FILE_OPEN_FAILED, e.getMessage());
        } catch (IOException e) {
            error(CONFIG_FILE_OPEN_FAILED, e.getMessage());
        } catch (SecurityException e) {
            error(CONFIG_FILE_ACCESS_DENIED, fname);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    error(CONFIG_FILE_CLOSE_FAILED, fname);
                }
            }
        }
    }

    public static void startAgent() throws Exception {
        String prop = System.getProperty("com.sun.management.agent.class",
                                         DEFAULT_AGENT_CLASS);
        
        // -Dcom.sun.management.agent.class=<agent classname>:<agent args>
        String[] values = prop.split(":"); 
       if (values.length < 1 || values.length > 2) {
            error(AGENT_CLASS_INVALID, "\"" + prop + "\"");
        }
        String cname = values[0];
        String args = (values.length == 2 ? values[1] : null);

        if (cname == null || cname.length() == 0) {
            error(AGENT_CLASS_INVALID, "\"" + prop + "\"");
        }

        if (cname != null) {
            try {
                // Instantiate the named class. 
                // invoke the premain(String args) method
                Class clz = ClassLoader.getSystemClassLoader().loadClass(cname);
                Method premain = clz.getMethod("premain",
                                               new Class[] { String.class });
                premain.invoke(null, /* static */
                               new Object[] { args });
            } catch (ClassNotFoundException ex) {
                error(AGENT_CLASS_NOT_FOUND, "\"" + cname + "\"");
            } catch (NoSuchMethodException ex) {
                error(AGENT_CLASS_PREMAIN_NOT_FOUND, "\"" + cname + "\"");
            } catch (SecurityException ex) {
                error(AGENT_CLASS_ACCESS_DENIED);
            } catch (Exception ex) {
                String msg = (ex.getCause() == null 
                                 ? ex.getMessage() 
                                 : ex.getCause().getMessage());
                error(AGENT_CLASS_FAILED, msg);
            }
        }
    }

    public static void error(String key) {
        System.err.print(getText("agent.err.error") + ": " + getText(key));
        System.exit(1);
    }

    public static void error(String key, String[] params) {
        if (params == null || params.length == 0) {
            error(key);
        } else {
            StringBuffer message = new StringBuffer(params[0]);
            for (int i = 1; i < params.length; i++) {
                message.append(" " + params[i]);
            }
            error(key, message.toString());
        }
    }


    public static void error(String key, String message) {
        System.err.print(getText("agent.err.error") + ": " + getText(key));
        System.err.println(": " + message);
        System.exit(1);
    }

    public static void error(Exception e) {
        e.printStackTrace();
        System.err.print(getText(AGENT_EXCEPTION) + ": " + e.toString());
        System.exit(1);
    }

    private static void initResource() {
        try {
            messageRB =
                ResourceBundle.getBundle("sun.management.resources.agent");
        } catch (MissingResourceException e) {
            throw new Error("Fatal: Resource for management agent is missing");
        }
    }

    public static String getText(String key) {
        if (messageRB == null) {
            initResource();
        }
        try {
            return messageRB.getString(key);
        } catch (MissingResourceException e) {
            return "Missing management agent resource bundle: key = \"" + key + "\"";
        }
    }

    public static String getText(String key, String... args) {
        if (messageRB == null) {
            initResource();
        }
	String format = messageRB.getString(key);
	if (format == null) {
	    format = "missing resource key: key = \"" + key + "\", " +
		"arguments = \"{0}\", \"{1}\", \"{2}\"";
	}
	return MessageFormat.format(format, (Object[]) args);
    }

}
