/*
 * @(#)VMTab.java	1.12 04/06/07
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


class VMTab extends Tab implements HyperlinkListener {
    JEditorPane info;
    private static final String cr = System.getProperty("line.separator");

    public static String getTabName() {
	return Resources.getText("VM");
    }

    public VMTab(VMPanel vmPanel) {
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
	final String str = formatText();
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

    String formatText() {
	ProxyClient proxyClient = vmPanel.getProxyClient();
	if (proxyClient.isDead()) {
	    return "";
	}

	String text = "<html><table>";

	try {
	    RuntimeMXBean         rmBean   = proxyClient.getRuntimeMXBean();
	    CompilationMXBean     cmpMBean = proxyClient.getCompilationMXBean();
	    OperatingSystemMXBean osMBean  = proxyClient.getOperatingSystemMXBean();

	    String newTable = "<tr><td colspan=2><font size =-1><hr><tr><td colspan=2><font size =-1>";

	    long t = rmBean.getUptime();
	    text += "<tr><td colspan=2><center><b>" +
                Resources.getText("VM Information") + "</b></center>";

            text += newRow(Resources.getText("Java Virtual Machine"),
                           rmBean.getVmName() + Resources.getText(" version ") +
                           rmBean.getVmVersion());
	    text += newRow(Resources.getText("Vendor"), rmBean.getVmVendor());
	    text += newRow(Resources.getText("Uptime"), formatTime(t));
	    text += newRow(Resources.getText("Name"), rmBean.getName());
	    String args = "";
	    java.util.List<String> inputArguments = rmBean.getInputArguments();
	    for (String arg : inputArguments) {
		args += arg + " ";
	    }
	    text += newRow(Resources.getText("VM arguments"), args);
	    text += newRow(Resources.getText("Class path"),  rmBean.getClassPath());
	    text += newRow(Resources.getText("Library path"), rmBean.getLibraryPath());
	    text += newRow(Resources.getText("Boot class path"),
			   rmBean.isBootClassPathSupported()
				? rmBean.getBootClassPath()
				: Resources.getText("Unavailable"));

	    text += newTable;
	    text += "<center><b>" + Resources.getText("Compiler") + "</b></center>";
	    text += newRow(Resources.getText("JIT compiler"), cmpMBean.getName());
	    text += newRow(Resources.getText("Total compile time"),
			   cmpMBean.isCompilationTimeMonitoringSupported()
				? formatTime(cmpMBean.getTotalCompilationTime())
				: Resources.getText("Unavailable"));

	    text += newTable;
	    text += "<center><b>" + Resources.getText("Operating System") + "</b></center>";
	    String osName = osMBean.getName();
	    String osVersion = osMBean.getVersion();
	    String osArch = osMBean.getArch();
	    int nCPUs = osMBean.getAvailableProcessors();
	    text += newRow(Resources.getText("Operating System"), osName + " " + osVersion);
	    text += newRow(Resources.getText("Architecture"), osArch);
	    text += newRow(Resources.getText("Number of processors"), nCPUs+"");

	    com.sun.management.OperatingSystemMXBean sunOSMBean  = 
               proxyClient.getSunOperatingSystemMXBean();
            if (sunOSMBean != null) {
	        text += newRow(Resources.getText("Process CPU time"), 
                               formatNanoTime(sunOSMBean.getProcessCpuTime()));
	        text += newRow(Resources.getText("Total physical memory"), 
                               formatKBytes(sunOSMBean.getTotalPhysicalMemorySize()));
	        text += newRow(Resources.getText("Free physical memory"), 
                               formatKBytes(sunOSMBean.getFreePhysicalMemorySize()));
	        text += newRow(Resources.getText("Committed virtual memory"), 
                               formatKBytes(sunOSMBean.getCommittedVirtualMemorySize()));
	        text += newRow(Resources.getText("Total swap space"), 
                               formatKBytes(sunOSMBean.getTotalSwapSpaceSize()));
	        text += newRow(Resources.getText("Free swap space"), 
                               formatKBytes(sunOSMBean.getFreeSwapSpaceSize()));
            }
             
	} catch (IOException e) {
	    text += cr + e + cr;
	} catch (UndeclaredThrowableException e) {
	    text += cr + e + cr;
	    proxyClient.markAsDead();
	}
	return text;
    }

    public void hyperlinkUpdate(HyperlinkEvent e) {
	if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
	    String link = e.getDescription();
	    vmPanel.setSelectedIndex(vmPanel.indexOfTab(link));
	}
    }
}
