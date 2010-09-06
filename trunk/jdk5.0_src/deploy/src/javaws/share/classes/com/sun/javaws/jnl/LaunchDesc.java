/*
 * @(#)LaunchDesc.java	1.29 04/01/21
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;

import java.util.Properties;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.util.Locale;
import java.net.URL;
import com.sun.deploy.xml.*;
import com.sun.javaws.exceptions.JNLPSigningException;

/*
 * Highlevel view on a JNLP file. This class is used to
 * separate the concrete syntax of the JNLP file from the
 * implementation of the launcher. Thus, several different
 * versions of the syntax could be potential supported at
 * the same time.
 *
 * Optain a concrete instance of this object using the
 * LaunchDescriptor factory class
 */
public class LaunchDesc implements XMLable {
    private String _specVersion;     // Version of JNLP specification
    private String _version;         // Version of JNLP file
    private URL  _home;              // Home of JNLP file
    private URL  _codebase;          // Codebase for all relative URLs
    private InformationDesc _information;
    private int _securiyModel;
    private ResourcesDesc _resources;
    private int _launchType;
    private ApplicationDesc _applicationDesc;
    private AppletDesc _appletDesc;
    private LibraryDesc _libraryDesc;
    private InstallerDesc _installerDesc;
    private String _internalCommand;
    private String _source;
    private boolean _propsSet = false;
    private byte[] _bits;
    
    static public final int SANDBOX_SECURITY = 0;
    static public final int ALLPERMISSIONS_SECURITY = 1;
    static public final int J2EE_APP_CLIENT_SECURITY = 2;
    
    public LaunchDesc(String specVersion, URL codebase, URL home, String version,
		      InformationDesc information, int securiyModel,
		      ResourcesDesc resources, int launchType,
		      ApplicationDesc applicationDesc, AppletDesc appletDesc, LibraryDesc libraryDesc,
		      InstallerDesc installerDesc, String internalCommand, String source, byte[] bits) {
        _specVersion = specVersion;
        _version = version;
        _codebase = codebase;
        _home = home;
        _information = information;
        _securiyModel = securiyModel;
        _resources= resources;
        _launchType = launchType;
        _applicationDesc = applicationDesc;
        _appletDesc = appletDesc;
        _libraryDesc = libraryDesc;
        _installerDesc = installerDesc;
        _internalCommand = internalCommand;
        _source = source;
	_bits = bits;
        if (_resources != null) _resources.setParent(this); // Setup backpointer
    }
    
    /*
     *  LaunchDescriptor header
     */
    public String getSpecVersion() { return _specVersion; }
    public synchronized URL getCodebase() { return _codebase; }
    public byte[] getBytes() { return _bits; }
    
    /** This returns the location of the JNLP as specified in the href attribute */
    public synchronized URL getLocation() {
        return _home;
    }
    
    /** This creates a default home if none is given based on the MAIN jar file
     *  location
     */
    public synchronized URL getCanonicalHome() {
        if (_home == null && _resources != null) {
	    JARDesc jd = _resources.getMainJar(true);
	    return (jd != null) ? jd.getLocation() : null;
        }
        return _home;
    }
    
    /*
     *  Informational properties
     */
    public InformationDesc getInformation() {
        return _information;
    }

    public String getInternalCommand() { 
        return _internalCommand;
    }    
    
    /*
     *  Security information.
     */
    public int getSecurityModel() { return _securiyModel; }
    
    /*
     *  resources information
     */
    public ResourcesDesc getResources() 	{ return _resources; }
    public boolean arePropsSet() 		{ return _propsSet; }
    public void setPropsSet(boolean propsSet) 	{ _propsSet = propsSet; }

    
    /*
     *  Application specific information
     */
    public final static int APPLICATION_DESC_TYPE = 1;
    public final static int APPLET_DESC_TYPE      = 2;
    public final static int LIBRARY_DESC_TYPE     = 3;
    public final static int INSTALLER_DESC_TYPE   = 4;
    public final static int INTERNAL_TYPE         = 5;
    
    public int              getLaunchType()             { return _launchType; }
    public ApplicationDesc  getApplicationDescriptor()  { return _applicationDesc; }
    public AppletDesc       getAppletDescriptor()       { return _appletDesc; }
    public InstallerDesc    getInstallerDescriptor()    { return _installerDesc; }
    
    public boolean isApplication() { return _launchType == APPLICATION_DESC_TYPE; }
    public boolean isApplet() { return _launchType == APPLET_DESC_TYPE; }
    public boolean isLibrary() { return _launchType == LIBRARY_DESC_TYPE; }
    public boolean isInstaller() { return _launchType == INSTALLER_DESC_TYPE; }
    public boolean isApplicationDescriptor() { return isApplication() || isApplet(); }
    public boolean isHttps() { return _codebase.getProtocol().equals("https"); }
    
    /** Returns the source file stripped for any encoding, such as UTF-8 */
    public String getSource() { return _source; }
    
    /** Check if the two launchDesc's are compatibile from a signing perseptive */
    public void checkSigning(LaunchDesc signedLd) throws JNLPSigningException {
        if (!signedLd.getSource().equals(getSource())) {
	    throw new JNLPSigningException(this, signedLd.getSource());
        }
    }
    
    /** Check that a JRE has been specified */
    public boolean isJRESpecified() {
        final boolean hasJre[] = new boolean[1];
        final boolean needJre[] = new boolean[1];
        if (getResources() != null) {
	    getResources().visit(new ResourceVisitor() {
			public void visitJARDesc(JARDesc jad) { needJre[0] = true; }
			public void visitPropertyDesc(PropertyDesc prd) { /* ignore */ }
			public void visitPackageDesc(PackageDesc pad) { /* ignore */ }
			public void visitExtensionDesc(ExtensionDesc ed) { needJre[0] = true;  }
			public void visitJREDesc(JREDesc jrd) { hasJre[0] = true; }
		    });
        }
        if (_launchType == APPLICATION_DESC_TYPE || _launchType == APPLET_DESC_TYPE) {
	    needJre[0] = true;
        }
        return hasJre[0] || !needJre[0];
    }
    
    /** Returns the contents of the LaunchDesc as an XML graph */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("spec", _specVersion);
        ab.add("codebase", _codebase);
        ab.add("version", _version);
        ab.add("href", _home);
        
        XMLNodeBuilder nb = new XMLNodeBuilder("jnlp", ab.getAttributeList());
        nb.add(_information);
        
        if (_securiyModel == ALLPERMISSIONS_SECURITY) {
	    nb.add(new XMLNode("security", null, new XMLNode("all-permissions", null), null));
        } else if (_securiyModel == J2EE_APP_CLIENT_SECURITY) {
	    nb.add(new XMLNode("security", null, new XMLNode("j2ee-application-client-permissions", null), null));
        }
        nb.add(_resources);
        nb.add(_applicationDesc);
        nb.add(_appletDesc);
        nb.add(_libraryDesc);
        nb.add(_installerDesc);
        return nb.getNode();
    }
    
    public String toString() {
        return asXML().toString();
    }
}



