/*
 * @(#)ResourcesDesc.java	1.33 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import java.net.URL;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.HashSet;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.deploy.xml.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.JREInfo;

/**
 *  This class contains information about the codebase and properties,
 *  i.e., how to locate the classes and optional-pacakages
 */
public class ResourcesDesc implements ResourceType {
    private ArrayList _list = null;
    private LaunchDesc _parent = null;
    
    /** Create empty resource list */
    public ResourcesDesc() {
        _list = new ArrayList();
    }
    
    public LaunchDesc getParent() { return _parent; }
    
    /* Make sure to set parent of nested elements too. This should be
     * solved in a nicer more OO way - but since this is pretty late
     * in the game, we just do it like this
     */
    void setParent(LaunchDesc parent) {
        _parent = parent;
        for(int i = 0; i < _list.size(); i++) {
            Object o = _list.get(i);
            if (o instanceof JREDesc) {
                JREDesc jredesc = (JREDesc)o;
                if (jredesc.getNestedResources() != null) {
                    jredesc.getNestedResources().setParent(parent);
                }
            }
        }
    }
    
    void addResource(ResourceType rd) {
        if (rd != null) {
            _list.add(rd);
        }
    }
    
    boolean isEmpty() { return _list.isEmpty(); }
    
    public JREDesc getSelectedJRE() {
        for(int i = 0; i < _list.size(); i++) {
            Object o = _list.get(i);
            if (o instanceof JREDesc && ((JREDesc)o).isSelected()) {
                return (JREDesc)o;
            }
        }
        return null;
    }
    
    public JARDesc[] getLocalJarDescs() {
	ArrayList jds= new ArrayList(_list.size());
	for(int i = 0; i < _list.size(); i++) {
	    Object o = _list.get(i);
	    if (o instanceof JARDesc) jds.add(o);
	}
	return toJARDescArray(jds);
    }

    // Fix for 4528392
    // make sure we include extension JNLP files when we check for updates
    public ExtensionDesc[] getExtensionDescs() {
	final ArrayList l = new ArrayList();
	ExtensionDesc[] ed_array = new ExtensionDesc[0];
	visit(new ResourceVisitor() {
	    public void visitJARDesc(JARDesc jd) { /* ignore */}	       
	    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
	    public void visitExtensionDesc(ExtensionDesc ed) { l.add(ed); }
	    public void visitJREDesc(JREDesc jd) { /* ignore */ }
	    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
	});
	return (ExtensionDesc[])l.toArray(ed_array);
    }
    /**
     *  Returns a list of all either all the eager or all resources
     *  This method goes recusivly through all elements, including
     *  JREs and extensions. This method maintains the ordering of
     *  resources as specified in the JNLP file.
     */
    public JARDesc[] getEagerOrAllJarDescs(final boolean allResources)  {
        final HashSet eagerParts = new HashSet();
        
        // First find all packages with eager elements in the resource desc that
        // must be downloaded
        if (!allResources) {
            visit(new ResourceVisitor() {
                        public void visitJARDesc(JARDesc jd) {
                            if (!jd.isLazyDownload() && jd.getPartName() != null) {
                                eagerParts.add(jd.getPartName());
                            }
                        }
                        public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                        public void visitExtensionDesc(ExtensionDesc ed) { /* ignore */ }
                        public void visitJREDesc(JREDesc jd) { /* ignore */ }
                        public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                    });
        }
        
        
        ArrayList l = new ArrayList();
        addJarsToList(l, eagerParts, allResources, true);
        return toJARDescArray(l);
    }
    
    /**
     *  Add to a list of either all the eager or all resources
     *  This method goes recusivly through all elements, including
     *  JREs and extensions. This method maintains the ordering of
     *  resources as specified in the JNLP file.
     */
    private void addJarsToList(final ArrayList list, final HashSet includeParts,
                               final boolean includeAll, final boolean includeEager) {
        
        // Iterate through list an add resources to the list.
        // The ordering of resources are preserved
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) {		
                        if (includeAll || (includeEager && !jd.isLazyDownload()) ||
                            includeParts.contains(jd.getPartName())) {
                            list.add(jd);
                        }
                    }
                    
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    
                    public void visitExtensionDesc(ExtensionDesc ed) {
                        // Get parts of extension
                        HashSet extensionParts = ed.getExtensionPackages(includeParts, includeEager);
			// Fix for 4528392
			// make sure we include extension installer jar
			// files when we check for updates
			// Fix for 4651841
			// this is caused by fix of 4528392.  we should not
			// setExtensionDesc if it was already set.  this
			// cause compareTo in JREDesc to fail since
			// it will then be comparing different instance
			// of the same object.  (we did not implement
			// our own equals method in JARDesc)
			if (ed.getExtensionDesc() == null) {
                            String knownPlatforms = JREInfo.getKnownPlatforms();
			    DiskCacheEntry dce = null;
			    try {
				dce = DownloadProtocol.getCachedExtension(
			 				 ed.getLocation(),
						   	 ed.getVersion(),
							 knownPlatforms);
				
				if (dce != null && dce.getFile() != null) {
				    LaunchDesc extensionLd = LaunchDescFactory.buildDescriptor(dce.getFile());			
				    ed.setExtensionDesc(extensionLd);
				}			  
			    } catch (Exception e) {
				Trace.ignoredException(e);
			    }
			}

                        if (ed.getExtensionDesc() != null) {
                            ResourcesDesc rd = ed.getExtensionDesc().getResources();			 
                            if (rd != null) {
                                rd.addJarsToList(list,
                                                 extensionParts,
                                                 includeAll,
                                                 includeEager);
                            }
                        }
                    }
                    
                    /* Get JRE resources if needed */
                    public void visitJREDesc(JREDesc jd) {
                        if (jd.isSelected()) {
                            // Include nested resources in selected JRE
                            ResourcesDesc rd = jd.getNestedResources();
                            if (rd != null) {
                                rd.addJarsToList(list,
                                                 includeParts,
                                                 includeAll,
                                                 includeEager);
                            }
                            
                            // If installer is being downloaded, include that too
                            if (jd.getExtensionDesc() != null) {
                                ResourcesDesc erd = jd.getExtensionDesc().getResources();
                                if (erd != null) {
                                    erd.addJarsToList(list,
                                                      new HashSet(),
                                                      includeAll,
                                                      includeEager);
                                }
                            }
                        }
                    }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
    }
    
    /** Return all resources needed by a specific set of parts */
    public JARDesc[] getPartJars(String[] parts) {
        HashSet partsSet = new HashSet();
        for(int i = 0; i < parts.length; i++) partsSet.add(parts[i]);
        ArrayList l = new ArrayList();
        addJarsToList(l, partsSet, false, false);
        return toJARDescArray(l);
    }
    
    /** Return all resources needed by a specific part */
    public JARDesc[] getPartJars(String part) {
        return getPartJars(new String[] { part });
    }
    
    /** Get all the resources needed when a specific resource is requested. Returns null
     *  if no resouce was found
     */
    public JARDesc[] getResource(final URL location, final String version) {
        final VersionString vs = (version != null) ? new VersionString(version) : null;
        final JARDesc[] resources = new JARDesc[1];
        // Find the given resource
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) {
                        if (URLUtil.equals(jd.getLocation(), location)) {
                            if (vs == null) {
                                resources[0] = jd;
                            } else if (vs.contains(jd.getVersion())) {
                                resources[0] = jd;
                            }
                        }
                    }
                    
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed) { /* ignore */ }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
        
        // Found no resource?
        if (resources[0] == null) return null;
        // Resource has a part?
        if (resources[0].getPartName() != null) {
            return getPartJars(resources[0].getPartName());
        }
        // No part, so just one resource
        return resources;
    }
    
    /** Get all resources needed by a particular part of an extension */
    public JARDesc[] getExtensionPart(final URL location, final String version, String[] parts) {
        ExtensionDesc ed = findExtension(location, version);
        if (ed == null) return null;
        ResourcesDesc rd = ed.getExtensionResources();
        if (rd == null) return null;
        return rd.getPartJars(parts);
    }
            
    private ExtensionDesc findExtension(final URL location, final String version) {
        final ExtensionDesc[] ea = new ExtensionDesc[1];
        
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) { /* ignore */ }
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed) {
                        // Check if we have already found a match
                        if (ea[0] == null) {
                            if (URLUtil.equals(ed.getLocation(), location) &&
                                    (version == null || new VersionString(version).contains(ed.getVersion()))) {
                                ea[0] = ed;
                            } else {
                                // Search recursivley
                                LaunchDesc ld = ed.getExtensionDesc();
                                if (ld != null && ld.getResources() != null) {
                                    ea[0] = ld.getResources().findExtension(location, version);
                                }
                            }
                        }
                    }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
        return ea[0];
    }
    
    /** Returns the main JAR. This is a non-recursive check. If forceMain, this
     *  will return the first JAR file, if none is specified. */
    public JARDesc getMainJar(boolean forceMain) {
        // Normal trick to get around final arguments to inner classes
        final JARDesc[] results = new JARDesc[2];
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) {
                        if (jd.isJavaFile()) {
                            // Keep track of first Java File
                            if (results[0] == null ) results[0] = jd;
                            // Keep tack of Java File marked main
                            if (jd.isMainJarFile()) results[1] = jd;
                        }
                    }
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed) { /* ignore */ }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
        // Default is the first, if none is specified as main. This might
        // return NULL if there is no JAR resources.
        JARDesc first = results[0];
        JARDesc main  = results[1];
        return (main != null && forceMain) ? main : first;
    }
    
    /** Return all the resources needed for a given part (A part
     *  is non-recursive)
     */
    public JARDesc[] getPart(final String name) {
        final ArrayList l = new ArrayList();
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) {
                        if (name.equals(jd.getPartName())) {
                            l.add(jd);
                        }
                    }
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed) { /* ignore */ }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
        return toJARDescArray(l);
    }
    
    /** Return all the resources needed for a given part of a given
     *  extension
     */
    public JARDesc[] getExtensionPart(final URL url, final String version, final String part) {
        final JARDesc[][] jdss = new JARDesc[1][];
        visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jd) { /* ignore */ }
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed) {
                        if (URLUtil.equals(ed.getLocation(), url)) {
                            if (version == null) {
                                if (ed.getVersion() == null && ed.getExtensionResources() != null) {
                                    jdss[0] = ed.getExtensionResources().getPart(part);
                                }
                            } else if (version.equals(ed.getVersion())) {
                                if (ed.getExtensionResources() != null) {
                                    jdss[0] = ed.getExtensionResources().getPart(part);
                                }
                            }
                        }
                    }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
                });
        return jdss[0];
    }
    
    // Helper method to convert to JavaArrays
    private JARDesc[] toJARDescArray(ArrayList array) {
        JARDesc[] jda = new JARDesc[array.size()];
        return (JARDesc[])array.toArray(jda);
    }
    
    
    /*
     *  Get the properties defined for this object
     */
    public Properties getResourceProperties() {
        final Properties props = new Properties();
        visit(new ResourceVisitor() {
                public void visitPropertyDesc(PropertyDesc pd) {
                    props.setProperty(pd.getKey(), pd.getValue());
                }
                // Ignore everything else
                public void visitJARDesc(JARDesc jd)             { }
                public void visitExtensionDesc(ExtensionDesc ed) { }
                public void visitJREDesc(JREDesc jd)             { }
                public void visitPackageDesc(PackageDesc jd)     { }
            });
        return props;
    }
    
    /** Value object to return the part and LaunchDesc that contains information about
     *  a certain resource.
     */
    public static class PackageInformation {
        private LaunchDesc _launchDesc;
        private String     _part;
        
        PackageInformation(LaunchDesc ld, String part) {
            _launchDesc = ld;
            _part = part;
        }
        
        public LaunchDesc getLaunchDesc() 	{ return _launchDesc; }
        public String     getPart() 		{ return _part; }
    }
    
    /** Checks if the given resource is specified in a package element, and returns
     *  the LaunchDesc and part name for it
     */
    public PackageInformation getPackageInformation(String name) {
        // Convert slashes to does, as used in the JNLP specification
        name = name.replace('/', '.');
        if (name.endsWith(".class")) name = name.substring(0, name.length() - 6);
        
        return visitPackageElements(this.getParent(), name);
    }
    
    static private PackageInformation visitPackageElements(final LaunchDesc ld, final String name) {
        final PackageInformation[] result = new PackageInformation[1];
        
        ld.getResources().visit(new ResourceVisitor() {
                    // No Properties in JARDesc's
                    public void visitJARDesc(JARDesc jd) { /* ignore */ }
                    public void visitPropertyDesc(PropertyDesc pd) { /* ignore */ }
                    public void visitJREDesc(JREDesc jd) { /* ignore */ }
                    
                    public void visitExtensionDesc(ExtensionDesc ed) {
			if (!ed.isInstaller()) {
			    LaunchDesc extLd = ed.getExtensionDesc();
			    if (result[0] == null && extLd.isLibrary() && extLd.getResources() != null) {
				result[0] = visitPackageElements(extLd, name);
			    }
			}
		    }
                    
                    public void visitPackageDesc(PackageDesc jd) {
                        if (result[0] == null && jd.match(name)) {
                            result[0] = new PackageInformation(ld, jd.getPart());
                        }
                    }
                });
        return result[0];
    }
    
    /** visitor dispatch */
    public void visit(ResourceVisitor rv) {
        for(int i = 0; i < _list.size(); i++) {
            ResourceType rt = (ResourceType)_list.get(i);
            rt.visit(rv);
        }
    }
    
    /** Output XML node structure for the content */
    public XMLNode asXML() {
        XMLNodeBuilder nb = new XMLNodeBuilder("resources", null);
        for(int i = 0; i < _list.size(); i++) {
            ResourceType rt = (ResourceType)_list.get(i);
            nb.add(rt);
        }
        return nb.getNode();
    }

    public void addNested(ResourcesDesc nested) { 
        if (nested != null) nested.visit(new ResourceVisitor() {
            public void visitJARDesc(JARDesc jd) { _list.add(jd); } 
            public void visitPropertyDesc(PropertyDesc pd) { _list.add(pd); }
            public void visitExtensionDesc(ExtensionDesc ed) { _list.add(ed); }             public void visitJREDesc(JREDesc jd) { /* ignore */ } 
            public void visitPackageDesc(PackageDesc jd) { /* ignore */ }
        });

    }

}


