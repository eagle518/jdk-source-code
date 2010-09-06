/*
 * @(#)JREInfo.java	1.17 04/03/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.util.*;
import java.lang.*;
import java.io.File;
import com.sun.deploy.util.Trace;

public class JREInfo {

    private static ArrayList _jres = new ArrayList();

    private String _platform;
    private String _product;
    private String _location;
    private String _path;
    private String _osname;
    private String _osarch;
    private boolean _enabled;
    private boolean _registered;
    private boolean _system;

    public String getPlatform() { return _platform; }
    public String getProduct() { return _product; }
    public String getLocation() { return _location; }
    public String getPath() { return _path; }
    public String getDebugJavaPath() { return Config.getInstance().getDebugJavaPath(_path); }
    public String getOSName() { return _osname; }
    public String getOSArch() { return _osarch; }
    public boolean isEnabled() { return _enabled; }
    public boolean isRegistered() { return _registered; }
    public boolean isSystemJRE() { return _system; }

    public void setPlatform(String value) { _platform = value; }
    public void setProduct(String value) { _product = value; }
    public void setLocation(String value) { _location = value; }
    public void setPath(String value) { _path = value; }
    public void setOSName(String value) { _osname = value; }
    public void setOSArch(String value) { _osarch = value; }
    public void setEnabled(boolean value) { _enabled = value; }
    public void setRegistered(boolean value) { _registered = value; }
    public void setSystemJRE(boolean value) { _system = value; }

    public JREInfo(String platform,
    		   String product,
    		   String location,
    		   String path,
    		   String osname,
    		   String osarch,
    		   boolean enabled,
    		   boolean registered) {
    	_platform = platform;
	_product = product;
        _location = location;
	_path = path;
	_osname = osname;
	_osarch = osarch;
	_enabled = enabled;
	_registered = registered;
	_system = false;
	if (_osname == null && _osarch == null) {
	    _osname = Config.getOSName();
	    _osarch = Config.getOSArch();
	}
	if (_location == null) {
	    _location = Config.getProperty(Config.JAVAWS_JRE_INSTALL_KEY);
	}
    }

    public JREInfo(int index, Properties p, boolean system) {
	this(p.getProperty(
		Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_PLATFORM_ID),
	     p.getProperty(
		Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_PRODUCT_ID),
	     p.getProperty(
		Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_LOCATION),
	     p.getProperty(
		Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_PATH),
	     p.getProperty(
		Config.JAVAWS_JRE_KEY + index  + Config.JAVAWS_JRE_OS_NAME),
	     p.getProperty(
		Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_OS_ARCH),
	     false, 
	     false);

	String str = p.getProperty(
	    Config.JAVAWS_JRE_KEY + index  + Config.JAVAWS_JRE_ISENABLED); 
	if (str != null && Boolean.valueOf(str).booleanValue()) {
	    setEnabled(true);
	}

	str = p.getProperty(
	    Config.JAVAWS_JRE_KEY + index  + Config.JAVAWS_JRE_ISREGISTERED); 
	if (str != null && Boolean.valueOf(str).booleanValue()) {
	    setRegistered(true);
	}
	setSystemJRE(system);
    }

    public JREInfo(JREInfo toCopy) {
    	this( 	toCopy.getPlatform(),
		toCopy.getProduct(),
		toCopy.getLocation(),
		toCopy.getPath(),
		toCopy.getOSName(),
		toCopy.getOSArch(),
		toCopy.isEnabled(),
		toCopy.isRegistered());
        setSystemJRE(toCopy.isSystemJRE()); 
    }

    public static void addJRE(JREInfo je) {
	_jres.add(je);
    }

    public static void removeJRE(int index) {
	_jres.remove(index);
    }

    public static JREInfo getJREInfo(int index)  {
	return (JREInfo) _jres.get(index);
    }

    public static void setJREInfo(int index, JREInfo je) {
	_jres.set(index, je);
    }

    public static void clear() {
	_jres.clear();
    }

    public static JREInfo[] get() {
        return (JREInfo []) _jres.toArray(new JREInfo[0]);
    }

    public static void initialize(Properties sysJreProps, Properties jreProps) {

	ArrayList indices = new ArrayList();

	int last = -1;
	_jres.clear();
        Enumeration en = jreProps.keys();
	while(en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    if (key.startsWith(Config.JAVAWS_JRE_KEY)) {
		int index = getJREIndex(key);
		if (index >=0 && index != last) {
		    Integer ii = new Integer(index);
		    if (!indices.contains(ii)) {
		    	indices.add(ii);
            		addJRE(new JREInfo(index, jreProps, false));
		    }
		    last = index;
		}
	    }
	}
        en = sysJreProps.keys();
	last = -1;
	indices = new ArrayList();
	while(en.hasMoreElements()) {
	    String key = (String) en.nextElement();
	    if (key.startsWith(Config.JAVAWS_JRE_KEY)) {
		int index = getJREIndex(key);
		if (index >=0 && index != last) {
		    Integer ii = new Integer(index);
		    if (!indices.contains(ii)) {
		    	indices.add(ii);
            		addJRE(new JREInfo(index, sysJreProps, true));
		    }
		    last = index;
		}
	    }
	}
	// add the home jre into the system list if not already there

	JREInfo home = getHomeJRE();
	if (home != null) {
	    boolean found = false;
	    for (int i=0; i<_jres.size(); i++) {
	        JREInfo jre = (JREInfo) _jres.get(i);
	        if (jre.isSystemJRE() && (jre.getPath() != null) &&
		    jre.getPath().equals(home.getPath())) {
		    found = true;
	        }
	    }
	    if (!found) {
	        JREInfo sys = new JREInfo(home);
		sys.setEnabled(true);
		sys.setLocation(Config.getProperty(Config.JAVAWS_JRE_INSTALL_KEY));
		sys.setRegistered(true);
		sys.setSystemJRE(true);
		String version = Config.getProperty(Config.VERSION_UPDATED_KEY);
		if (version != null && (version.length() >= 3)) {
		    sys.setPlatform(version.substring(0,3));
		    sys.setProduct(version);
		}
		addJRE(sys);
	    }
	}
    }

    private static int getJREIndex(String key) {
	int i1 = Config.JAVAWS_JRE_KEY.length();
	int i2 = key.indexOf(".", i1);
	if (i2 > i1) {
	    String indexString = key.substring(i1, i2);
	    try {
		return Integer.parseInt(indexString);	
	    } catch (NumberFormatException nfe) {
	    }
	}
	return -1;
    }

    public boolean isOsInfoMatch(String osname, String osarch) {
        // assume it is for the current platform if osname and
        // osarch is not specified
        if (_osname == null || _osarch == null) return true;

	// only compare arch if osname is SunOS
	if (_osname.equals("SunOS")) { 
	    return (_osname.equals(osname) && _osarch.equals(osarch));
	} else {
	    return _osname.equals(osname);
	}

    }

    public String toString() {
	int index = 0;
	while (index < _jres.size()) {
	    if (this.equals(_jres.get(index))) { 
		break; 
	    }
	    index++;
	}

	return ((index < _jres.size()) ? 
		("JREInfo for index "+index+":\n") :
		("JREInfo (not in list):\n")) +
            "    platform is: "+_platform + "\n" +
            "    product is: "+_product + "\n" +
            "    location is: "+_location + "\n" +
            "    path is: "+_path + "\n" +
            "    osname is: "+_osname + "\n" +
            "    osarch is: "+_osarch + "\n" +
            "    enabled is: "+_enabled + "\n" +
            "    registered is: "+_registered + "\n" +
            "    system is: "+_system + "\n" +
	    "";
    }

    public static void printJREs() {
	for (int i=0; i<_jres.size(); i++) {
	    Trace.println(_jres.get(i).toString());
	}
    }

    public static String getKnownPlatforms() {
        StringBuffer knownPlatforms = new StringBuffer();
        for(int i = 0; i < _jres.size(); i++) {
	    JREInfo jre = (JREInfo) _jres.get(i);
            knownPlatforms.append(jre.getPlatform());
            knownPlatforms.append(" ");
        }
        return knownPlatforms.toString();
    }

    public static String getDefaultJavaPath() {
	JREInfo home = getHomeJRE();
	if (home != null) {
	    return home.getPath();
	}
	return ((JREInfo) _jres.get(0)).getPath();
    }

    public static JREInfo getHomeJRE() {
        String home = Config.getJavaHome();
        File homeDir = new File(home);
        for (int i=0; i<_jres.size(); i++) {
	    JREInfo jre = (JREInfo) _jres.get(i);
            String path = jre.getPath();
            if (path != null) {
                File bin = (new File(path)).getParentFile();
                // If path is not null, but incorrect, exception is thrown when
                // ControlPanel starts.  Should not happen.  Check if bin is null.
                if (bin!=null && bin.getParentFile().equals(homeDir)) {
                    return jre;
                }
            }
        }
	return null;
    }

    public static void removeJREsIn(String pathPrefix) {
	Iterator it = _jres.iterator();
	while (it.hasNext()) {
	    JREInfo jre = (JREInfo) it.next();
            String path = jre.getPath();
            if (path != null) {
		if (path.startsWith(pathPrefix)) {
		    it.remove();
		}
	    }
	}
    }


}

