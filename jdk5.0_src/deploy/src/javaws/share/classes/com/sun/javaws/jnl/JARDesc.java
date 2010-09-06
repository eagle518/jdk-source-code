/*
 * %W% %E%
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.Globals;
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
    private boolean _isMainFile;  // Only used for Java JAR files (a main JAR file is implicitly eager)
    private String  _part;        // The name of the part this JAR file belongs to or null if no part specified
    private ResourcesDesc _parent;   // Back-pointer to the Resources that contains this JAR
    
    public JARDesc(URL location, String version, boolean isLazy,
		   boolean isMainFile, boolean isNativeLib, String part, int size, ResourcesDesc parent) {
        _location = location;
        _version = version;
        _isLazyDownload = (isLazy && 
	    (!_isMainFile) &&    	// The main file cannot be lazy
	    (!Globals.isImportMode()));	// nothing is lazy in import mode
        _isNativeLib = isNativeLib;
        _isMainFile = isMainFile;
        _part = part;
        _size = size;
        _parent = parent;
    }
    
    /** Type of JAR resource */
    public boolean isNativeLib() { return _isNativeLib; }
    public boolean isJavaFile()  { return !_isNativeLib; }
    
    /** Returns URL/version for JAR file */
    public URL getLocation()   { return _location; }
    public String getVersion() { return _version; }
    
    /** Returns if resource should be downloaded lazily */
    public boolean isLazyDownload() { return _isLazyDownload; }
    
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
    
    /** Return contents as an XML document */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("href", _location);
        ab.add("version", _version);
	ab.add("part", _part);
        ab.add("download", (isLazyDownload() ? "lazy" : "eager"));
        ab.add("main", (isMainJarFile() ? "true" : "false"));
	String type = (_isNativeLib ? "nativelib" : "jar");
        return new XMLNode("jar", ab.getAttributeList());
    }
    
    public String toString() { return "JARDesc[" + _location + ":" + _version; }
}

