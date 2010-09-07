/*
 * %W% %E%
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.Environment;
import com.sun.deploy.config.Config;
import java.net.URL;
import com.sun.deploy.xml.*;

/**
 *  Describes a downloaded JAR file. This file can either contain
 *  Java resources or native libraries.
 */
public class JARDesc implements ResourceType {
    private URL     _location;
    private String  _version;
    private int     _size;         // Optional size of download
    private boolean _isNativeLib;
    private boolean _isLazyDownload;
    private boolean _isProgressDownload;
    private boolean _isMainFile;  // Only used for Java JAR files (a main JAR file is implicitly eager)
    private String  _part;        // The name of the part this JAR file belongs to or null if no part specified
    private ResourcesDesc _parent;   // Back-pointer to the Resources that contains this JAR
    private JARUpdater _updater = null;
    private boolean _pack200Enabled = false;
    private boolean _versionEnabled = false;

    
    public JARDesc(URL location, String version, boolean isLazy,
		   boolean isMainFile, boolean isNativeLib, String part, 
		   int size, ResourcesDesc parent) {
        this (location, version, isLazy, isMainFile, isNativeLib, part,
              size, parent, false);
    }
 
    public JARDesc(URL location, String version, boolean isLazy,
		   boolean isMainFile, boolean isNativeLib, String part, 
		   int size, ResourcesDesc parent, 
                   boolean isProgress) {
        _location = location;
        _version = version;
        _isLazyDownload = (isLazy && 
	    (!_isMainFile));    	// The main file cannot be lazy
        _isNativeLib = isNativeLib;
        _isMainFile = isMainFile;
        _part = part;
        _size = size;
        _parent = parent;
        _isProgressDownload = isProgress;
    }

    public void setPack200Enabled() {
        _pack200Enabled = true;
    }

    public void setVersionEnabled() {
        _versionEnabled = true;
    }

    public boolean isPack200Enabled() {

        if (Config.isJavaVersionAtLeast15()) {
            return _pack200Enabled;
        }

        return false;         
    }
    
    public boolean isVersionEnabled() {
        return _versionEnabled;
    }

    
    /** Type of JAR resource */
    public boolean isNativeLib() { return _isNativeLib; }
    public boolean isJavaFile()  { return !_isNativeLib; }

    public boolean isProgressJar() { return _isProgressDownload; }
    
    /** Returns URL/version for JAR file */
    public URL getLocation()   { return _location; }
    public String getVersion() { return _version; }
    
    /** Returns if resource should be downloaded lazily */
    public boolean isLazyDownload() { return _isLazyDownload; }
    public void setLazyDownload(boolean isLazy) { _isLazyDownload = isLazy; }
    
    /** Returns if this is the main JAR file */
    public boolean isMainJarFile() { return _isMainFile; }
    
    /** Return part name or null if none specified */
    public String getPartName() { return _part; }
    
    /** Return size of download. This is zero if not known */
    public int getSize() { return _size; }
    
    /** Get parent LaunchDesc */
    public ResourcesDesc getParent() { return _parent; }
    
    /** Visitor dispatch */
    public void visit(ResourceVisitor rv) {
        rv.visitJARDesc(this);
    }
    
    public synchronized JARUpdater getUpdater() {
	if (_updater == null) {
	    _updater = new JARUpdater(this);
	}
	return _updater;
    }

    /** Return contents as an XML document */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("href", _location);
        ab.add("version", _version);
        ab.add("part", _part);
        ab.add("download", (isProgressJar()) ? "progress" :
              ((isLazyDownload() ? "lazy" : "eager")));
        ab.add("main", (isMainJarFile() ? "true" : "false"));
        String type = (_isNativeLib ? "nativelib" : "jar");
        return new XMLNode(type, ab.getAttributeList());
    }
    
    public String toString() { return "JARDesc[" + _location + ":" + _version +"]"; }

    public boolean equals(Object obj) {
	if ( !(obj instanceof JARDesc)) {
	    return false;
	}

	if (this == obj) return true;

	JARDesc jd = (JARDesc) obj;
	if (this.getVersion() != null) {
	    return (this.getVersion().equals(jd.getVersion()) &&
		    this.getLocation().toString().equals(jd.getLocation().toString()));
	} else {
	    return (jd.getVersion() == null &&
		    this.getLocation().toString().equals(jd.getLocation().toString()));
	} 
    }

    public int hashCode() {
	int hashcode = 0;
	if (getVersion() != null) {
	    hashcode = getVersion().hashCode();
	}

	if (getLocation() != null) {
	    hashcode ^= getLocation().toString().hashCode();
	}
	
	return hashcode;
    } 

    private static final boolean isConfigValid;

    static {
        isConfigValid = Config.isConfigValid(); // trigger Config being init
    }
}

