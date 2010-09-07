/*
 * @(#)LaunchSelection.java	1.51 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.util.ArrayList;
import java.io.File;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.VersionString;
import com.sun.deploy.util.JVMParameters;
import com.sun.deploy.net.DownloadEngine;
/*
 * Class containing single method to determine what JRE to use.
 *
 */
public class LaunchSelection {
    
   /**
    * Implements functionality to
    *  traverse through the whole JNLP graph.
    *    - start traversal 
    *    - digest the selected JREDesc and JREInfo while traversing
    *    - end traversal
    *
    * Results:
    *    - JVMParameters for the desired JVM arguments,
    *      which are accumulated from all JREDesc:
    *        - max heapsize wins
    *        - unique arguments
    *        - chronological order from inside to outside,
    *          the JNLP graph
    *
    *    - JREInfo for the desired and installed JRE version
    *        - the highes JRE version required/allowed
    *          in the JNLP graph
    *
    *    - isRunningJVMSatisfying() if the running JREInfo
    *      satisfy us, i.e. JRE version and jvm arguments
    */
    public interface MatchJREIf {

        // @return true if the matcher has been run,
        // i.e. traversed through at least one LaunchDesc.
        // If not, it's internal state and all values are 
        // undefined.
        public boolean hasBeenRun();

        // notify that we are about to start traversal,
        // usual you want to reset your internal state to null;
        // @param ld the root LaunchDesc
        public void beginTraversal(LaunchDesc ld);

        // Digest the selected JREDesc and installed JREInfo.
        // JREInfo may be null, if not installed.
        public void digest(JREDesc jd, JREInfo ji);

        // Digest the LaunchDesc
        // Will be called after visiting each LaunchDesc's resource
        public void digest(LaunchDesc ld);

        // notify that we are have finished the traversal,
        // usual you want to compute the final match,
        // i.e. JREInfo
        // @param ld the root LaunchDesc
        public void endTraversal(LaunchDesc ld);

        // @return the final selected JREInfo
        //         after traversion through the whole tree
        //         This shall be the highest match, or the compatible running JVM
        //         May be null, of no JREInfo matches
        public JREInfo getSelectedJREInfo();

        // @return the final selected JREDesc
        //         after traversion through the whole tree.
        //         This shall be the highest requested version.
        //         Shall not be null, if no JREDesc is specified within the JNLP file,
        //         any JREDesc (VersionID '0+') shall be used.
        public JREDesc getSelectedJREDesc();

        // @return  the final accumulated JVMParameters
        //          after traversion through the whole tree
        //          Shall not be null.
        public JVMParameters getSelectedJVMParameters();

        // @return  the final accumulated JVMParameters
        //          after traversion through the whole tree
        //          Shall not be null.
        public String getSelectedJVMParameterString();

        // @return  the final selected maximum initial Heap size
        //          wrongly refered to as minHeap
        //          Can be below default heap size
        public long getSelectedInitHeapSize();

        // @return  the final selected maximum maxHeap size
        //          Cannot be below default heap size
        public long getSelectedMaxHeapSize();

        // @return true if the current JVM satisfies 
        //         getSelectedJREInfo() and 
        //         getSelectedJVMParameters()
        public boolean isRunningJVMSatisfying(boolean includeInsecure);
	
	// @return true if the current JVM satisfies 
        //         getSelectedJREInfo()
	public boolean isRunningJVMVersionSatisfying();

	// @return true if the current JVM satisfies  
        //         getSelectedJVMParameters()
	public boolean isRunningJVMArgsSatisfying(boolean includeInsecure);

        public String toString();
    }

    public static MatchJREIf createDefaultMatchJRE() {
        return new DefaultMatchJRE();
    }

    /** Select the JRE to choose. The selection algorithm will try to
     *  match based on the order specified in the JNLP file. For example,
     *
     *    <j2se version="1.3 1.2"/>
     *    <j2se version="1.4"/>
     *
     * If a wildcard is used, e.g., 1.2+ or 1.2*, we will try to match on
     * the current running or highest version installed
     *
     * Will match on the platform versions 1.3 1.2 1.4 in that given order
     */
    protected static JREInfo selectJRE(final LaunchDesc ld, MatchJREIf matcher) {
        matcher.beginTraversal(ld);

        ResourcesDesc rdescs = ld.getResources();
        if(rdescs != null ) {
            // select JREDesc and potential installed JREInfo
            selectJREDescHelper(ld, matcher);

            // mark and nest the selected JREDesc
            JREDesc jd = matcher.getSelectedJREDesc();
            if(jd!=null) {
                jd.markAsSelected();
                rdescs.addNested(jd.getNestedResources());
            }

            // traverse through all extensions,
            // incl. the selected JREDesc contained ones
            selectJREExtensionHelper(ld, matcher);
        }

        matcher.endTraversal(ld);

        return matcher.getSelectedJREInfo();
    }

    private static final String  anyJREVersion = "0+"; // default version

    private static void selectJREDescHelper(final LaunchDesc ld, final MatchJREIf matcher) {
        ResourcesDesc rd = ld.getResources();
        if (rd == null) {
            return;
        }
        
        // Iterate through all JREDesc's
        final ArrayList listJREDesc = new ArrayList();
        
        rd.visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jad) { /* ignore */ }
                    public void visitPropertyDesc(PropertyDesc prd)  { /* ignore */ }
                    public void visitPackageDesc(PackageDesc pad)  { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed)  { /* ignore */ }
                    public void visitJREDesc(JREDesc jre) {
                        if(ld.isLibrary()) {
                            Trace.println("JNLP JREDesc in Component ignored: "+ld.getLocation());
                        } else {
                            listJREDesc.add(jre);
                        }
                    }
                });

        JREDesc jreDesc = null;
        JREInfo jreInfo = null;

        if(listJREDesc.size()>0) {
            // try to fetch the 1st JREDesc, for which we have a JREInfo installed
            for(int i = 0; jreInfo==null && i < listJREDesc.size(); i++) {
                jreDesc = (JREDesc)listJREDesc.get(i);
                URL location = jreDesc.getHref();
                jreInfo = selectJRE(location, jreDesc.getVersion());
            }

            if(jreInfo==null) {
                // no JREInfo installed for any JREDesc ..
                // fetch the first entry ..
                jreDesc = (JREDesc)listJREDesc.get(0);
            }
        } else {
            // no JREDesc given at all ..
            // use a generic any version
            jreDesc = new JREDesc(anyJREVersion, -1, -1, null, 
                                  null, new ResourcesDesc());
            rd.addResource(jreDesc);
            jreInfo = LaunchSelection.selectJRE(jreDesc.getHref(), jreDesc.getVersion());
        }
        matcher.digest(jreDesc, jreInfo);
    }

    private static void selectJREExtensionHelper(final LaunchDesc ld, final MatchJREIf matcher) {
        ResourcesDesc rd = ld.getResources();
        if (rd == null) {
            return;
        }
        
        // Iterate through all extensions
        final ArrayList listExtDesc = new ArrayList();
        
        rd.visit(new ResourceVisitor() {
                    public void visitJARDesc(JARDesc jad) { /* ignore */ }
                    public void visitPropertyDesc(PropertyDesc prd)  { /* ignore */ }
                    public void visitPackageDesc(PackageDesc pad)  { /* ignore */ }
                    public void visitExtensionDesc(ExtensionDesc ed)  { 
                        listExtDesc.add(ed);
                    }
                    public void visitJREDesc(JREDesc jre) { /* ignore */ }
                });

        matcher.digest(ld);

        if(ld.isInstaller()) {
            return; // no deep recursion for installers
        }

        for(int i = 0; i < listExtDesc.size(); i++) {
            ExtensionDesc ed = (ExtensionDesc)listExtDesc.get(i);
            
            LaunchDesc extensionLd = ed.getExtensionDesc();

            //if we have descriptor already then do not try to rebuild it
            //from the same xml file (download is not allowed here anyway)
            if (extensionLd == null) {
                try {
                    File cachedFile = DownloadEngine.getCachedFile(
                            ed.getLocation(), ed.getVersion(), false /* no download */,
                            false, null /* no platform required if just using cache */);

                    if (null != cachedFile) {
                        extensionLd = LaunchDescFactory.buildDescriptor(
                            cachedFile, ed.getCodebase(), ed.getLocation(), ed.getLocation());
                    }
                } catch (Exception e) {
                    Trace.ignoredException(e); // that's ok for the matching algo
                }
            }

            if(null==extensionLd) {
                continue; // next 
            }

            if ( extensionLd.isInstaller() ) 
            {
                // mark sub-extension installer but don't consider it for matching
                ed.setInstaller(true);
            } else {
                ed.setExtensionDesc(extensionLd);
                // Do recursion.
                selectJREExtensionHelper(extensionLd, matcher);
            }
        }
    }


    // it is essential, that JREInfo maintains the list of installed JREInfo's
    // in order: highest product version first and declining.
    public static JREInfo selectJRE(URL location, String version) {
        JREInfo[] jres = JREInfo.getAll();
        if (jres == null) return null;

        VersionString vs = new VersionString(version);
        for(int i = 0; i < jres.length; i++) {
 
            // first check if JRE osName and osArch matches
            // (only if osName and osArch exist)
            if (jres[i].isOsInfoMatch(Config.getOSName(), Config.getOSArch())) {
         
                if (jres[i].isEnabled()) {
 
                    if ((location == null) ?
                        isPlatformMatch(jres[i], vs) :
                        isProductMatch(jres[i], location, vs)) {
                        return jres[i];
                    }
                }
            }
        }
        return null;
    }
    
    public static boolean isPlatformMatch(JREInfo jre, VersionString vs) {
        boolean isFCS;
        // A platform match is only allowed for FCS versions of JREs.
        // By convention, if the product-id contains a '-' then
        // it is non-FCS version. This convention is used for 1.3.0 and
        // higher

        // new convension to consider j4b versioning, -er and -rev are FCS
        String product = jre.getProduct();
        if ((product == null) || (isInstallJRE(jre)) || 
            (product.indexOf('-') == -1) || 
            (product.indexOf("-rev") != -1) ||
            (product.indexOf("-er") != -1)) {
            isFCS = true;
        } else {
           isFCS = false;
        }

        if ((new File(jre.getPath())).exists()) {
            return (vs.contains(jre.getPlatform()) && isFCS);
        }
        return false;
    }

    public static boolean isProductMatch(JREInfo jre, URL location,
                                                  VersionString vs) {
        if ((new File(jre.getPath())).exists()) {
            return (jre.getLocation().equals(location.toString()) &&
                vs.contains(jre.getProduct()));
        }
        return false;
    }

    public static boolean isInstallJRE(JREInfo jre) {
        File installDir = new File(Config.getJavaHome());
        File jrePath = new File(jre.getPath());
        File bindir = jrePath.getParentFile();
        return installDir.equals(bindir.getParentFile());
    }
}




