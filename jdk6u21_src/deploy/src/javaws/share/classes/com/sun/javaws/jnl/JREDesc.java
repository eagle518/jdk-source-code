/*
 * @(#)JREDesc.java	1.23 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;

import java.net.URL;

/**
 *  Interface that describes the JRE requirements
 *  for an application
 */
public class JREDesc implements ResourceType {
    private String _version;
    private long   _maxHeap;
    private long   _minHeap;
    private String _vmargs;
    private URL    _href;
    private boolean _isSelected;
    private ResourcesDesc _resourceDesc;
    
    // Link to ExtensionDesc for this JRE. This is used if the JRE is being downloaded
    private LaunchDesc _extensioDesc;
    
    public JREDesc(String version, long minHeap, long maxHeap, String vmargs,
		   URL href, ResourcesDesc resourcesDesc) {
        _version = version;
        _maxHeap = maxHeap;
        _minHeap = minHeap;
	    _vmargs = vmargs;
        _href = href;
        _isSelected = false;
        _resourceDesc = resourcesDesc;
        _extensioDesc = null;
    }

    public String toString()
    {
        return "JREDesc[version "+_version+", heap="+_minHeap+"-"+_maxHeap+", args="+_vmargs+", href="+_href+", sel="+_isSelected+", "+
        _resourceDesc+", "+_extensioDesc+"]";
    }
        
    /*
     *  JRE specification
     */
    public String getVersion() { return _version; }
    public URL getHref() { return _href; }
    
    /*
     *  JVM arguments. Returns NULL or -1 if not-specified
     */
    public long getMinHeap() { return _minHeap; }
    public long getMaxHeap() { return _maxHeap; }
    public String getVmArgs() { return _vmargs; }
    
    /*
     * Is selected or not
     */
    public boolean isSelected() { return _isSelected; };
    public void markAsSelected() { _isSelected = true; }
    
    /** Get nested resources */
    public ResourcesDesc getNestedResources() { return _resourceDesc; }
    
    /*
     *  Get/Set ExtensionDesc for this JRE
     */
    public LaunchDesc getExtensionDesc() { return _extensioDesc; }
    public void setExtensionDesc(LaunchDesc ld) { _extensioDesc = ld; }
    
    /* visitor dispatch */
    public void visit(ResourceVisitor rv) {
        rv.visitJREDesc(this);
    }

    public String getSource() {
	if (_href != null) {
	    return _href.getHost();
	}
	try {
	    URL url = new URL(Config.getProperty(
		Config.JAVAWS_JRE_INSTALL_KEY));
	    return url.getHost();
	} catch (Throwable t) {
	    Trace.ignored(t);
	    return "";
	}
    }
    
    /** Outputs XML structure for contents */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        if (_minHeap > 0) {
            ab.add("initial-heap-size", _minHeap);
        }
        if (_maxHeap > 0) {
            ab.add("max-heap-size", _maxHeap);
        }
	if (_vmargs != null) {
	    ab.add("java-vm-args", _vmargs);
	}
        ab.add("href", _href);
	if (_version != null) {
            ab.add("version", _version);
	}
        XMLNode extcNode = (_extensioDesc != null) ? _extensioDesc.asXML() : null;
	if (_resourceDesc != null) {
	    extcNode = _resourceDesc.asXML();
	}
        return new XMLNode("java", ab.getAttributeList(), extcNode, null);
    }
}

