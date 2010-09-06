/*
 * @(#)SummaryTab.java	1.24 04/06/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.io.*;
import java.lang.management.*;
import java.lang.reflect.*;
import java.net.URL;
import java.util.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;

import static sun.tools.jconsole.Formatter.*;

class SummaryTab extends Tab implements HyperlinkListener {
    JEditorPane info;

    public static String getTabName() {
	return Resources.getText("Summary");
    }

    public SummaryTab(VMPanel vmPanel) {
	super(vmPanel, getTabName());

	setLayout(new BorderLayout());

	info = new JEditorPane();
	info.setContentType("text/html");
	info.setEditable(false);
	info.addHyperlinkListener(this);
	((DefaultCaret)info.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);

	add(new JScrollPane(info));
    }

    public void update() {
	final String str = formatSummary();
	try {
	    SwingUtilities.invokeAndWait(new Runnable() {
		public void run() {
		    info.setText(str);
		}
	    });
	} catch (InvocationTargetException ex) {
	    // Ignore
	} catch (InterruptedException ex) {
	    // Ignore
	}
    }

    String formatSummary() {
	ProxyClient proxyClient = vmPanel.getProxyClient();
	if (proxyClient.isDead()) {
	    return "";
	}

	String summary = "<html><table cellpadding=1>";

	try {
	    RuntimeMXBean         rmBean     = proxyClient.getRuntimeMXBean();
	    CompilationMXBean     cmpMBean   = proxyClient.getCompilationMXBean();
	    ThreadMXBean          tmBean     = proxyClient.getThreadMXBean();
	    MemoryMXBean          memoryBean = proxyClient.getMemoryMXBean();
	    ClassLoadingMXBean    clMBean    = proxyClient.getClassLoadingMXBean();
	    OperatingSystemMXBean osMBean    = proxyClient.getOperatingSystemMXBean();

	    String newTable = "<tr><td colspan=4><font size =-1><hr><tr><td colspan=4><font size =-1>";

	    long t = rmBean.getUptime();
	    summary += "<tr><td colspan=4><center><b>" +
                Resources.getText("Summary") + "</b></center>";

            com.sun.management.OperatingSystemMXBean sunOSMBean  =
               proxyClient.getSunOperatingSystemMXBean();
            if (sunOSMBean != null) {
	        summary += newRow(Resources.getText("Uptime"), 
                                  formatTime(t),
                                  Resources.getText("Process CPU time"),
                                  formatNanoTime(sunOSMBean.getProcessCpuTime()));
            } else {
	        summary += newRowWith4Columns(Resources.getText("Uptime"), 
                                              formatTime(t));
            }

	    if (cmpMBean.isCompilationTimeMonitoringSupported()) {
		summary += newRowWith4Columns(
                               Resources.getText("Total compile time"),
			       formatTime(cmpMBean.getTotalCompilationTime()));
	    }


	    summary += newTable;
	    summary += "<center><b><a href=Threads>" +
                Resources.getText("Threads") + "</a></b></center>";
	    int tlCount = tmBean.getThreadCount();
	    int tdCount = tmBean.getDaemonThreadCount();
	    int tpCount = tmBean.getPeakThreadCount();
	    long ttCount = tmBean.getTotalStartedThreadCount();
	    summary += newRow(Resources.getText("Live Threads"),   justify(tlCount, 5),
			      Resources.getText("Peak"),           justify(tpCount, 5));
	    summary += newRow(Resources.getText("Daemon threads"), justify(tdCount, 5),
			      Resources.getText("Total started"),  justify(ttCount, 5));

	    summary += newTable;
	    summary += "<center><b><a href=Memory>" +
                Resources.getText("Memory") + "</a></b></center>";

            MemoryUsage u = memoryBean.getHeapMemoryUsage();
            long finalizerQueueLength = memoryBean.getObjectPendingFinalizationCount();
	    summary += newRow(Resources.getText("Current heap size"), 
                              formatKBytes(u.getUsed()),
			      Resources.getText("Committed memory"),  
                              formatKBytes(u.getCommitted()));
	    summary += newRowWith4Columns(
                           Resources.getText("Maximum heap size"), 
                           formatKBytes(u.getMax()));
	    summary += newRowWith4Columns(
                           Resources.getText("Objects pending for finalization"), 
                           justify(finalizerQueueLength, 6));

	    Collection<GarbageCollectorMXBean> garbageCollectors =
					proxyClient.getGarbageCollectorMXBeans();
	    for (GarbageCollectorMXBean garbageCollectorMBean : garbageCollectors) {
		String gcName = garbageCollectorMBean.getName();
		long gcCount = garbageCollectorMBean.getCollectionCount();
		long gcTime = garbageCollectorMBean.getCollectionTime();

		summary += newRowWith4Columns
                    (Resources.getText("Garbage collector"),
                     Resources.getText("GcInfo", gcName, gcCount,
                         gcTime < 0 ? Resources.getText("Unavailable"):formatTime(gcTime)));
	    }

	    summary += newTable;
	    summary += "<center><b><a href=Classes>" +
                Resources.getText("Classes") + "</a></b></center>";
	    long clCount = clMBean.getLoadedClassCount();
	    long cuCount = clMBean.getUnloadedClassCount();
	    long ctCount = clMBean.getTotalLoadedClassCount();
	    summary += newRow(Resources.getText("Current classes loaded"), justify(clCount, 5),
			      Resources.getText("Total classes unloaded"), justify(cuCount, 5));
	    summary += newRowWith4Columns(
                           Resources.getText("Total classes loaded"),
                           justify(ctCount, 5));

	    summary += newTable;
	    summary += "<center><b>" + Resources.getText("Operating System") +
                "</b></center>";
            if (sunOSMBean != null) {
                summary += newRow(Resources.getText("Total physical memory"),
                                  formatKBytes(sunOSMBean.getTotalPhysicalMemorySize()),
                                  Resources.getText("Free physical memory"),
                                  formatKBytes(sunOSMBean.getFreePhysicalMemorySize()));
                summary += newRowWith4Columns(
                               Resources.getText("Committed virtual memory"),
                               formatKBytes(sunOSMBean.getCommittedVirtualMemorySize()));
            }

	} catch (IOException e) {
	    summary += "\n"+ e + "\n";
	} catch (UndeclaredThrowableException e) {
	    summary += "\n"+ e + "\n";
	    proxyClient.markAsDead();
	}
	return summary;
    }

    public void hyperlinkUpdate(HyperlinkEvent e) {
	if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
	    String link = e.getDescription();
	    vmPanel.setSelectedIndex(vmPanel.indexOfTab(link));
	}
    }
}
