/*
 * @(#)DebugHelperImpl.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.lang.reflect.*;
import java.util.*;

/*
 * Implementation of DebugHelper for debug compiled classes
 *
 * Supports debug tracing and assertions
 */
abstract class DebugHelperImpl extends DebugHelper {
    protected static DebugSettings 		settings;
    protected static DebugHelperImpl		globalDebugHelperImpl;
    private static boolean			initialized = false;

    private static final String PROP_ON = "on";
    private static final String PROP_TRACE = "trace";
    private static final String PROP_ASSERT = "assert";
    private DebugHelperImpl	parent = null;
    private boolean		tracingOn = on;
    private boolean		assertionsOn = on;

    static final void initGlobals() {
	if ( !initialized ) {
	    NativeLibLoader.loadLibraries();
	    initialized = true;
	    // first time anyone called us
	    settings = DebugSettings.getInstance();
	    globalDebugHelperImpl = GlobalDebugHelperImpl.getInstance();
	}
    }

    /*
     * Factory method to create a DebugHelper object for a
     * given class
     */
    static final DebugHelper factoryCreate(Class classToDebug) {
	initGlobals();
	return new ClassDebugHelperImpl(classToDebug);
    }

    protected DebugHelperImpl(DebugHelperImpl parent) {
	this.parent = parent;
    }

    public final synchronized void setAssertOn(boolean enabled) {
	assertionsOn = enabled;
    }

    public final synchronized void setTraceOn(boolean enabled) {
	tracingOn = enabled;
    }

    public final synchronized void setDebugOn(boolean enabled) {
	try {
	    Class	dbgClassObj = DebugHelper.class;
	    Field	fieldOn = dbgClassObj.getDeclaredField(DBG_ON_FIELD_NAME);
    
	    if ( !Modifier.isFinal(fieldOn.getModifiers()) ) {
	    // if DebugHelper.on is non-final, set it
		fieldOn.setBoolean(this, enabled);
	    }//otherwise we're in a production build and do nothing...
	} catch (NoSuchFieldException nfe) {
	    nfe.printStackTrace();
	} catch (IllegalAccessException iae ) {
	    iae.printStackTrace();
	}
    }

    public final synchronized void println(Object object) {
	if (tracingOn) {
	    printlnImpl(object.toString());
	}
    }

    public final synchronized void print(Object object) {
	if (tracingOn) {
	    printImpl(object.toString());
	}
    }
    
    public final synchronized String toString() {
	String strval;
	strval ="Debug {"+
		"	on="+on+", " +
		"	trace="+tracingOn+", "+
		"	assert="+assertionsOn+
		"}";
	return strval;
    }

    static native synchronized void printlnImpl(String str);
    static native synchronized void printImpl(String str);

    protected native synchronized void setCTracingOn(boolean enabled);
    protected native synchronized void setCTracingOn(boolean enabled, String file);
    protected native synchronized void setCTracingOn(boolean enabled, String file, int line);

    public final synchronized void printStackTrace() {
	if (tracingOn) {
	    Thread.dumpStack();
	}
    }

    public final synchronized void assertion(boolean expr) {
	assertion(expr, "");
    }

    public final synchronized void assertion(boolean expr, String msg) {
	if (assertionsOn && expr == false) {
	    throw new AssertionFailure(msg);
	}
    }
    
    protected void setParent(DebugHelperImpl dbgHelperImpl) {
	parent = dbgHelperImpl;
    }
    
    protected DebugHelperImpl getParent() {
	return parent;
    }
    
    protected void loadSettings() {
	boolean	debugSelf =	getBoolean(PROP_ON, parent != null ? parent.on : true);
	boolean	assertSelf =	getBoolean(PROP_ASSERT, parent != null ? parent.assertionsOn : true);
	boolean	traceSelf =	getBoolean(PROP_TRACE, parent != null ? parent.tracingOn : false);
	
	// set global debug settings
	setDebugOn(debugSelf);
	setAssertOn(assertSelf);
	setTraceOn(traceSelf);
	//println(debug);
    }

    /**
     * Gets named boolean property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return boolean value of the named property
     */
    protected synchronized boolean getBoolean(String key, boolean defval) {
	String	value = getString(key, String.valueOf(defval));
	return value.equalsIgnoreCase("true");
    }
    
    /**
     * Gets named integer property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return integer value of the named property
     */
    protected synchronized int getInt(String key, int defval) {
	String	value = getString(key, String.valueOf(defval));
	return Integer.parseInt(value);
    }
    
    /**
     * Gets named String property
     * @param key	Name of property
     * @param defval	Default value if property does not exist
     * @return string value of the named property
     */
    protected synchronized String getString(String key, String defval) {
	String value = settings.getString(key, defval);
	return value;
    }
    
    class AssertionFailure extends Error {
	AssertionFailure(String msg) {
	    super(msg);
	}
    }
}

class GlobalDebugHelperImpl extends DebugHelperImpl {
    private static final String PROP_CTRACE = "ctrace";
    private static final int	PROP_CTRACE_LEN = PROP_CTRACE.length();
    private static DebugHelperImpl instance = null;
    
    private boolean	ctracingOn;
    
    static final DebugHelperImpl getInstance() {
	if (instance == null) {
	    instance = new GlobalDebugHelperImpl();
	}
	return instance;
    }
    
    private GlobalDebugHelperImpl() {
	super(null);
	setParent(this);
 	loadSettings();
    }
    
    protected void loadSettings() {
	super.loadSettings();
	loadNativeSettings();
    }
    
    /*
     * Loads settings related to native (C/C++) code
     */
    private void loadNativeSettings() {
	boolean	ctracingOn;
	
	ctracingOn = getBoolean(PROP_CTRACE, false);
	setCTracingOn(ctracingOn);
	
	//
	// Filter out file/line ctrace properties from debug settings
	//
	Vector		traces = new Vector();
	Enumeration 	enum_ = settings.getPropertyNames();
	
	while ( enum_.hasMoreElements() ) {
	    String key = (String)enum_.nextElement();
	    if ( key.startsWith(PROP_CTRACE) && key.length() > PROP_CTRACE_LEN ) {
		traces.addElement(key);
	    }
	}
	
	// sort traces list so file-level traces will be before line-level ones
	Collections.sort(traces);
	
	//
	// Setup the trace points
	//
	Enumeration	enumTraces = traces.elements();
	
	while ( enumTraces.hasMoreElements() ) {
	    String	key = (String)enumTraces.nextElement();
	    String 	trace = key.substring(PROP_CTRACE_LEN+1);
	    String	filespec;
	    String	linespec;
	    int		delim= trace.indexOf('@');
	    boolean	enabled;
	    
	    // parse out the filename and linenumber from the property name
	    filespec = delim != -1 ? trace.substring(0, delim) : trace;
	    linespec = delim != -1 ? trace.substring(delim+1) : "";
	    enabled = settings.getBoolean(key, false);
	    //System.out.println("Key="+key+", File="+filespec+", Line="+linespec+", Enabled="+enabled);
	    
	    if ( linespec.length() == 0 ) {
	    // set file specific trace setting
	    	setCTracingOn(enabled, filespec);
	    } else {
	    // set line specific trace setting
		int	linenum = Integer.parseInt(linespec, 10);
		setCTracingOn(enabled, filespec, linenum);
	    }
	}
    }
}
    
class PackageDebugHelperImpl extends DebugHelperImpl {
    private static HashMap	hashMap = new HashMap();
    private String		packageName;
    
    private PackageDebugHelperImpl(Package pkg) {
	super(globalDebugHelperImpl);
	packageName = pkg.getName();
 	loadSettings();
   }
    
    public synchronized String getString(String key, String defval) {
	return super.getString(key+"."+packageName, defval);
    }

    static DebugHelperImpl getInstance(Package pkg) {
	if (pkg == null) {
	// not in a package, so return global debug impl
	    return globalDebugHelperImpl;
	}
	
	PackageDebugHelperImpl	pkgDebugHelperImpl;
	pkgDebugHelperImpl = (PackageDebugHelperImpl)hashMap.get(pkg);
	if ( pkgDebugHelperImpl == null ) {
	// no package debug impl created for this package yet, so make one
	    pkgDebugHelperImpl = new PackageDebugHelperImpl(pkg); 
	    hashMap.put(pkg, pkgDebugHelperImpl);
	}
	return pkgDebugHelperImpl;
    }
}

class ClassDebugHelperImpl extends DebugHelperImpl {
    private String	className;
    
    ClassDebugHelperImpl(Class classToDebug) {
	super(globalDebugHelperImpl);
	checkDeclaration(classToDebug);
	className = classToDebug.getName();
	Package	pkg = classToDebug.getPackage();
	DebugHelperImpl parent = PackageDebugHelperImpl.getInstance(pkg);
	setParent(parent);
 	loadSettings();
   }

    public synchronized String getString(String key, String defval) {
	return super.getString(key+"."+className, defval);
    }
    
    /*
     * Verifies that the class the DebugHelp object is declared in has
     * declared it to be 'private static final DebugHelp dbg'. This enforces
     * naming across classes and prevents per-instance Debug objects.
     * (Debug objects are only useful at the class level.)
     */
    private void checkDeclaration(Class classToDebug) {
	//
	// verify that the class has a private static field named 'dbg'
	//
	boolean		isDbgFieldOK = false;
	Field   	dbgField = null;
	final Class	classToCheck = classToDebug;

	// go into privileged mode so we can snoop into the class private fields
	dbgField = (Field)java.security.AccessController.doPrivileged(
	    new java.security.PrivilegedAction() {
		public Object run() {
		    try {
			Field field = classToCheck.getDeclaredField(DBG_FIELD_NAME);
			field.setAccessible(true);
			return field;
		    } catch(NoSuchFieldException nfe) {
			nfe.printStackTrace();
		    } catch(SecurityException se) {
			se.printStackTrace();
		    }
		    return null;
		}
	    }
	);

	// check that the field is a private static instance of Debug
	isDbgFieldOK =  dbgField != null &&
			dbgField.getType() == DebugHelper.class &&
			Modifier.isPrivate(dbgField.getModifiers()) &&
			Modifier.isStatic(dbgField.getModifiers()) &&
			Modifier.isFinal(dbgField.getModifiers());
	if (!isDbgFieldOK) {
	    throw new AssertionFailure("Incorrect or missing declaration of dbg field. Must be declared 'private static final DebugHelper dbg'");
	}
    }
}
