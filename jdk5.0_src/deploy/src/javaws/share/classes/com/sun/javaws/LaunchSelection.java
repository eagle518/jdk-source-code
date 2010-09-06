/*
 * @(#)LaunchSelection.java	1.33 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import java.util.Locale;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.io.File;
import java.util.Properties;
import java.util.Enumeration;
import com.sun.javaws.jnl.*;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.VersionString;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
/*
 * Class containing single method to determine what JRE to use.
 *
 */
public class LaunchSelection {
    
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
    static JREInfo selectJRE(LaunchDesc ld) {
        final JREDesc selectedJREDesc[] = new JREDesc [1];
        final JREInfo selectedJRE[] = new JREInfo[1];
	
        // Iterate through all JREDesc's
        ResourcesDesc rdescs = ld.getResources();
        rdescs.visit(new ResourceVisitor() {
		    public void visitJARDesc(JARDesc jad) { /* ignore */ }
		    public void visitPropertyDesc(PropertyDesc prd)  { /* ignore */ }
		    public void visitPackageDesc(PackageDesc pad)  { /* ignore */ }
		    public void visitExtensionDesc(ExtensionDesc ed)  { /* ignore */ }
		    public void visitJREDesc(JREDesc jre) {
			if (selectedJRE[0] == null) {
			    handleJREDesc(jre, selectedJRE, selectedJREDesc);
			}
		    }
		});
        // Mark the selected JRE in launchDesc
        selectedJREDesc[0].markAsSelected();
	// add the Resources nested in the selected JRE
	rdescs.addNested(selectedJREDesc[0].getNestedResources());
        return selectedJRE[0];
    }


    public static JREInfo selectJRE(URL location, String version) {
	JREInfo[] jres = JREInfo.get();
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
    
    /* Split up version attribute and search for a match in order */
    static private void handleJREDesc(JREDesc jd,
				      JREInfo[] selectedJRE,
				      JREDesc[] selectedJREDesc) {
        URL location = jd.getHref();
        String version = jd.getVersion();
	
	/* Split version-string at spaces so we match on each one in order */
	StringTokenizer tokenStream = new StringTokenizer(version, " ", false);
	int count = tokenStream.countTokens();

        if (count > 0) {
	    String versionParts[] = new String[count];
	    for (int i=0; i<count; i++) {
	        versionParts[i] = tokenStream.nextToken();
	    }
	    matchJRE(jd, versionParts, selectedJRE, selectedJREDesc);
	    if (selectedJRE[0] != null) return;
	}
    }
    
    /* Find a match for a particular location and array of versions */
    /* note - use currently running jre if match, otherwise use first match */
    static private void matchJRE(JREDesc jd,
				 String [] versions,
				 JREInfo[] selectedJRE,
				 JREDesc[] selectedJREDesc) {
	URL location = jd.getHref();
        VersionString vs;  // Make sure to use passed in versions
	
	
	JREInfo[] jres = JREInfo.get();
        if (jres == null) return;
	
        for (int j=0; j<versions.length; j++) {
            vs = new VersionString(versions[j]);
            for(int i = 0; i < jres.length; i++) {

		// first check if JRE osName and osArch matches
		// (only if osName and osArch exist)
		if (jres[i].isOsInfoMatch(Config.getOSName(), Config.getOSArch())) {
	
		    if (jres[i].isEnabled()) {

			boolean jreMatch = ((location == null) ?
			    isPlatformMatch(jres[i], vs) :
			    isProductMatch(jres[i], location, vs));
			boolean pathMatch = 
			    JnlpxArgs.getJVMCommand().equals(
				new File(jres[i].getPath()));
			boolean heapMatch = JnlpxArgs.isCurrentRunningJREHeap(
					jd.getMinHeap(), jd.getMaxHeap());
			
			if (jreMatch && pathMatch && heapMatch) {
			    
			    Trace.println("LaunchSelection: findJRE: Match on current JRE", TraceLevel.BASIC);
			    
			    // Match on current JRE!
			    selectedJRE[0] = jres[i];
			    selectedJREDesc[0] = jd ;
			    return;  // We are done
			} else if (jreMatch && (selectedJRE[0] == null)) {
			    // Match, but not on current. Remember the first match ,
			    // and keep scanning to see if we get a
			    // match on current running JRE
			    
			    Trace.print("LaunchSelection: findJRE: No match on current JRE because ", TraceLevel.BASIC);
			    if (!jreMatch) Trace.print("versions dont match, ", TraceLevel.BASIC);
			    if (!pathMatch) Trace.print("paths dont match, ", TraceLevel.BASIC);
			    if (!heapMatch) Trace.print("heap sizes dont match", TraceLevel.BASIC);
			    Trace.println("", TraceLevel.BASIC);
			    
			    
			    selectedJRE[0] = jres[i];
			    selectedJREDesc[0] = jd ;
			}
		    }
		}
	    }
	}
	// Always remember the first one
	if (selectedJREDesc[0] == null) selectedJREDesc[0] = jd;
    }

    private static boolean isPlatformMatch(JREInfo jre, VersionString vs) {
	boolean isFCS;
	// A platform match is only allowed for FCS versions of JREs.
	// By convention, if the product-id contains a '-' then
	// it is non-FCS version. This convention is used for 1.3.0 and
	// higher
	String product = jre.getProduct();
	if (product != null && (product.indexOf('-') != -1) &&
	    (!product.startsWith("1.2")) && (!isInstallJRE(jre)) ) {
	    isFCS = false;
         } else {
            isFCS = true;
         }

	if ((new File(jre.getPath())).exists()) {
	    return (vs.contains(jre.getPlatform()) && isFCS);
	}
	return false;
    }

    private static boolean isProductMatch(JREInfo jre, URL location,
						  VersionString vs) {
	if ((new File(jre.getPath())).exists()) {
	    return (jre.getLocation().equals(location.toString()) &&
		vs.contains(jre.getProduct()));
	}
	return false;
    }

    private static boolean isInstallJRE(JREInfo jre) {
	File installDir = new File(Config.getJavaHome());
	File jrePath = new File(jre.getPath());
	File bindir = jrePath.getParentFile();
	return installDir.equals(bindir.getParentFile());
    }
}




