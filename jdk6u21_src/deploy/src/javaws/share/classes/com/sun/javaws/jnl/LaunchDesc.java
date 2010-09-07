/*
 * @(#)LaunchDesc.java	1.53 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;

import java.net.URL;
import java.net.MalformedURLException;
import com.sun.deploy.xml.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.util.TraceLevel;
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
    private UpdateDesc _update;
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
    private JREInfo _selectedJRE = null;
    private JREInfo _homeJRE = null;
    private LaunchSelection.MatchJREIf _matchImpl = null;
    private boolean _signed = false;
    private LDUpdater _updater = null;

    /* in some cases URL used to download this JNLP file
       is different from one stored inside.
       This is not illegal per se as you may want to launch
       application suing copy of JNLP file stored locally.
       However, there are situations when we need to access
       source cache entry for this JNLP file and this is only possible
       if we know original URL. */
    private URL _originalURL = null; 
    
    static public final int SANDBOX_SECURITY = 0;
    static public final int ALLPERMISSIONS_SECURITY = 1;
    static public final int J2EE_APP_CLIENT_SECURITY = 2;
    
    public LaunchDesc(String specVersion, URL codebase, URL home, 
                    String version, InformationDesc information, int securiyModel,
                    UpdateDesc update, ResourcesDesc resources, int launchType,
                    ApplicationDesc applicationDesc, AppletDesc appletDesc, 
                    LibraryDesc libraryDesc, InstallerDesc installerDesc, 
                    String internalCommand, String source, byte[] bits,
                    LaunchSelection.MatchJREIf matchImpl) {

        _specVersion = specVersion;
        _version = version;
        _codebase = codebase;
        _home = home;
        _information = information;
        _securiyModel = securiyModel;
        _update = update;
        _resources= resources;
        _launchType = launchType;
        _applicationDesc = applicationDesc;
        _appletDesc = appletDesc;
        _libraryDesc = libraryDesc;
        _installerDesc = installerDesc;
        _internalCommand = internalCommand;
        _source = source;
        _bits = bits;
        _matchImpl = matchImpl;
        _signed=false;
        if (_resources != null) {
            _resources.setParent(this); // Setup backpointer
            if (isApplication() || isApplet() || isInstaller()) {
                JARDesc jd = _resources.getMainJar(true);
                if (jd != null) {
                    jd.setLazyDownload(false);
                }
            }
        }
    }

    //returns URL used to load this entry
    //can be null
    public URL getSourceURL() {
        return _originalURL;
    }

    public void setSourceURL(URL u) {
        _originalURL = u;
    }

    public JREInfo getHomeJRE() {
        if (_homeJRE == null) {
            _homeJRE = JREInfo.getHomeJRE();
        }
        return _homeJRE;
    }

    public LaunchSelection.MatchJREIf getJREMatcher() {
        if(!_matchImpl.hasBeenRun()) {
            selectJRE();
        }
        return _matchImpl;
    }

    /**
     * re-select the JRE
     */
    public JREInfo selectJRE() {
        _selectedJRE = LaunchSelection.selectJRE(this, _matchImpl);
        return _selectedJRE; 
    }

    /**
     * re-select the JRE with the given matchImpl
     */
    public JREInfo selectJRE(LaunchSelection.MatchJREIf matchImpl) { 
        _matchImpl = matchImpl;
        return selectJRE();
    }

    /*
     *  LaunchDescriptor header
     */
    public String getSpecVersion() { return _specVersion; }
    public synchronized URL getCodebase() { return _codebase; }
    public byte[] getBytes() { return _bits; }
    
    /*
     * This returns the location of the JNLP as specified in the href 
     */
    public synchronized URL getLocation() {
        return _home;
    }
    
    /** This creates a default home if none is given based on the MAIN jar file
     *  location plus the string "jnlp"
     */
    public synchronized URL getCanonicalHome() {
        if (_home == null && _resources != null) {
            JARDesc jd = _resources.getMainJar(true);
            URL canonicalHome = null;
            try {
                if (jd != null) {
                    URL u = com.sun.deploy.net.HttpUtils.removeQueryStringFromURL(jd.getLocation());
                    canonicalHome = new URL(u.toString() + "jnlp");
                } else {
                    // if no jars - may be only extensions
                    ExtensionDesc[] ed = _resources.getExtensionDescs();
                    if (ed.length > 0) {
                        URL u = com.sun.deploy.net.HttpUtils.removeQueryStringFromURL(
                                ed[0].getLocation());
                        canonicalHome = new URL(u.toString() + ".jarjnlp");
                    }
                }
            } catch (MalformedURLException mue) {
                Trace.ignoredException(mue);
            }
            return (canonicalHome != null) ? canonicalHome : null;
        }
        return _home;
    }
    
    /** This creates a default home if none is given based on the MAIN jar file
     *  location
     */
    public synchronized String getSplashCanonicalHome() {
        if (_home == null && _resources != null) {
            JARDesc jd = _resources.getMainJar(true);
            return (jd != null) ? jd.getLocation().toString() + "jnlp" : null;
        }
        return _home.toString();
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
    public int getSecurityModel() { 
        return _securiyModel; 
    }

    /*
     *  Update information.
     */
    public UpdateDesc getUpdate() { return _update; }
    
    /*
     *  Is this LaunchDesc signed correctly
     */
    public boolean isSigned() { return _signed; }

    public boolean hasIdenticalContent(byte[] content) {
        byte a[] = getBytes();
        if (a == null || content == null || a.length != content.length) {
            return false;
        }

        for (int i = 0; i < a.length; i++) {
            if (a[i] != content[i]) {
                return false;
            }
        }

        return true;
    }

    private boolean _trusted = false;
    public void setTrusted() {
        Trace.println("Mark trusted: "+_home, TraceLevel.SECURITY);
        _trusted = true;
    }

    public boolean isTrusted() {
        Trace.println("Istrusted: "+_home+" "+_trusted, TraceLevel.SECURITY);
        return _trusted;
    }

    /*
     *  Is this LaunchDesc secure, i.e. does _not_ need signing
     */
    public boolean isSecure() { 
        return LaunchDesc.SANDBOX_SECURITY == getSecurityModel(); 
    }
    
    /*
     *  Are the JVM Arguments secure, 
     *  i.e. successfull verification of all JARs decides whether insecure
     *  arguments will be used for relaunch.
     */
    public boolean isSecureJVMArgs() { 
        return getJREMatcher().getSelectedJVMParameters().isSecure();
    }
    
    /*
     *  resources information
     */
    public ResourcesDesc getResources()         { return _resources; }
    public boolean arePropsSet()                { return _propsSet; }
    public void setPropsSet(boolean propsSet)   { _propsSet = propsSet; }

    public JREInfo getSelectedJRE()             {
        if (_selectedJRE == null) {
            selectJRE();
        }
        return _selectedJRE;
    }

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

    public boolean isHttps() { 
        if (_codebase != null) {
            return _codebase.getProtocol().equals("https"); 
        }
        getCanonicalHome();
        if (_home != null) {
            return _home.getProtocol().equals("https"); 
        }
        return false;
    }
    
    /** Returns the source file stripped for any encoding, such as UTF-8 */
    public String getSource() { return _source; }
    
    /** Check if the two launchDesc's are compatibile from a signing perseptive */
    public void checkSigning(LaunchDesc signedLd) throws JNLPSigningException {
        if (!signedLd.getSource().equals(getSource())) {
            throw new JNLPSigningException(this, signedLd.getSource());
        }
        _signed = true;
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

    public AppInfo getAppInfo() {
        AppInfo ainfo = new AppInfo(getLaunchType(), _information.getTitle(), 
            _information.getVendor(), getCanonicalHome(), 
            null, null, false, false, null, null);
        IconDesc icon = _information.getIconLocation(
            AppInfo.ICON_SIZE, IconDesc.ICON_KIND_DEFAULT);
        if (icon != null) {
            ainfo.setIconRef(icon.getLocation());
            ainfo.setIconVersion(icon.getVersion());
        }
        return ainfo;
    }

    public synchronized LDUpdater getUpdater() {
	if (_updater == null) {
	    _updater = new LDUpdater(this);
	}
	return _updater;
    }

    public String getProgressClassName() {
        // Check LaunchDesc for mainclass
        String result;
        if (_applicationDesc != null) {
            result = _applicationDesc.getProgressClass();
        } else if (_appletDesc != null) {
            result = _appletDesc.getProgressClass();
        } else if (_libraryDesc != null) {
            result = _libraryDesc.getProgressClass();
        } else {
            result = null;
        }
        if (result != null) {
            return result;
        }

        final String results[] = new String[1];
        if (getResources() != null) {
            getResources().visit(new ResourceVisitor() {
                public void visitJARDesc(JARDesc jad) { /* ignore */ }
                public void visitPropertyDesc(PropertyDesc prd) { /* ignore */ }
                public void visitPackageDesc(PackageDesc pad) { /* ignore */ }
                public void visitExtensionDesc(ExtensionDesc ed) { 
                    LaunchDesc ld = ed.getExtensionDesc();
                    if (ld != null && results[0] == null) {
                        results[0] = ld.getProgressClassName();
                    }
                }
                public void visitJREDesc(JREDesc jrd) { /* ignore */ }
            });
        }
        return results[0];
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
        nb.add(_update);
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

    public String getVersion() {
        return _version;
    }
}



