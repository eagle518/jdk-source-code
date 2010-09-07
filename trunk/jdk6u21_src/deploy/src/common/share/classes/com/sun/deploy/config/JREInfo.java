/*
 * @(#)JREInfo.java	1.45 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.util.*;
import java.lang.*;
import java.io.File;
import com.sun.deploy.util.SyncAccess;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.VersionID;

/**
 * JREInfo contains a static list of all installed JRE's,
 * system wide or user configured/added.
 *
 * The list is ordered with the highest product version as the first element
 * and declining.
 *
 */
public class JREInfo {
    private static ArrayList _jres = new ArrayList();
    private static SyncAccess syncAccess = new SyncAccess(SyncAccess.SHARED_READ_MODE);

    private String _platform;
    private VersionID _platformVersion;
    private String _product;
    private VersionID _productVersion;
    private String _location;
    private String _path;
    private NativePlatform _nativePlatform;
    private String _vm_args;
    private boolean _enabled;
    private boolean _registered;
    private boolean _system;

    public String getPlatform() { return _platform; }
    public VersionID getPlatformVersion() { return _platformVersion; }
    public String getProduct() { return _product; }
    public VersionID getProductVersion() { return _productVersion; }
    public String getLocation() { return _location; }
    public String getPath() { return _path; }
    public String getVmArgs (){ return _vm_args; }
    public String getDebugJavaPath() { return Config.getInstance().getDebugJavaPath(_path); }
    public String getOSName() { return _nativePlatform.getOSName(); }
    public String getOSArch() { return _nativePlatform.getOSArch(); }
    public boolean isEnabled() { return _enabled; }
    public boolean isRegistered() { return _registered; }
    public boolean isSystemJRE() { return _system; }

    public void setPlatform(String value) { _platform = value; _platformVersion = new VersionID(value); }
    public void setProduct(String value) { _product = value;  _productVersion = new VersionID(value); }
    public void setLocation(String value) { _location = value; }
    public void setPath(String value) { _path = Config.getJavaCommand(Config.getLongPathName(value)); }
    public void setVmArgs(String vmArgs){ _vm_args = vmArgs; }
    public void setEnabled(boolean value) { _enabled = value; }
    public void setRegistered(boolean value) { _registered = value; }
    public void setSystemJRE(boolean value) { _system = value; }

    public void setOSName(String value) { _nativePlatform = new NativePlatform(value, getOSArch()); }
    public void setOSArch(String value) { _nativePlatform = new NativePlatform(getOSName(), value); }

    public JREInfo(    String platform,
                       String product,
                       String location,
                       String path,
                       String args,
                       String osname,
                       String osarch,
                       boolean enabled,
                       boolean registered) {
        setProduct(product);
        if(null!=platform) {
            setPlatform(platform);
        } else {
            setPlatform(getPlatformByProduct(product));
        }
        _location = location;
        _path = Config.getJavaCommand(Config.getLongPathName(path));
        _vm_args = args;
        _enabled = enabled;
        _registered = registered;
        _system = false;
        _nativePlatform = new NativePlatform(osname, osarch);
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
                Config.JAVAWS_JRE_KEY + index + Config.JAVAWS_JRE_ARGS),
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
           this(toCopy.getPlatform(),
                toCopy.getProduct(),
                toCopy.getLocation(),
                toCopy.getPath(),
                toCopy.getVmArgs(),
                toCopy.getOSName(),
                toCopy.getOSArch(),
                toCopy.isEnabled(),
                toCopy.isRegistered());
        setSystemJRE(toCopy.isSystemJRE()); 
    }

    // match against current native platform
    public boolean isOsInfoMatch() {
        return _nativePlatform.compatible(NativePlatform.getCurrentNativePlatform());
    }

    // match against given native platform
    public boolean isOsInfoMatch(String osname, String osarch) {
        return _nativePlatform.compatible(new NativePlatform(osname, osarch));
    }

    public String toString() {
        int index = findJREByPath_int(this, _jres);
        return ((index >= 0) ? 
                ("JREInfo for index "+index+":\n") :
                ("JREInfo (not in list):\n")) +
            "    platform is: "+_platform + "\n" +
            "    product is: "+_product + "\n" +
            "    location is: "+_location + "\n" +
            "    path is: "+_path + "\n" +
            "    args is: "+_vm_args + "\n" +
            "    native platform is: "+_nativePlatform.toString() + "\n" +
            "    enabled is: "+_enabled + "\n" +
            "    registered is: "+_registered + "\n" +
            "    system is: "+_system + "\n" +
            "";
    }

    // Return the JRE path, for both, javapi and javaws.
    // JAVAWS delivers the full path to the java executable,
    // which is stripped of using this method.
    public String getJREPath()
    {
        return Config.getJavaHome(getPath());
    }


    //
    // public static .. synchronized
    // 

    /**
     * deduce the platform ID by the product's family version
     */
    public static String getPlatformByProduct(String product) {
        int majorDot, minorDot;

	    if (product != null) {
            majorDot = product.indexOf(".", 0);
            if(majorDot>0 && majorDot<product.length()-1) {
                minorDot = product.indexOf(".", majorDot+1);
                if(minorDot>majorDot) {
                    return product.substring(0, minorDot);
                } 
                minorDot = product.indexOf("*", majorDot+1);
                if(minorDot>majorDot) {
                    return product.substring(0, minorDot);
                }
            }
        }
        return product;
    }

    public static int findJREByPath(JREInfo je, ArrayList jres) {
        int i;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            i = findJREByPath_int(je, jres);
        } finally {
            lock.release();
        }
        return i;
    }

    public static int findJREByProductVersion(JREInfo je, ArrayList jres) {
        int i;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            i = findJREByProductVersion_int(je, jres);
        } finally {
            lock.release();
        }
        return i;
    }

    public static void addJRE(JREInfo je) {
        addJRE(je, true);
    }

    public static void addJRE(JREInfo je, boolean replace) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            addJRE_int(je, replace);
        } finally {
            lock.release();
        }
    }

    public static void removeJRE(int index) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            _jres.remove(index);
        } finally {
            lock.release();
        }
    }

    public static JREInfo getJREInfo(int index)  {
        JREInfo jreInfo;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            jreInfo = (JREInfo) _jres.get(index);
        } finally {
            lock.release();
        }
        return jreInfo;
    }

    public static void setJREInfo(int index, JREInfo je) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            _jres.set(index, je);
        } finally {
            lock.release();
        }
    }

    public static void clear() {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            _jres.clear();
        } finally {
            lock.release();
        }
    }

    /*
     * return all JREInfo jres, which are javaws.
     */
    public static JREInfo[] getAll() {
        JREInfo [] jreInfos;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            jreInfos = (JREInfo []) _jres.toArray(new JREInfo[0]);
        } finally {
            lock.release();
        }
        return jreInfos;
    }

    public static void initialize(Properties sysJreProps, Properties jwsJreProps) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            ArrayList indices = new ArrayList();

            int last = -1;
            _jres.clear();
            Enumeration en = jwsJreProps.keys();
            while(en.hasMoreElements()) {
                String key = (String) en.nextElement();
                if (key.startsWith(Config.JAVAWS_JRE_KEY)) {
                    int index = getJREIndex_int(key);
                    if (index >=0 && index != last) {
                        Integer ii = new Integer(index);
                        if (!indices.contains(ii)) {
                                indices.add(ii);
                                addJRE_int(new JREInfo(index, jwsJreProps, false), true);
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
                    int index = getJREIndex_int(key);
                    if (index >=0 && index != last) {
                        Integer ii = new Integer(index);
                        if (!indices.contains(ii)) {
                                indices.add(ii);
                                addJRE_int(new JREInfo(index, sysJreProps, true), true);
                        }
                        last = index;
                    }
                }
            }

	    validateJREs_int();
            validateHomeJRE_int();
        } finally {
            lock.release();
        }
    }

    /**
     * Import old deployment.javapi.* entries into JREInfo
     * We only import jre entries, no jdk
     */
    public static void importJpiEntries(Properties jpiJreProps) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
	    HashMap paths   = new HashMap();
	    HashMap args    = new HashMap();
	    HashMap enabled = new HashMap();
	    HashMap osNames = new HashMap();
	    HashMap osArchs = new HashMap();
	    
	    Enumeration en = jpiJreProps.keys();
	    while(en.hasMoreElements()) {
		String key = (String) en.nextElement();
		String val = (String) jpiJreProps.get(key);
		             
		int begin, end;
		String pathStr;
		String argStr;
		String version = null;
		
		// read properties to maps
		if (key.startsWith(Config.JPI_JRE_KEY)) {                          
		    begin = Config.JPI_JRE_KEY.length(); 
		    
		    if (key.endsWith(Config.JPI_JAVA_OSARCH)) {
			end = key.indexOf(Config.JPI_JAVA_OSARCH);
			version = key.substring(begin, end);
			osArchs.put(version, val);
		    } else if (key.endsWith(Config.JPI_JAVA_OSNAME)) {
			end = key.indexOf(Config.JPI_JAVA_OSNAME);
			version = key.substring(begin, end);
			osNames.put(version, val);
		    } else if (key.endsWith(Config.JPI_JAVA_PATH)) {
			end = key.indexOf(Config.JPI_JAVA_PATH);
			version = key.substring(begin, end);
			paths.put(version, val);
		    } else if (key.endsWith(Config.JPI_JAVA_ARGS)) {
			end = key.indexOf(Config.JPI_JAVA_ARGS);
			version = key.substring(begin, end);
			args.put(version, val);
		    } else if (key.endsWith(Config.JPI_JAVA_ENABLED)) {
			// only exists in earlier 6u10 builds prior to b23
			end = key.indexOf(Config.JPI_JAVA_ENABLED);
			version = key.substring(begin, end);
			enabled.put(version, Boolean.valueOf(val));
		    }
		}
	    }
        
	    // Very likely these entries are already in the _jres since they
	    // are also javaws jre entries. When addJRE_int() to _jres list,
	    // duplicates by path will be removed.
	    Iterator iter = paths.keySet().iterator();
	    while(iter.hasNext()) {
		String jreVersion = (String) iter.next();
		String jrePath = (String)paths.get(jreVersion);
		String jreOsname = (String)osNames.get(jreVersion);
		String jreOsarch = (String)osArchs.get(jreVersion);
		
		String jreArgs = (String)args.get(jreVersion);
		Boolean tmpEnabled = (Boolean) enabled.get(jreVersion);
		boolean jreEnabled = (tmpEnabled == null) ? true : tmpEnabled.booleanValue();
		JREInfo jre = new JREInfo((String)null, jreVersion, (String)null,
					  jrePath, jreArgs, jreOsname, jreOsarch, jreEnabled, false);
		
		addJRE_int(jre, true);
	    }        
	} finally {
	    lock.release();
	}
    }

    /*
     * Add system-wide JREs to the list.
     */
    public static void setInstalledJREList(Vector vec) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
            // Vector contains: version, path pairs.                
            for (int i = 0; i < vec.size(); i+=2){
                String version = (String)vec.get(i);
                String path = (String)vec.get(i + 1);

                /*
                 * Do not add x.x to the list, because it will point to the latest
                 * installed JRE and we'll have duplicate path entries.
                 * Example:
                 * Do not add 1.5 to the list, because if you install 1.5.0_01, they will
                 * be pointing to the same JRE.
                 */
                if ( version.lastIndexOf(".") > 2 ){
                    // These are system-wide JREs. And they are from the registry on windows.
		    // On Unix, these are the current running JRE. They will be marked as 
		    // "registered" and not allowed to be removed in control panel.
                    JREInfo jre = new JREInfo((String) null, version, (String)null, path, "", 
                                              Config.getOSName(), Config.getOSArch(), true, true);
		    // if we found the jre in the _jres list, mark it as "registered
		    int idx = findJREByPath_int(jre, _jres);
		    if (idx < 0) {
			addJRE_int(jre, false);
		    } else {
			((JREInfo) _jres.get(idx)).setRegistered(true);
		    }
                }
            }
	    validateJREs_int();
            validateHomeJRE_int();
        } finally {
            lock.release();
        }
    }
        
    public static void printJREs() {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            System.out.println("\nJREInfo: "+_jres.size()+" entries");
            for (int i=0; i<_jres.size(); i++) {
                System.out.println("JREInfo "+i+":");
                System.out.println(_jres.get(i).toString());
            }
        } finally {
            lock.release();
        }
    }

    public static void traceJREs() {
        if (!Trace.isTraceLevelEnabled(TraceLevel.BASIC)) return;

        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            Trace.println("\nJREInfo: "+_jres.size()+" entries", TraceLevel.BASIC);
            for (int i=0; i<_jres.size(); i++) {
                Trace.println("JREInfo "+i+":", TraceLevel.BASIC);
                Trace.println(_jres.get(i).toString(), TraceLevel.BASIC);
            }
        } finally {
            lock.release();
        }
    }

    public static String getKnownPlatforms() {
        StringBuffer knownPlatforms = new StringBuffer();
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            for(int i = 0; i < _jres.size(); i++) {
                JREInfo jre = (JREInfo) _jres.get(i);
                knownPlatforms.append(jre.getPlatform());
                knownPlatforms.append(" ");
            }
        } finally {
            lock.release();
        }
        return knownPlatforms.toString();
    }

    public static String getDefaultJavaPath() {
        JREInfo home;
        String  homeS;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            home = getHomeJRE_int();
            if (home != null) {
                homeS = home.getPath();
            } else {
                homeS = ((JREInfo) _jres.get(0)).getPath();
            }
        } finally {
            lock.release();
        }
        return homeS;
    }

    /**
     * returns the currently running JRE (javaws),
     * found in our global JREInfo list
     */
    public static JREInfo getHomeJRE() {
        JREInfo home;
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.READ_OP);
        try {
            home = getHomeJRE_int();
        } finally {
            lock.release();
        }
        return home;
    }

    public static void removeJREsIn(String pathPrefix) {
        SyncAccess.Lock lock = syncAccess.lock(SyncAccess.WRITE_OP);
        try {
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
        } finally {
            lock.release();
        }
    }

    // helper public method to validate the jre path
    public static boolean isValidJREPath(String path) {
        if (path != null) {
            File f = new File(path);
            return (f.exists() && !f.isDirectory());
        }
        return false;
    }
    
    //
    // private static .. non synchronized
    // 

    private static int findJREByPath_int(JREInfo je, ArrayList jres) {
        int index = jres.size()-1;
        while (index >= 0) {
            JREInfo ha = (JREInfo)jres.get(index);
            if (je.isOsInfoMatch(ha.getOSName(), ha.getOSArch()) &&// native platform check
		je.isSystemJRE() == ha.isSystemJRE() &&            // system check
                (je.getPath() != null) &&                          // path
                Config.samePaths(je.getPath(), ha.getPath())) {    // path
                break; 
            }
            index--;
        }
        return index;
    }

    private static int findJREByProductVersion_int(JREInfo je, ArrayList jres) {
        String  jeL = je.getLocation();
        if(jeL==null) return -1;

        int index = jres.size()-1;
        while (index >= 0) {
            JREInfo ha = (JREInfo)jres.get(index);
            String  haL = ha.getLocation();
            if (je.isSystemJRE()==ha.isSystemJRE() &&                  // system check
                haL!=null &&                                           // location
                jeL.equals(haL) &&                                     // location
                je.getProductVersion().equals(ha.getProductVersion())) // product version
            {
                break; 
            }
            index--;
        }
        return index;
    }

    private static void addJRE_int(JREInfo je, boolean replace) {
        // replace JRE if path is the same
        if (je.getPath() != null) {
            /* we now only support 1.3+ */
            if (je.getPlatform() != null && 
                je.getPlatform().compareTo("1.3") >= 0) {
		int i = findJREByPath_int(je, _jres);
                if(i>=0 && !replace) {
                    return; // bail out .. no duplicates allowed
                }
                if(i>=0) {	    
		    // propagate vmargs from to-be-removed entry
		    JREInfo iJre = (JREInfo) _jres.get(i);
		    if ( (je.getVmArgs() == null ||"".equals(je.getVmArgs())) && 
			 null != iJre.getVmArgs()) {
			je.setVmArgs(iJre.getVmArgs());
		    }

		    _jres.remove(i); // remove existing version
                }

                // add in order, highest version first and declining
                for(i=0; i<_jres.size(); i++) {
                    JREInfo iJre = (JREInfo) _jres.get(i);
                    if(je.getProductVersion().isGreaterThanOrEqual(iJre.getProductVersion())) {
                        _jres.add(i, je); // add new before the next lower version
                        return;
                    }
                }
                _jres.add(je); // add new to the end of the list
            }
        }
    }

    /**        
     * add the home jre into the user and system list if not already there
     */
    private static void validateHomeJRE_int() {
        JREInfo jreHome = getHomeJRE_int();
        if (jreHome != null) {
            int idx = findJREByPath_int(jreHome, _jres);
	    if (idx < 0) {
		// add home jre to user list
		addJRE_int(jreHome, true);
	    }

	    JREInfo sysJreHome = new JREInfo(jreHome);
	    sysJreHome.setSystemJRE(true);
	    idx = findJREByPath_int(sysJreHome, _jres);
	    if (idx < 0) {
		addJRE_int(sysJreHome, true);
	    }

	    //remove duplicate and incorrect entry in the _jres array
	    int i;
	    for (i = 0; i < _jres.size(); i++) {
		JREInfo iJre = (JREInfo) _jres.get(i);
		// if the version and operating system info. are the same
		// but the path is different from the home JRE, remove the
		// entry from the _jres array.
		if (!iJre.isSystemJRE() &&
		    iJre.getProduct().equals(jreHome.getProduct()) &&
		    iJre.getOSName().equals(jreHome.getOSName()) &&
		    iJre.getOSArch().equals(jreHome.getOSArch()) &&
		    !Config.samePaths(iJre.getPath(), jreHome.getPath())) {
		    _jres.remove(i);
		}
	    }
        }
    }

    /**
     * validate jre path exists and remove invalid entry from the _jres array
     */
    private static void validateJREs_int() {
	int i;
	for (i = 0; i < _jres.size(); i++) {
	    JREInfo iJre = (JREInfo) _jres.get(i);
	    // validate path only if match current native platform
	    if (iJre.isOsInfoMatch() && !isValidJREPath(iJre.getPath())) {
		_jres.remove(i);
	    }
	}
    }

    private static int getJREIndex_int(String key) {
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

    // return a JREInfo matching the currently running one.
    //        This will either return a currently existing entry,
    //        while attempting to fix the product version;
    //        or a new user JREInfo (system == false). 
    //        The new JREInfo will be copied to a system JREInfo when
    //        validateHomeJRE2SystemList_int().
    private static JREInfo getHomeJRE_int() {
        // create a matching JREInfo for the currently running one
        String version  = Config.getJavaVersion();
        String path     = Config.getLongPathName(Config.getJREHome());
        JREInfo jreHome = new JREInfo((String) null, version, (String)null, path, "", 
                                  Config.getOSName(), Config.getOSArch(), true, false);

        int idx = findJREByPath_int(jreHome, _jres);
        if(idx>=0) {
            // verify and fix(version) the existing entry
            JREInfo jre = (JREInfo) _jres.get(idx);
            if ( ! jreHome.getProduct().equals(jre.getProduct()) ) {
                jre.setProduct(jreHome.getProduct());
            }
            jreHome = jre; // use the existing entry!
        }

        // Make sure the OS / arch match -- failures can
        // happen in mixed 32/64-bit environments where we
        // bootstrap things off a 32-bit JRE but end up
        // launching a 64-bit JRE
        //
        // Would be better if we had more precise information
        // about which architectures a given JRE supported
        // (i.e., both i386 and x86_64, or sparc and sparcv9)
        if (!jreHome.isOsInfoMatch()) {
            // Reset the OS name and arch based on the current values
            jreHome.setOSName(Config.getOSName());
            jreHome.setOSArch(Config.getOSArch());
        }

        return jreHome;
    }
}

