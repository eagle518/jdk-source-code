/*
 * @(#)DefaultMatchJRE.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.util.Locale;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Enumeration;

import com.sun.deploy.config.JREInfo;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.VersionID;
import com.sun.deploy.util.VersionString;
import com.sun.deploy.util.JVMParameters;

/*
 * Class containing single method to determine what JRE to use.
 *
 * We have to traverse through all JREDecs of the whole JNLP graph
 * to collect all JREDesc's requirements, which are:
 *      - The highest requested JRE
 *      - The list of all JVM arguments
 * 
 * The match function will do this for us,
 * and therefor not determine if a single JREDesc will match.
 *
 * A pending matchHomeJRE call will compute the final match
 * 
 *
 */
public class DefaultMatchJRE implements LaunchSelection.MatchJREIf {
    private static final boolean DEBUG = false;

    private JREDesc       selectedJREDesc;
    private JREInfo       selectedJREInfo;

    private boolean       matchComplete;
    private boolean       matchSecureComplete;
    private boolean       matchVersion;
    private boolean       matchJVMArgs;
    private boolean       matchSecureJVMArgs;
    private long          selectedMaxHeap;
    private long          selectedInitHeap;
    private String        selectedJVMArgString;
    private JVMParameters selectedJVMArgs;

    public DefaultMatchJRE() {
        reset(null);
    }

    public boolean hasBeenRun() {
        return null!=selectedJVMArgs;
    }

    public void beginTraversal(LaunchDesc ld) {
        reset(ld);
        if(DEBUG) {
            Trace.println("\tMatch: beginTraversal");
        }
    }

    private void reset(LaunchDesc ld) {
        matchComplete=false;
        matchSecureComplete=false;
	matchVersion=false;
	matchJVMArgs=false;
	matchSecureJVMArgs=false;
        selectedInitHeap=-1;
        selectedJVMArgString=null;
        selectedJREDesc=null;
        selectedJREInfo=null;
        if(null==ld) {
            // not in traversal, just null
            selectedMaxHeap=-1;
            selectedJVMArgs = null;
        } else {
            // in traversal, real init
            selectedMaxHeap=JVMParameters.getDefaultHeapSize();
            selectedJVMArgs = new JVMParameters();
        }
    }

    public JREInfo getSelectedJREInfo() {
        return selectedJREInfo;
    }
    public JREDesc getSelectedJREDesc() {
        return selectedJREDesc;
    }
    public JVMParameters getSelectedJVMParameters()
    { return selectedJVMArgs; }

    public String getSelectedJVMParameterString() {
        return selectedJVMArgString;
    }
    public long getSelectedInitHeapSize() {
        return selectedInitHeap;
    }
    public long getSelectedMaxHeapSize() {
        return selectedMaxHeap;
    }

    public boolean isRunningJVMSatisfying(boolean includeInsecure) {
        if (includeInsecure) {
            return matchComplete;
        } else {
            return matchSecureComplete;
        }
    }
    
    public boolean isRunningJVMVersionSatisfying() {
	return matchVersion;
    }

    public boolean isRunningJVMArgsSatisfying(boolean includeInsecure) {
	if (includeInsecure) {
            return matchJVMArgs;
        } else {
            return matchSecureJVMArgs;
        }
    }

    public void digest(JREDesc jd, JREInfo ji)
    {
        if(DEBUG) {
            Trace.println("Match: digest selected JREDesc: "+jd+", JREInfo: "+ji);
        }

        selectedJREDesc = jd;
        selectedJREInfo = ji;
        
        // try the max heap size
        long heapSize = jd.getMaxHeap();
        if(heapSize>selectedMaxHeap) {
            selectedMaxHeap=heapSize;
            if(DEBUG) {
                Trace.println("\tMatch: selecting maxHeap: "+heapSize);
            }
        } else {
            if(DEBUG) {
                Trace.println("\tMatch: ignoring maxHeap: "+heapSize);
            }
        }
        // try the initial heap size
        heapSize = jd.getMinHeap();
        if(heapSize>selectedInitHeap) {
            selectedInitHeap=heapSize;
            if(DEBUG) {
                Trace.println("\tMatch: selecting InitHeap: "+heapSize);
            }
        } else {
            if(DEBUG) {
                Trace.println("\tMatch: ignoring InitHeap: "+heapSize);
            }
        }

        // vmargs JREDesc
        if(DEBUG) {
            Trace.println("\tMatch: digesting vmargs: "+jd.getVmArgs());
        }
        selectedJVMArgs.parse(jd.getVmArgs());
        if(DEBUG) {
            Trace.println("\tMatch: digested vmargs: "+selectedJVMArgs);
        }

        // take over JVMParameters heap size, 
        // which might have been altered by JREDesc's vmargs 
        heapSize = selectedJVMArgs.getMaxHeapSize();
        if(heapSize>selectedMaxHeap) {
            selectedMaxHeap=heapSize;
            if(DEBUG) {
                Trace.println("\tMatch: selecting maxHeap(2): "+heapSize);
            }
        }
        selectedJVMArgs.setMaxHeapSize(JVMParameters.getDefaultHeapSize()); // reset


        if(DEBUG) {
            Trace.println("\tMatch: JVM args after accumulation: "+selectedJVMArgs);
        }

    }

    public void digest(LaunchDesc ld) {
        ResourcesDesc rd = ld.getResources();
        if(null!=rd) {
            // properties JREDesc nested resources
            selectedJVMArgs.addProperties(rd.getResourcePropertyList());
        }

        if(DEBUG) {
            Trace.println("\tMatch: digest LaunchDesc: "+ld.getLocation());
            if(null!=rd) {
                Trace.println("\tMatch: digest properties: "+rd.getResourcePropertyList());
            } else {
                Trace.println("\tMatch: digest properties: ResourcesDesc null");
            }
            Trace.println("\tMatch: JVM args: "+selectedJVMArgs);
        }
    }

    public void endTraversal(LaunchDesc ld) {
        if(DEBUG) {
            Trace.println("\tMatch: endTraversal ..");
        }

        if(ld.isApplicationDescriptor() && null==selectedJREDesc) {
            // should not happen
            // we do not allow application/applet jnlp file with no resources
            // specified (see Launcher.prepareLaunchFile)
            throw new InternalError("selectedJREDesc null");
        }

        // set initial heap, if mentioned and different from default
        if(selectedInitHeap>0 && selectedInitHeap != JVMParameters.getDefaultHeapSize()) {
            selectedJVMArgs.parse("-Xms" + JVMParameters.unparseMemorySpec(selectedInitHeap));
        }
        // set max heap, if mentioned above default
        selectedJVMArgs.setMaxHeapSize(selectedMaxHeap);

        // final arguments
        selectedJVMArgString=selectedJVMArgs.getCommandLineArgumentsAsString(false);
        if(DEBUG) {
            Trace.println("\tMatch: JVM args final: "+selectedJVMArgString);
        }
            
        // done, if no installed JREInfo found 
        if(selectedJREInfo == null) {
            return;
        }

        JREInfo homeJREInfo = ld.getHomeJRE();
        if ( ! homeJREInfo.getProductVersion().match(selectedJREInfo.getProductVersion()) ) {
            if(DEBUG) {
                Trace.println("\tMatch: Running JREInfo Version mismatches: "+homeJREInfo.getProductVersion()+" != "+
			      selectedJREInfo.getProductVersion());
            }
            matchVersion = false;
        } else {
            if(DEBUG) {
                Trace.println("\tMatch: Running JREInfo Version    match: "+homeJREInfo.getProductVersion()+" == "+
			      selectedJREInfo.getProductVersion());
            }
	    matchVersion = true;
        }

        JVMParameters runningJVMParams = JVMParameters.getRunningJVMParameters();
        if ( runningJVMParams==null )
        {
           if (Trace.isTraceLevelEnabled(TraceLevel.BASIC))
               Trace.println("\t Match: Running JVM is not set: want:<"+
                  selectedJVMArgs.getCommandLineArgumentsAsString(false)+">", 
                  TraceLevel.BASIC);
            matchJVMArgs = false;
            matchSecureJVMArgs = false;
        } else if ( runningJVMParams.satisfies(selectedJVMArgs) ) {
            if(DEBUG) {
                Trace.println("\t Match: Running JVM args match: have:<"+
                                runningJVMParams.getCommandLineArgumentsAsString(false)+">  satisfy want:<"+
                                selectedJVMArgs.getCommandLineArgumentsAsString(false)+">");
            }
            matchJVMArgs = true;
            matchSecureJVMArgs = true;
        } else if ( runningJVMParams.satisfiesSecure(selectedJVMArgs) ) {
            if(DEBUG) {
                Trace.println("\t Match: Running JVM args match the secure subset: have:<"+
                                runningJVMParams.getCommandLineArgumentsAsString(false)+">  satisfy want:<"+
                                selectedJVMArgs.getCommandLineArgumentsAsString(false)+">");
            }
            matchJVMArgs = false;
            matchSecureJVMArgs = true;
        } else {
            if(DEBUG) {
                Trace.println("\t Match: Running JVM args mismatch: have:<"+
                                runningJVMParams.getCommandLineArgumentsAsString(false)+"> !satisfy want:<"+
                                selectedJVMArgs.getCommandLineArgumentsAsString(false)+">");
            }
            matchJVMArgs = false;
            matchSecureJVMArgs = false;
        }

	matchComplete = matchVersion & matchJVMArgs;
	matchSecureComplete = matchVersion & matchSecureJVMArgs;
    }

    public String toString() {
        return "DefaultMatchJRE: "+
               "\n  JREDesc:    "+getSelectedJREDesc()+
               "\n  JREInfo:    "+getSelectedJREInfo()+
               "\n  Init Heap:  "+getSelectedInitHeapSize()+
               "\n  Max  Heap:  "+getSelectedMaxHeapSize()+
               "\n  Satisfying: "+isRunningJVMSatisfying(true)+
                             ", "+isRunningJVMSatisfying(false)+
	       "\n  SatisfyingVersion: "+isRunningJVMVersionSatisfying()+
	       "\n  SatisfyingJVMArgs: "+isRunningJVMArgsSatisfying(true)+
	                             ", "+isRunningJVMSatisfying(false)+
               "\n  SatisfyingSecure: "+isRunningJVMSatisfying(true)+
               "\n  Selected JVMParam: "+getSelectedJVMParameters()+
               "\n  Running  JVMParam: "+JVMParameters.getRunningJVMParameters();
    }
}

