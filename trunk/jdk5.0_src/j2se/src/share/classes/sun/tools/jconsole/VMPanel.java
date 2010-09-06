/*
 * @(#)VMPanel.java	1.31 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;
import java.util.List;
import java.util.Timer;

import javax.swing.*;

public class VMPanel extends JTabbedPane {
    private ProxyClient proxyClient;
    private Timer timer;
    private int updateInterval;
    private String hostName;
    private int port;
    private int vmid;
    private String userName;
    private String password;
    private String jmxUrl;
    private boolean mmEnabled = true;
    
    private static final String windowsLaF =
	"com.sun.java.swing.plaf.windows.WindowsLookAndFeel";


    private static ArrayList<TabInfo> tabInfos = new ArrayList<TabInfo>();

    static {
	tabInfos.add(new TabInfo(SummaryTab.class, SummaryTab.getTabName(), true));
	tabInfos.add(new TabInfo(MemoryTab.class,  MemoryTab.getTabName(),  true));
	tabInfos.add(new TabInfo(ThreadTab.class,  ThreadTab.getTabName(),  true));
	tabInfos.add(new TabInfo(ClassTab.class,   ClassTab.getTabName(),   true));
	tabInfos.add(new TabInfo(MBeansTab.class,  MBeansTab.getTabName(),  true));
	tabInfos.add(new TabInfo(VMTab.class,      VMTab.getTabName(),      true));

	// Add tab classes from the command line
	String classProp = System.getProperty("jconsole.addTabClass");
	if (classProp != null) {
	    String[] userClassNames = classProp.split(":");
	    for (String className : userClassNames) {
		try {
		    Class tabClass = Class.forName(className);
		    Method getNameMethod = tabClass.getDeclaredMethod("getTabName", new Class[0]);
		    String tabName = (String)getNameMethod.invoke(null, new Object[0]);
		    tabInfos.add(new TabInfo(tabClass, tabName, true));
		} catch (Exception ex) {
		    System.err.println(ex);
		}
	    }
	}
    }

    public static TabInfo[] getTabInfos() {
	return tabInfos.toArray(new TabInfo[tabInfos.size()]);
    }

    VMPanel(ProxyClient proxyClient, int updateInterval) {
	this.proxyClient = proxyClient;
	this.updateInterval = updateInterval;
	this.hostName = proxyClient.getHostName();
	this.port     = proxyClient.getPort();
	this.vmid     = proxyClient.getVmid();
	this.userName = proxyClient.getUserName();
	this.password = proxyClient.getPassword();
	this.jmxUrl = proxyClient.getUrl();

	for (TabInfo tabInfo : tabInfos) {
	    if (tabInfo.tabVisible) {
		addTab(tabInfo);
	    }
	}

	LookAndFeel laf = UIManager.getLookAndFeel();
	setTransparency(laf.getClass().getName().equals(windowsLaF));

	TimerTask timerTask = new TimerTask() {
	    public void run() {
		update();
	    }
	};
	String timerName = "Timer-"+getConnectionName();
	timer = new Timer(timerName, true);
	timer.schedule(timerTask, 0, updateInterval);
    }

    private synchronized void addTab(TabInfo tabInfo) {
	Tab tab = instantiate(tabInfo);
	if (tab != null) {
	    addTab(tabInfo.name, tab);
	} else {
	    tabInfo.tabVisible = false;
	}
    }

    private synchronized void insertTab(TabInfo tabInfo, int index) {
	Tab tab = instantiate(tabInfo);
	if (tab != null) {
	    insertTab(tabInfo.name, null, tab, null, index);
	} else {
	    tabInfo.tabVisible = false;
	}
    }

    public synchronized void removeTabAt(int index) {    
	super.removeTabAt(index);
    }

    private Tab instantiate(TabInfo tabInfo) {
	try {
	    Constructor con = tabInfo.tabClass.getConstructor(VMPanel.class);
	    return (Tab)con.newInstance(this);
	} catch (Exception ex) {
	    System.err.println(ex);
	    return null;
	}
    }

    synchronized boolean isMMEnabled() {
	return mmEnabled;
    }
    
    public int getUpdateInterval() {
	return updateInterval;
    }

    /**
     * WARNING NEVER CALL THIS METHOD TO MAKE JMX REQUEST 
     * IF  assertThread == false.
     * DISPATCHER THREAD IS NOT ASSERTED.
     * IT IS USED TO MAKE SOME LOCAL MANIPULATIONS.
     */
    ProxyClient getProxyClient(boolean assertThread) {
	if(assertThread)
	    return getProxyClient();
	else
	    return proxyClient; 
    }
    
    public ProxyClient getProxyClient() {
	String threadClass = Thread.currentThread().getClass().getName();
	if (threadClass.equals("java.awt.EventDispatchThread")) {
	    String msg = "Calling VMPanel.getProxyClient() from the Event Dispatch Thread!";
	    new RuntimeException(msg).printStackTrace();
	    System.exit(1);
	}
	return proxyClient;
    }

    public void disconnect() {
	//proxyClient.disconnect();
	for (Tab tab : getTabs()) {
	    tab.dispose();
	}
	timer.cancel();
    }

    // A thread safe clone of JTabbedPane's Vector
    protected synchronized List<Tab> getTabs() {
	ArrayList<Tab> list = new ArrayList<Tab>();
	int n = getTabCount();
	for (int i = 0; i < n; i++) {
	    list.add((Tab)getComponentAt(i));
	}
	return list;
    }

    // Note: This method is called on a TimerTask thread. Any GUI manipulation
    // must be performed with invokeLater() or invokeAndWait().
    private void update() {
	if(!isMMEnabled()) {
	    try {
		if(!proxyClient.isDead())
		    proxyClient.getMBeanServerConnection().getDefaultDomain();
	    } catch (IOException ex) {
		proxyClient.markAsDead();
	    }
	    
	    if (proxyClient.isDead()) {
		disconnect();
		// TODO: Run this on the Event Dispatch Thread!
		JConsole jc = (JConsole)SwingUtilities.getWindowAncestor(this);
		if (jc != null) {
		    jc.vmPanelDied(this);
		}
	    }
	    return;
	}
	
	List<Tab> tabs = getTabs();
	int n = tabs.size();
	for (int i = 0; i < n; i++) {
	    try {
		if (!proxyClient.isDead()) {
		    tabs.get(i).update();
		}
	    }catch(Exception e) {
		synchronized(this) {
		    mmEnabled = false;
		}
		// TODO: Run setEnabledAt() on the Event Dispatch Thread!
		setEnabledAt(i, false);
		//Stop any polling
		timer.cancel();
		timer.purge();
	    }
	}
	if(!isMMEnabled()) {
	    // TODO: Run setSelectedIndex() on the Event Dispatch Thread!
	    // TODO: Do not use hard-coded indices!
	    setSelectedIndex(4);
	    
	    //Launch a polling dedicated to monitor MBean panel
	    TimerTask timerTask = new TimerTask() {
		    public void run() {
			update();
		    }
		};
	    String timerName = "Timer-"+ proxyClient.getConnectionName();
	    timer = new Timer(timerName, true);
	    timer.schedule(timerTask, 0, updateInterval);
	    return;
	}
	
	if (proxyClient.isDead()) {
	    disconnect();
	    // TODO: Run this on the Event Dispatch Thread!
	    JConsole jc = (JConsole)SwingUtilities.getWindowAncestor(this);
	    if (jc != null) {
		jc.vmPanelDied(this);
	    }
	}
    }

    public String getHostName() {
	return hostName;
    }

    public int getPort() {
	return port;
    }

    public String getUserName() {
	return userName;
    }

    public String getUrl() {
	return jmxUrl;
    }


    public String getPassword() {
	return password;
    }
    
    public String getConnectionName() {
	return proxyClient.getConnectionName();
    }

    public void setTransparency(boolean transparent) {
	for (Component comp : getComponents()) {
	    if (comp instanceof Tab) {
		setTransparency((JComponent)comp, transparent);
	    }
	}
    }

    private void setTransparency(JComponent comp, boolean transparent) {
	comp.setOpaque(!transparent);
	for (Component child : comp.getComponents()) {
	    if (child instanceof JPanel) {
		setTransparency((JComponent)child, transparent);
	    }
	}
    }


    static class TabInfo {
	Class tabClass;
	String name;
	boolean tabVisible;

	TabInfo(Class tabClass, String name, boolean tabVisible) {
	    this.tabClass = tabClass;
	    this.name = name;
	    this.tabVisible = tabVisible;
	}
    }
}
