/*
 * @(#)MBeansTab.java	1.22 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.datatransfer.*;
import java.awt.dnd.*;
import java.awt.BorderLayout;
import java.awt.EventQueue;
import java.awt.Color;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.util.Timer;

import javax.management.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.TreePath;
import javax.swing.tree.DefaultMutableTreeNode;

import sun.tools.jconsole.inspector.*;

public class MBeansTab extends Tab
    implements NotificationListener {
    private XMBeanTree beanTree;
    private XSheet xmbeanSheet;
    private XDataViewer viewer;
      
    public static String getTabName() {
	return Resources.getText("MBeans");
    }

    public MBeansTab(VMPanel vmPanel) {
	super(vmPanel, getTabName());
	try {
	    setupTab();
	    workerAdd(new Runnable() {
		    public void run() {
			MBeansTab.this.synchroniseMBeanServerView();
		    }
		});
	}catch(Exception e) {
	    System.out.println("Error when synchronizing with MBeanServer : "+
			       e);
	}
    }
    
    public void dispose() {
	super.dispose();
	xmbeanSheet.dispose();
    }

    public int getUpdateInterval() {
	return vmPanel.getUpdateInterval();
    }
    
    void synchroniseMBeanServerView() {
	try {	    
	    getMBeanServerConnection().addNotificationListener(
							       new ObjectName("JMImplementation:type=MBeanServerDelegate"),
							       this,
							       null,
							       null);
	    
	    final Set newSet = 
		getMBeanServerConnection().queryNames(null,null);
	    beanTree.removeAll();
	    
	    // now those that remain are new ones
	    Object list[] = newSet.toArray();
	    ObjectName mbean;
	    for (int i = 0; i < list.length; i++) {
		mbean = (ObjectName)list[i];
		try {
		    beanTree.addMBeanToView(MBeansTab.this, mbean);
		}catch(Exception e) {
		    System.out.println("Error adding " + mbean);
		}
	    }	
	    SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			beanTree.revalidate();
			beanTree.repaint();
			repaint();
		    }
		});
	}
	catch (Exception e) {
	    System.out.println("Error when synchronizing with MBeanServer : "+
			       e);
	}
    }  	
    
    public MBeanServerConnection getMBeanServerConnection() {
	return vmPanel.getProxyClient().getMBeanServerConnection();
    }

    public void update() {

    }
    
    private java.util.List getSelected() {
	return beanTree.getSelectedXMBeans();
    }

    private void unSelectAll() {
	beanTree.unSelectAll();
    }
    
    private void setupTab() {
	JPanel p;				
	JScrollPane theScrollPane;
	
	setLayout((new VariableGridLayout(0, 1, 4, 4, true, true)));
	JSplitPane mainSplit = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
	mainSplit.setDividerLocation(150);
	// set up the tree view panel
	JPanel treePanel  = new JPanel(new BorderLayout());
	beanTree = (XMBeanTree)new XTree();
	beanTree.setCellRenderer(new XTreeRenderer());
	
	theScrollPane = new JScrollPane(beanTree,
					JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
					JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	theScrollPane.setBorder(BorderFactory.createBevelBorder(BevelBorder.LOWERED));
	treePanel.setBackground(new Color(102,102,153));
	treePanel.add(theScrollPane,BorderLayout.CENTER);

	mainSplit.add(treePanel,JSplitPane.LEFT,0);

	//Create Viewer
	viewer = new XDataViewer(this);

	xmbeanSheet = new XSheet(beanTree, viewer, this);

	beanTree.addXMBeanListener(xmbeanSheet);
	mainSplit.add(xmbeanSheet,JSplitPane.RIGHT,0);

	JPanel pan = new JPanel(new BorderLayout());
	BorderedComponent comp = new BorderedComponent(Resources.getText("MBeans"),
						       pan,
						       false);
	pan.add(mainSplit);
	add(comp);
    }
    
    /* notification listener */
    public void handleNotification(Notification notification,
				   java.lang.Object handback) {
	if (notification instanceof MBeanServerNotification) {
	    try {
		ObjectName mbean = 
		    ((MBeanServerNotification)notification).getMBeanName();
		if (notification.getType().equals(MBeanServerNotification.REGISTRATION_NOTIFICATION)) {
		    beanTree.addMBeanToView(this,mbean);
		    return;
		}
		
		if (notification.getType().equals(MBeanServerNotification.UNREGISTRATION_NOTIFICATION))
		    beanTree.delMBeanFromView(mbean);
	    } catch (Exception t) {
		System.out.println("Error when handling notification : " + 
				   t);
	    }
	}
    }
}
