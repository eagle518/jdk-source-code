/*
 * @(#)DebugSettings.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.io.*;
import java.util.*;

/*
 * Internal class that manages sun.awt.Debug settings.
 * Settings can be specified on a global, per-package,
 * or per-class level.
 *
 * Properties affecting the behaviour of the Debug class are
 * loaded from the awtdebug.properties file at class load
 * time. The properties file is assumed to be in the
 * user.home directory. A different file can be used
 * by setting the awtdebug.properties system property.
 * 	e.g. java -Dawtdebug.properties=foo.properties
 *
 * Only properties beginning with 'awtdebug' have any
 * meaning-- all other properties are ignored.
 *
 * You can override the properties file by specifying
 * 'awtdebug' props as system properties on the command line.
 *	e.g. java -Dawtdebug.trace=true
 * Properties specific to a package or a class can be set
 * by qualifying the property names as follows:
 *	awtdebug.<property name>.<class or package name>
 * So for example, turning on tracing in the com.acme.Fubar
 * class would be done as follows:
 *	awtdebug.trace.com.acme.Fubar=true
 *
 * Class settings always override package settings, which in
 * turn override global settings.
 */
final class DebugSettings {
    /* standard debug property key names */
    static final String PREFIX = "awtdebug";
    static final String PROP_FILE = "properties";

    /* default property settings */
    private static final String	DEFAULT_PROPS[] = {
	"awtdebug.assert=true",
	"awtdebug.trace=false",
	"awtdebug.on=true",
	"awtdebug.ctrace=false"
    };

    /* global instance of the settings object */
    private static DebugSettings	instance = null;
    
    private Properties	props = new Properties();

    static DebugSettings getInstance() {
	if (instance == null) {
	    instance = new DebugSettings();
	}
	return instance;
    }
    
    private DebugSettings() {
	new java.security.PrivilegedAction() {
	    public Object run() {
		loadProperties();
		return null;
	    }
	}.run();
    }
    
    /*
     * Load debug properties from file, then override
     * with any command line specified properties
     */
    private synchronized void loadProperties() {
	// setup initial properties
	java.security.AccessController.doPrivileged(
	    new java.security.PrivilegedAction()
	{
	    public Object run() {
		loadDefaultProperties();
		loadFileProperties();
		loadSystemProperties();
		return null;
	    }
	});
	
	// echo the initial property settings
	println(this);
    }
    
    public String toString() {
	Enumeration enum_ = props.propertyNames();
	ByteArrayOutputStream bout = new ByteArrayOutputStream();
	PrintStream pout = new PrintStream(bout);

	pout.println("------------------");
	pout.println("AWT Debug Settings");
	pout.println("------------------");
	while ( enum_.hasMoreElements() ) {
	    String	key = (String)enum_.nextElement();
	    String	value = props.getProperty(key,"");
	    pout.println(key+"="+value);
	}
	pout.println("------------------");
	return new String(bout.toByteArray());
    }
    
    /*
     * Sets up default property values
     */
    private void loadDefaultProperties() {
	// is there a more inefficient way to setup default properties?
	// maybe, but this has got to be close to 100% non-optimal
	try {
	    for ( int nprop = 0; nprop < DEFAULT_PROPS.length; nprop++ ) {
		StringBufferInputStream in = new StringBufferInputStream(DEFAULT_PROPS[nprop]);
		props.load(in);
		in.close();
	    }
	} catch(IOException ioe) {
	}
    }

    /*
     * load properties from file, overriding defaults
     */
    private void loadFileProperties() {
	String		propPath;
	Properties	fileProps;
	
	// check if the user specified a particular settings file
	propPath = System.getProperty(PREFIX + "." + PROP_FILE, "");
	if (propPath.equals("")) {
	// otherwise get it from the user's home directory
	    propPath = System.getProperty("user.home", "") +
			File.separator +
			PREFIX + "." + PROP_FILE;
	}

	File	propFile = new File(propPath);
	try {
	    println("Reading debug settings from '" + propFile.getCanonicalPath() + "'...");
	    FileInputStream	fin = new FileInputStream(propFile);
	    props.load(fin);
	    fin.close();
	} catch ( FileNotFoundException fne ) {
	    println("Did not find settings file.");
	} catch ( IOException ioe ) {
	    println("Problem reading settings, IOException: " + ioe.getMessage());
	}
    }

    /*
     * load properties from system props (command line spec'd usually),
     * overriding default or file properties
     */
    private void loadSystemProperties() {
	// override file properties with system properties
	Properties sysProps = System.getProperties();
	Enumeration enum_ = sysProps.propertyNames();
	while ( enum_.hasMoreElements() ) {
	    String key = (String)enum_.nextElement();
	    String value = sysProps.getProperty(key,"");
	    // copy any "awtdebug" properties over
	    if ( key.startsWith(PREFIX) ) {
		props.setProperty(key, value);
	    }
	}
    }
    
    /**
     * Gets named boolean property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return boolean value of the named property
     */
    public synchronized boolean getBoolean(String key, boolean defval) {
	String	value = getString(key, String.valueOf(defval));
	return value.equalsIgnoreCase("true");
    }
    
    /**
     * Gets named integer property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return integer value of the named property
     */
    public synchronized int getInt(String key, int defval) {
	String	value = getString(key, String.valueOf(defval));
	return Integer.parseInt(value);
    }
    
    /**
     * Gets named String property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return string value of the named property
     */
    public synchronized String getString(String key, String defval) {
	String	actualKeyName = PREFIX + "." + key;
	String	value = props.getProperty(actualKeyName, defval);
	//println(actualKeyName+"="+value);
	return value;
    }
    
    public synchronized Enumeration getPropertyNames() {
	Vector		propNames = new Vector();
	Enumeration	enum_ = props.propertyNames();
	
	// remove global prefix from property names
	while ( enum_.hasMoreElements() ) {
	    String propName = (String)enum_.nextElement();
	    propName = propName.substring(PREFIX.length()+1);
	    propNames.addElement(propName);
	}
	return propNames.elements();
    }

    
    private void println(Object object) {
	DebugHelperImpl.printlnImpl(object.toString());
    }
}
