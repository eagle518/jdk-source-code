/*
 * @(#)XSheet.java	1.16 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.FlowLayout;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.event.*;
import java.awt.Insets;
import java.awt.Dimension;
import java.util.*;
import java.io.*;

// jaw import
import javax.management.*;
import javax.management.openmbean.CompositeData;
import sun.tools.jconsole.Resources;
import sun.tools.jconsole.MBeansTab;

public class XSheet extends JPanel 
    implements ActionListener, NotificationListener, ChangeListener  {

    private JComponent lastPanel;

    //mbean currently displayed
    private XMBean mbean;
   
    //MBean Pane
    private XMBeanPane mbeanPane;
   
    //Object Pane
    private XPane objectPane;

    //Array Pane
    private XPane arrayPane;

    //XMBeanTree
    protected XMBeanTree mbeanTree;

    //Refresh JButton (mbean case)
    private JButton refreshButton;
    
    //Refresh JButton (notif case)
    private JButton clearButton, subscribeButton, unsubscribeButton;

    //current TabbedPane index for mbean already displayed
    private HashMap<XMBean, Integer> tabIndex = new HashMap<XMBean, Integer>();

    //current mbean type
    protected int currentType = -1;

    protected final static int MBEAN = 1;
    protected final static int OBJECT = 2;
    protected final static int ARRAY = 3;

    private XDataViewer viewer;

    private JPanel south;
    private MBeansTab mbeansTab;
    
    private JFrame window = new JFrame();

    public XSheet(XMBeanTree mbeanTree, 
		  XDataViewer viewer,
		  MBeansTab mbeansTab) {
	this.mbeanTree = mbeanTree;
	this.viewer = viewer;
	this.mbeansTab = mbeansTab;
	//only single selection mode supported yet
	mbeanTree.getSelectionModel().
	    setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
	setupScreen();
    }

    public void dispose() {
	if(mbeanPane != null)
	    mbeanPane.dispose(mbeansTab);
    }

    private void setupScreen() {
	setLayout(new BorderLayout());
	add(lastPanel = new JPanel(),BorderLayout.CENTER);
	// add the refresh button
	south = new JPanel();
	// add a unregister button
	south.add(refreshButton = new JButton(Resources.getText("Refresh")),
		  BorderLayout.SOUTH);
	refreshButton.addActionListener(this);
	refreshButton.setEnabled(false);
	// add a clear button
	clearButton = new JButton(Resources.getText("Clear"));
	clearButton.addActionListener(this);
	subscribeButton = new JButton(Resources.getText("Subscribe"));
	subscribeButton.addActionListener(this);
	unsubscribeButton = new JButton(Resources.getText("Unsubscribe"));
	unsubscribeButton.addActionListener(this);
	add(south,BorderLayout.SOUTH);	
    }
    
    public synchronized void notificationReceived() {
	XPane mbeanPane = getMBeanPane(mbean);
    }

    public void displaySelectedPaths(List paths) {
	if (paths == null) {
	    clearCurrentPane();
	}
	else if (paths.size() == 0) {
	    clearCurrentPane();
	}
	else {
	    try {
		XMBean selectedMBean = 
		    (XMBean) ((DefaultMutableTreeNode) 
			      ((TreePath) paths.get(0)).getLastPathComponent())
		    .getUserObject();
		
		display(selectedMBean);
	    }
	    catch (Throwable ex) {
		EventQueue.invokeLater(new ThreadDialog(this, 
							ex.getMessage(),
							Resources.
							getText("Problem " +
								"displaying "+
								"MBean"), 
							JOptionPane.
							ERROR_MESSAGE));
	    }
	}
    }

    public void displaySelectedMBeans(List mbeans) {
	if (mbeans == null) {
	    clearCurrentPane();
	}
	else if (mbeans.size() == 0) {
	    clearCurrentPane();
	}
	else {
	    try {
		display((XMBean) mbeans.get(0));
	    }
	    catch (Throwable ex) {
		EventQueue.invokeLater(new ThreadDialog(this, 
							ex.getMessage(),
							Resources.getText("Problem displaying MBean"), 
							JOptionPane.ERROR_MESSAGE));
	    }
	}
    }
    
    private void initMBeanPane() {
	if (mbeanPane == null) {
	    mbeanPane = new XMBeanPane();
	    mbeanPane.init();
	    mbeanPane.addChangeListener(this);
	}
    }

    protected XPane getMBeanPane(XMBean mbean) {
	initMBeanPane();
	return mbeanPane;
    }

    protected XPane getCurrentPane() {
	if (currentType != -1) {
	    return mbeanPane;
	}
	else {
	    return null;
	}
    }
    
    public void stateChanged(ChangeEvent e) {
	if(isNotificationPaneSelected()) {
	    south.remove(refreshButton);
	    south.add(subscribeButton, BorderLayout.EAST);
	    south.add(unsubscribeButton, BorderLayout.CENTER);
	    south.add(clearButton, BorderLayout.WEST);
	} else {
	    south.remove(clearButton);
	    south.remove(subscribeButton);
	    south.remove(unsubscribeButton);
	    south.add(refreshButton, BorderLayout.SOUTH);
	}
	validate();
	repaint();
    }
    
    public void registerListener(XMBean mbean) 
	throws InstanceNotFoundException, IOException {
	mbeanPane.registerListener(mbean);
	validate();
    }
    
    public void unregisterListener(XMBean mbean) {
	boolean unregistered = mbeanPane.unregisterListener(mbean);
	if(unregistered)
	    validate();
    }

    public void display(final XMBean mbean) throws Exception {
	mbeansTab.workerAdd(new Runnable() {
		public void run() {
		    try {
			//saves panel index
			if (lastPanel instanceof XTabbedPane) {
			    int selectedIndex = 
				((XTabbedPane) lastPanel).getSelectedIndex();
			    tabIndex.put(XSheet.this.mbean, selectedIndex);
			}
			
			final MBeanInfo mbeanInfo = mbean.getMBeanInfo();
			
			currentType = MBEAN;
			initMBeanPane();
			
			mbeanPane.load(mbean,mbeanInfo);
			JComponent newPanel = null;
			if (lastPanel!= mbeanPane) {
			    newPanel = mbeanPane;
			} else
			    newPanel = lastPanel;
			
			//sets panel index
			if (tabIndex.containsKey(mbean)) {
			    int selectedIndex = tabIndex.get(mbean);
			    ((XMBeanPane)newPanel).
				setSelectedIndex(selectedIndex, false);
			}
			else {
			    ((XMBeanPane)newPanel).setSelectedIndex(0, false);
			    tabIndex.put(mbean, 0);
			}
			
			mbeanPane.enableNotifications(mbean);
			XSheet.this.mbean = mbean;
		    }catch (Throwable ex) {
			EventQueue.invokeLater(new ThreadDialog(XSheet.this, 
								ex.getMessage(),
								Resources.getText("Problem displaying MBean"), 
								JOptionPane.ERROR_MESSAGE));
		    }
		    updatePanel(mbeanPane);
		}
	    });	   
    }		 
    
    private void updatePanel(JComponent newPanel) {
	if (lastPanel!=newPanel) {
	    invalidate();
	    remove(lastPanel);
	    lastPanel = newPanel;
	    add(lastPanel,BorderLayout.CENTER);
	    validate();
	}
	mbeanPane.enable();
	refreshButton.setEnabled(true);
	validateTree();
	repaint();
    }

    private boolean isNotificationPaneSelected() {
	return mbeanPane.isNotificationPaneSelected();
    }

    private void clearCurrentPane() {
	if (currentType != -1) {
	    mbean = null;
	    invalidate();
	    mbeanPane.disable();
	    refreshButton.setEnabled(false);
	    getCurrentPane().clear();
	    tabIndex.clear();
	    currentType = -1;
	    validateTree();
	    repaint();
	}
	
    }

    public void refreshCurrentTable() {
	try {
	    if (currentType != -1) {
		getCurrentPane().refresh();
	    }
	}catch(IOException e) {
	    System.out.println("Error while refreshing :" + e.toString());
	}
    }
	
    public void displayInWindow(Component source,
				Component component, 
				String title) {
	try {
	    window.getContentPane().getComponent(0);
	    window.getContentPane().remove(0);
	}catch(Exception e) {
	    // Not yet added.
	}
	JPanel p = new JPanel();
	window.getContentPane().add(p.add(component));
	window.setTitle(title);
	//window.setPreferredSize(new Dimension(200, 200));
	int width = component.getPreferredSize().getWidth() > 200 ? 
	    (int)component.getPreferredSize().getWidth() : 200;
	int height = component.getPreferredSize().getHeight() > 150 ?
	    (int) component.getPreferredSize().getHeight() : 150;
	Dimension d = new Dimension(width + 20, height + 20);
	window.setPreferredSize(d);
	window.setLocationRelativeTo(source);
	//window.setLocation(getLocationOnScreen());
	window.pack();
	window.setVisible(true);
    }

    public void handleNotification(Notification e, 
				   Object handback) {
	//Tree Selection
	if (e.getType().equals(XMBeanTree.SELECTION_CHANGED_EVENT)) {
	    displaySelectedPaths((List) handback);
	}
	//Node Update
	else if (e.getType().equals(XMBeanTree.VALUE_UPDATE_EVENT)) {
	    XMBean updatedMBean = (XMBean) handback;
	    if (updatedMBean.equals(mbean)) {
		refreshCurrentTable();
	    }
	}
	//Operation result
	else if (e.getType().equals(XOperations.OPERATION_INVOCATION_EVENT)) {
	    if(handback != null) {
		
		Component comp = 
		    viewer.createOperationViewer(handback, 
						 mbean);
		if(comp != null) {
		    displayInWindow((Component) e.getSource(),
				    comp, 
				    Resources.getText("Operation return value"));
		    return;
		}
	    }
	    JOptionPane.showMessageDialog(this, 
					  handback == null ? "null" : 
					  handback,
					  Resources.getText("Operation return value"), 
					  JOptionPane.INFORMATION_MESSAGE);
	    refreshCurrentTable();
	}
	//Nb notifications changed
	else if (e.getType().equals(XMBeanNotifications.
				    NOTIFICATION_RECEIVED_EVENT)) {
	    XMBean listener = (XMBean) handback;
	    Long received = (Long) e.getUserData();
	    if(listener.equals(mbean))
		mbeanPane.updateReceivedNotifications(received.longValue());
	}
    }
    
    public void actionPerformed(ActionEvent e) {
	if (e.getSource() instanceof JButton) {
	    JButton button = (JButton) e.getSource();
	    if (button == refreshButton) {
		mbeansTab.workerAdd(new Runnable() {
			public void run() {
			    refreshCurrentTable();   
			}
		    });
		return;
	    }
	    
	    if (button == clearButton) {
		clearCurrentNotifications();
		return;
	    }
	    
	    if (button == subscribeButton) {
		mbeansTab.workerAdd(new Runnable() {
			public void run() {
			    try {
				registerListener(mbean);
			    }catch(Throwable ex) {
				ex = Utils.getActualException(ex);
				EventQueue.invokeLater(new ThreadDialog(XSheet.this, 
									ex.getMessage(),
									Resources.getText("Problem adding listener"), 
									JOptionPane.ERROR_MESSAGE));
			    }
			}
		    });
		return;
	    }
	    
	    if (button == unsubscribeButton) {
		mbeansTab.workerAdd(new Runnable() {
			public void run() {
			    try {
				unregisterListener(mbean);
			    }catch(Throwable ex) {
				ex = Utils.getActualException(ex);
				EventQueue.invokeLater(new ThreadDialog(XSheet.this, 
									ex.getMessage(),
									Resources.getText("Problem adding listener"), 
									JOptionPane.ERROR_MESSAGE));
			    }
			}
		    });
		return;
	    }   
	}
    }
    
    private void clearCurrentNotifications() {
	mbeanPane.clearCurrentNotifications();
    }
    
    protected XSheet getSheet() {
	return this;
    }
    
    protected class XMBeanPane extends XPane {

	private XMBeanAttributes mbeanAttributes;
	private XMBeanOperations mbeanOperations;
	private XMBeanNotifications mbeanNotifications;
	private XMBeanInfo mbeanInformations;	

	private boolean[]  loadedIndexes;
	private XMBean mbean;
	private MBeanInfo mbeanInfo;
	
	public void disable() {
	    setEnabledAt(0, false);
	    setEnabledAt(1, false);
	    disableNotifications();
	    setEnabledAt(3, false);
	}
	
	public void dispose(MBeansTab tab) {
	    XDataViewer.dispose(tab);
	    mbeanNotifications.dispose();
	}

	public void enable() {
	    setEnabledAt(0, (mbeanInfo.getAttributes().length > 0));
	    setEnabledAt(1, (mbeanInfo.getOperations().length > 0));
	    setEnabledAt(3, true);
	}

	private void disableNotifications() {
	    clearNotifications();
	    mbeanNotifications.disableNotifications();
	}

	public void clearCurrentNotifications() {
	    if(mbeanNotifications.clearCurrentNotifications())
		setTitleAt(2, Resources.getText("Notifications") +
			   "[0]");
	}
	
	public boolean isNotificationPaneSelected() {
	    int index = getSelectedIndex();
	    if(index == 2)
		return true;
	    else
		return false;
	}

	public void enableNotifications(XMBean mbean)  {
	    if(mbean.isBroadcaster()) {
		setEnabledAt(2, true);
		if (mbeanNotifications.isListenerRegistered(mbean)) {
		    long received = 
			getReceivedNotifications(mbean);
		    setTitleAt(2, Resources.getText("Notifications") +
			       "[" + received + "]");
		} else
		    setTitleAt(2, Resources.getText("Notifications"));
	    }
	    else {
		clearNotifications();
	    }
	}
	
	private void clearNotifications() {
	    setEnabledAt(2, false);
	    setTitleAt(2, Resources.getText("Notifications"));
	}

	public void init() {
	    
	    add(Resources.getText("Attributes"),
		new JScrollPane(mbeanAttributes = 
				new  XMBeanAttributes(viewer, mbeansTab)));
	    
	    add(Resources.getText("Operations"),
		new JScrollPane(mbeanOperations = 
				new XMBeanOperations(mbeansTab)));
	    mbeanOperations.addOperationsListener(getSheet());
	    
	    add(Resources.getText("Notifications"),
		new JScrollPane(mbeanNotifications = 
				new XMBeanNotifications()));
	    mbeanNotifications.addNotificationsListener(getSheet());
	    
	    add(Resources.getText("Info"),
		new JScrollPane(mbeanInformations = new XMBeanInfo()));
	}
	
	public void setSelectedIndex(int index) {
	    setSelectedIndex(index, true);
	}

	public void setSelectedIndex(final int index, 
				     final boolean threadIt) {
	    if(threadIt) {
		mbeansTab.workerAdd(new Runnable() {
			public void run() {
			    setSelectedIndex(index, mbean, mbeanInfo);
			}
		    });
	    } else {
		setSelectedIndex(index, mbean, mbeanInfo);
	    }
	}

	private void setSelectedIndex(int index, 
				      XMBean mbean, 
				      MBeanInfo mbeanInfo) {
	    load(index,mbean,mbeanInfo);
	    super.setSelectedIndex(index);
	}

	//loads when folder selected   
	public void load(int index,XMBean mbean, MBeanInfo mbeanInfo) {
	    try {
		if (mbean == null) return; //init of the Pane
		if (loadedIndexes[index]) return; //folder already  loaded
		if (index == 0) {
		    mbeanAttributes.loadAttributes(mbean,mbeanInfo);
		}
		else if (index == 1) {
		    mbeanOperations.loadOperations(mbean,mbeanInfo);
		}
		else if (index == 2) {
		    mbeanNotifications.loadNotifications(mbean);
		}
		else if (index == 3) {		
		    mbeanInformations.loadInfo(mbean,mbeanInfo);
		}
		loadedIndexes[index] = true;
	    }
	    catch (Throwable ex) {
		EventQueue.invokeLater(new ThreadDialog(this, 
							ex.getMessage(),
							Resources.getText("Problem displaying MBean"), 
							JOptionPane.ERROR_MESSAGE));
	    }
	}
	
	void updateReceivedNotifications(long received) {
	    setTitleAt(2, Resources.getText("Notifications") + 
		       "[" + received + "]");
	}
	
	//loads mbean parameters
	public void load(XMBean mbean, MBeanInfo mbeanInfo) {
	    this.mbean = mbean;
	    this.mbeanInfo = mbeanInfo;
	    loadedIndexes = new boolean[4];
	}
	
	public void registerListener(XMBean mbean) 
	    throws InstanceNotFoundException, IOException {
	    mbeanNotifications.registerListener(mbean);
	    enableNotifications(mbean);
	}
	
	public boolean  unregisterListener(XMBean mbean) {
	    boolean unregistered = 
		mbeanNotifications.unregisterListener(mbean);
	    if(unregistered)
		setTitleAt(2, Resources.getText("Notifications"));
	    return unregistered;
	}

	public long getReceivedNotifications(XMBean mbean) {
	    return mbeanNotifications.getReceivedNotifications(mbean);
	}

	public void clear() {
	    mbeanAttributes.emptyTable();
	    mbeanAttributes.removeAttributes();
	    mbeanOperations.removeOperations();
	    mbeanNotifications.emptyTable();
	    mbeanInformations.emptyTable();
	    mbean = null;
	}
	    
	public void refresh() throws IOException {
	    mbeanAttributes.refreshAttributes();
	}
    }
}
	

    
		
		
		
	    
    
    
