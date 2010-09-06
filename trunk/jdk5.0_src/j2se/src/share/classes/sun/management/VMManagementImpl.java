/*
 * @(#)VMManagementImpl.java	1.13 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import sun.misc.Perf;
import sun.management.counter.*;
import sun.management.counter.perf.*;
import java.nio.ByteBuffer;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.List;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Collections;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.security.action.GetPropertyAction;

/**
 * Implementation of VMManagement interface that accesses the management 
 * attributes and operations locally within the same Java virtual
 * machine.
 */
class VMManagementImpl implements VMManagement {

    private static String version;

    private static boolean compTimeMonitoringSupport;
    private static boolean threadContentionMonitoringSupport;
    private static boolean currentThreadCpuTimeSupport; 
    private static boolean otherThreadCpuTimeSupport;
    private static boolean bootClassPathSupport;

    static {
        version = getVersion0();
        if (version == null) {
            throw new InternalError("Invalid Management Version");
        }
        initOptionalSupportFields();
    }
    private native static String getVersion0();
    private native static void initOptionalSupportFields();

    // Optional supports
    public boolean isCompilationTimeMonitoringSupported() {
        return compTimeMonitoringSupport;
    }

    public boolean isThreadContentionMonitoringSupported() {
        return threadContentionMonitoringSupport;
    }

    public boolean isCurrentThreadCpuTimeSupported() {
        return currentThreadCpuTimeSupport;
    }

    public boolean isOtherThreadCpuTimeSupported() {
        return otherThreadCpuTimeSupport; 
    }

    public boolean isBootClassPathSupported() {
        return bootClassPathSupport;
    }

    public native boolean isThreadContentionMonitoringEnabled(); 
    public native boolean isThreadCpuTimeEnabled();


    // Class Loading Subsystem
    public int    getLoadedClassCount() {
        long count = getTotalClassCount() - getUnloadedClassCount();
        return (int) count;
    }
    public native long getTotalClassCount();
    public native long getUnloadedClassCount();

    public native boolean getVerboseClass();

    // Memory Subsystem
    public native boolean getVerboseGC();

    // Runtime Subsystem
    public String   getManagementVersion() {
        return version;
    }

    public String getVmId() {
        int pid = getProcessId();
        String hostname = "localhost";
        try {
            hostname = InetAddress.getLocalHost().getHostName();
        } catch (UnknownHostException e) {
            // ignore
        }
 
        return pid + "@" + hostname;
    }
    private native int getProcessId();

    public String   getVmName() {
        return System.getProperty("java.vm.name");
    }

    public String   getVmVendor() {
        return System.getProperty("java.vm.vendor");
    }
    public String   getVmVersion() {
        return System.getProperty("java.vm.version");
    }
    public String   getVmSpecName()  {
        return System.getProperty("java.vm.specification.name");
    }
    public String   getVmSpecVendor() {
        return System.getProperty("java.vm.specification.vendor");
    }
    public String   getVmSpecVersion() {
        return System.getProperty("java.vm.specification.version");
    }
    public String   getClassPath() {
        return System.getProperty("java.class.path");
    }
    public String   getLibraryPath()  {
        return System.getProperty("java.library.path");
    }
    public String   getBootClassPath( ) {
        PrivilegedAction pa
            = new GetPropertyAction("sun.boot.class.path");
        String result = (String) AccessController.doPrivileged(pa);
        return result;
    }

    private List<String> vmArgs = null;
    private List<String> unmodifiableVmArgsList = null;
    public synchronized List<String> getVmArguments() {
        if (vmArgs == null) {
            vmArgs = new ArrayList<String>();
            String args = getVmArguments0();
            if (args != null) {
                StringTokenizer st = new StringTokenizer(args, " ");
                while (st.hasMoreTokens()) {
                    vmArgs.add(st.nextToken());
                }
            }
            unmodifiableVmArgsList = Collections.unmodifiableList(vmArgs); 
        }
        return unmodifiableVmArgsList;
    }
    public native String getVmArguments0();

    public native long getStartupTime();
    public native int getAvailableProcessors();

    // Compilation Subsystem
    public String   getCompilerName() {
        String name = (String) AccessController.doPrivileged(
            new PrivilegedAction() {
                public Object run() {
                    return System.getProperty("sun.management.compiler");
                }
            });
        return name;
    }
    public native long getTotalCompileTime();

    // Thread Subsystem
    public native long getTotalThreadCount();
    public native int  getLiveThreadCount();
    public native int  getPeakThreadCount();
    public native int  getDaemonThreadCount();

    // Operating System
    public String getOsName() {
        return System.getProperty("os.name");
    }
    public String getOsArch() {
        return System.getProperty("os.arch");
    }
    public String getOsVersion() {
        return System.getProperty("os.version");
    }

    // Hotspot-specific runtime support
    public native long getSafepointCount();
    public native long getTotalSafepointTime();
    public native long getSafepointSyncTime();
    public native long getTotalApplicationNonStoppedTime();

    public native long getLoadedClassSize();
    public native long getUnloadedClassSize();
    public native long getClassLoadingTime();
    public native long getMethodDataSize();
    public native long getInitializedClassCount();
    public native long getClassInitializationTime();
    public native long getClassVerificationTime();

    // Performance Counter Support
    private PerfInstrumentation perfInstr = null;
    private boolean noPerfData = false;

    private synchronized PerfInstrumentation getPerfInstrumentation() {  
        if (noPerfData || perfInstr != null) {
             return perfInstr;
        }

        // construct PerfInstrumentation object 
        Perf perf = (Perf) AccessController.doPrivileged(new Perf.GetPerfAction());
        try {
            ByteBuffer bb = perf.attach(0, "r");
            if (bb.capacity() == 0) {
                noPerfData = true;
                return null;
            }
            perfInstr = new PerfInstrumentation(bb);
        } catch (IllegalArgumentException e) {
            // If the shared memory doesn't exist e.g. if -XX:-UsePerfData 
            // was set
            noPerfData = true;
        } catch (IOException e) {
            throw new InternalError(e.getMessage());
        }
        return perfInstr;
    }

    public List    getInternalCounters(String pattern) {
        PerfInstrumentation perf = getPerfInstrumentation();
        if (perf != null) {
            return perf.findByPattern(pattern);
        } else {
            return Collections.EMPTY_LIST;
        }
    }
}
