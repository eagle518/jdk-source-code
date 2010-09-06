/*
 * @(#)XMBeanNotifications.java	1.18 04/06/21
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
import java.awt.Font;

import java.text.SimpleDateFormat;

import java.awt.FlowLayout;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.event.*;
import java.awt.Insets;
import java.awt.Dimension;
import java.util.*;
import java.io.*;
import java.lang.reflect.Array;

// jaw import
import javax.management.*;
import javax.management.openmbean.CompositeData;
import javax.management.openmbean.TabularData;

import sun.tools.jconsole.Resources;

public class XMBeanNotifications extends JTable implements NotificationListener {
    
    private final static String[] columnNames =  {
	Resources.getText("TimeStamp"), 
	Resources.getText("Type"), 
	Resources.getText("UserData"),
	Resources.getText("SeqNum"), 
	Resources.getText("Message"),
	Resources.getText("Event"), 
	Resources.getText("Source")};
    
    private HashMap<ObjectName, XMBeanNotificationsListener> listeners = 
	new HashMap<ObjectName, XMBeanNotificationsListener>();
    private boolean subscribed;
    private XMBeanNotificationsListener currentListener;
    public final static String NOTIFICATION_RECEIVED_EVENT = 
	"jconsole.xnotification.received"; 
    
    private List<NotificationListener> notificationListenersList;
    private boolean enabled;
    private Font normalFont, boldFont;
    private int rowMinHeight = -1;
    private CellEditor cellEditor = new CellEditor();
    private NotifMouseListener mouseListener = new NotifMouseListener();
    private SimpleDateFormat timeFormater = new SimpleDateFormat("HH:mm:ss:SSS");
    
    public XMBeanNotifications() {
	super(new TableSorter(columnNames,0));
	setColumnSelectionAllowed(false);
	setRowSelectionAllowed(false);
	getTableHeader().setReorderingAllowed(false);
	ArrayList<NotificationListener> l = 
	    new ArrayList<NotificationListener>(1);
	notificationListenersList = Collections.synchronizedList(l);
	
	addMouseListener(mouseListener);

	TableColumnModel colModel = getColumnModel();
	colModel.getColumn(0).setPreferredWidth(45);
	colModel.getColumn(1).setPreferredWidth(50);
	colModel.getColumn(2).setPreferredWidth(50);
	colModel.getColumn(3).setPreferredWidth(40);
	colModel.getColumn(4).setPreferredWidth(50);
	colModel.getColumn(5).setPreferredWidth(50);
	setColumnEditors();
    }

    public boolean isCellEditable(int row, int col) {
	UserDataCell cell = getUserDataCell(row, col);

	if(cell != null && cell.isMaximized()) {
	    return true;
	}
	
	return false;
    }

    public synchronized Component prepareRenderer(TableCellRenderer renderer,
						  int row, int column) {
	//In case we have a repaint thread that is in the process of 
	//repainting an obsolete table, just ignore the call.
	//It can happen when MBean selection is switched at a very quick rate
	if(row >= getRowCount())
	    return null;
	
	Component comp = super.prepareRenderer(renderer, row, column);
	
	if (normalFont == null) {
	    normalFont     = comp.getFont();
	    boldFont       = normalFont.deriveFont(Font.BOLD);
	}
	UserDataCell cell = getUserDataCell(row, 2);
	if (column == 2 && cell != null) {
	    comp.setFont(boldFont);
	    int size = cell.getHeight();
	    if(size > 0) {
		if(getRowHeight(row) != size)
		    setRowHeight(row, size);
	    }
	} else {
	    comp.setFont(normalFont);
	}
	
	return comp;
    }
    
    public synchronized TableCellRenderer getCellRenderer(int row, 
							  int column) {
	//In case we have a repaint thread that is in the process of 
	//repainting an obsolete table, just ignore the call.
	//It can happen when MBean selection is switched at a very quick rate
	if(row >= getRowCount())
	    return null;
       
	DefaultTableCellRenderer renderer;
	String toolTip = null;
	UserDataCell cell = getUserDataCell(row, column);
	if(cell != null && cell.isInited()) {
	    renderer = (DefaultTableCellRenderer) cell.getRenderer();
	}
	else {
	    renderer = (DefaultTableCellRenderer) 
		super.getCellRenderer(row,
				      column);
	}
	
	if(cell != null)
	    toolTip = Resources.getText("Double click to expand/collapse")+". "
		+ cell.toString();
	else {
	    Object val = 
		((DefaultTableModel) getModel()).getValueAt(row,column);
	    if(val != null)
		toolTip = val.toString();
	}
	
	renderer.setToolTipText(toolTip);
	
	return renderer;
    }
    
    private UserDataCell getUserDataCell(int row, int column) {
	Object obj = ((DefaultTableModel) getModel()).getValueAt(row,column);
	if(obj instanceof UserDataCell) return (UserDataCell) obj;
	return null;
    }

    synchronized void dispose() {
	listeners.clear();
    }
    
    public long getReceivedNotifications(XMBean mbean) {
	XMBeanNotificationsListener listener = 
	    (XMBeanNotificationsListener) listeners.get(mbean.getObjectName());
	if(listener == null) return 0;
	else
	    return listener.getReceivedNotifications();
    }
    
    public synchronized boolean clearCurrentNotifications() {
	emptyTable();
	if(currentListener != null) {
	    currentListener.clear();
	    return true;
	} else
	    return false;
    }
    
    public synchronized boolean unregisterListener(XMBean mbean) {
	return unregister(mbean.getObjectName());
    }

    public synchronized void registerListener(XMBean mbean) 
	throws InstanceNotFoundException, IOException {
	if(!subscribed) {
	    try {
		mbean.getMBeanServerConnection().
		    addNotificationListener(new ObjectName("JMImplementation:type=MBeanServerDelegate"),
					    this,
					    null,
					    null);
		subscribed = true;
	    }catch(Exception e) {
		System.out.println("Error adding listener for delegate :"+ 
				   e.getMessage());
	    }
	}
	
	XMBeanNotificationsListener listener = 
	    listeners.get(mbean.getObjectName());
	if(listener == null) {
	    listener = new XMBeanNotificationsListener(this, 
						       mbean, 
						       columnNames);
	    listeners.put(mbean.getObjectName(), listener);
	} else {
	    if(!listener.isRegistered()) {
		emptyTable();
		listener.register();
	    }
	}
	enabled = true;
	currentListener = listener;
    }

    public synchronized void handleNotification(Notification notif,
						Object handback) {
	try {
	    if (notif instanceof MBeanServerNotification) {
		ObjectName mbean = 
		    ((MBeanServerNotification)notif).getMBeanName();
		if (notif.getType().indexOf("JMX.mbean.unregistered")>=0){
		    unregister(mbean);		    
		}
	    }
	} catch(Exception e) {
	     System.out.println("Error unregistering notification:"+ 
			       e.getMessage());
	}
    }
    
    public synchronized void disableNotifications() {
	emptyTable();
	currentListener = null;
	enabled = false;
    }

    private synchronized boolean unregister(ObjectName mbean) {
	XMBeanNotificationsListener listener = listeners.get(mbean);
	if(listener != null && listener.isRegistered()) {
	    listener.unregister();
	    currentListener = null;
	    return true;
	} else
	    return false;
    }

    public void addNotificationsListener(NotificationListener nl) {
	    notificationListenersList.add(nl);
    }
	
    public void removeNotificationsListener(NotificationListener nl) {
	notificationListenersList.remove(nl);
    }
    
    void fireNotificationReceived(XMBeanNotificationsListener listener,
				  XMBean mbean,
				  Object[] rowData,
				  long received) {
	if(enabled) {
	    DefaultTableModel tableModel = (DefaultTableModel) getModel();
	    if(listener == currentListener) {
		
		//tableModel.addRow(rowData);
		tableModel.insertRow(0, rowData);
	
		//tableModel.newDataAvailable(new TableModelEvent(tableModel));
		repaint();
	    }
	}
	
	Notification notif = new Notification(NOTIFICATION_RECEIVED_EVENT,
					      this,
					      0);
	notif.setUserData(new Long(received));
	for(NotificationListener nl : notificationListenersList) 
	    nl.handleNotification(notif,mbean);
    }

    private void updateModel(List<Object[]> data) {
	emptyTable();
	DefaultTableModel tableModel = (DefaultTableModel) getModel();
	for(Object[] rowData : data) 
	    tableModel.addRow(rowData);
    }

    public synchronized boolean isListenerRegistered(XMBean mbean) {
	XMBeanNotificationsListener listener = 
	    listeners.get(mbean.getObjectName());
	if(listener == null) return false;
	return listener.isRegistered();
    }
    
    public synchronized void loadNotifications(XMBean mbean) {
	XMBeanNotificationsListener listener = 
	    listeners.get(mbean.getObjectName());
	emptyTable();
	if(listener != null ) {
	    enabled = true;
	    List<Object[]> data = listener.getData();
	    updateModel(data);
	    currentListener = listener;
	    validate();
	    repaint();
	} else 
	    enabled = false;
    }

    private void setColumnEditors() {
	TableColumnModel tcm = getColumnModel();
	for (int i=0;i<columnNames.length;i++) {
	    if(i == 2) {
		TableCellEditor editor = getCellEditor(i);
		if (editor != null) {
		    TableColumn tc = tcm.getColumn(i);
		    tc.setCellEditor(getCellEditor(i));
		}
	    }
	}
    }
    public TableCellEditor getCellEditor(int column) {
	return cellEditor;
    }

    public boolean isTableEditable() {
	return true;
    }

    public synchronized void emptyTable() {
	DefaultTableModel model = (DefaultTableModel)getModel();
	//invalidate();
	while (model.getRowCount()>0)
	    model.removeRow(0);
	validate();
    }

    synchronized void updateUserDataCell(int row,
					 int col) {
	Object obj = getModel().getValueAt(row, 2);
	if(obj instanceof UserDataCell) {
	    UserDataCell cell = (UserDataCell) obj;
	    if(!cell.isInited()) {	
		if(rowMinHeight == -1)
		    rowMinHeight = getRowHeight(row);
		
		cell.init(super.getCellRenderer(row, col),
			  rowMinHeight);
	    }
	    
	    cell.switchState();
	    setRowHeight(row, 
			 cell.getHeight());
	    
	    if(!cell.isMaximized()) {
		//Back to simple editor.
		editCellAt(row, 
			   2);
	    }
	    
	    invalidate();
	    repaint();
	}
    }

    class UserDataCellRenderer extends DefaultTableCellRenderer {
	Component comp;
	UserDataCellRenderer(Component comp) {
	    this.comp = comp;
	}
	
	public Component getTableCellRendererComponent(JTable table, 
						       Object value, 
						       boolean isSelected, 
						       boolean hasFocus, 
						       int row, 
						       int column) {
	    return comp;
	}
	
	public Component getComponent() {
	    return comp;
	}
	
    }
    
    class UserDataCell {
	TableCellRenderer minRenderer;
	UserDataCellRenderer maxRenderer;
	int minHeight;
	boolean minimized = true;
	boolean init = false;
	Object userData;
       UserDataCell(Object userData, Component max) {
	   this.userData = userData;
	   this.maxRenderer = new UserDataCellRenderer(max);
	   
       }
       
       public String toString() {
	   if(userData == null) return null;
	   if(userData.getClass().isArray()) {
	       String name = 
		   Utils.getArrayClassName(userData.getClass().getName());
	       int length = Array.getLength(userData);
	       return name + "[" + length +"]";
	   }
	    
	    if(userData instanceof CompositeData ||
	       userData instanceof TabularData)
		return userData.getClass().getName();
	    
	    return userData.toString();
       }
	
	boolean isInited() {
	    return init;
	}
	
	void init(TableCellRenderer minRenderer,
		  int minHeight) {
	    this.minRenderer = minRenderer;	        
	    this.minHeight = minHeight;
	    init = true;
	}
	
	void switchState() {
	    minimized = !minimized;
	}
	boolean isMaximized() {
	    return !minimized;
	}
	void minimize() {
	    minimized = true;
	}
	
	void maximize() {
	    minimized = false;
	}
	
	int getHeight() {
	    if(minimized) return minHeight;
	    else 
		return (int) maxRenderer.getComponent().
		    getPreferredSize().getHeight() ;
	}
	
	TableCellRenderer getRenderer() {
	    if(minimized) return minRenderer;
	    else return maxRenderer;
	}
    }
    
    class NotifMouseListener extends MouseAdapter {
	
	public void mousePressed(MouseEvent e) {
	    if(e.getButton() == MouseEvent.BUTTON1) {
		if(e.getClickCount() >= 2) {
		    int row = XMBeanNotifications.this.getSelectedRow();
		    int col = XMBeanNotifications.this.getSelectedColumn();
		    if(col != 2) return;
		    if(col == -1 || row == -1) return;
		    
		    XMBeanNotifications.this.updateUserDataCell(row, 
								col);
		}
	    }   
	}
    }

    class CellEditor extends XTextFieldEditor {
	// implements javax.swing.table.TableCellEditor
	public Component getTableCellEditorComponent(JTable table, 
						     Object value,
						     boolean isSelected, 
						     int row, 
						     int column) {
	    Object val = value;
	    if(column == 2) {		
		Object obj = getModel().getValueAt(row,
						   column);
		if(obj instanceof UserDataCell) {
		    UserDataCell cell = (UserDataCell) obj;
		    if(cell.getRenderer() instanceof UserDataCellRenderer) {
			UserDataCellRenderer zr = 
			    (UserDataCellRenderer) cell.getRenderer();
			return zr.getComponent();
		    }
		}
	    }
	    return super.getTableCellEditorComponent(table, 
						     val, 
						     isSelected, 
						     row, 
						     column);   
	}
    }

    class XMBeanNotificationsListener implements NotificationListener {
	private String[] columnNames;
	private XMBean xmbean;
	private long received;
	private XMBeanNotifications notifications;
	private boolean unregistered;
	private ArrayList<Object[]> data = new ArrayList<Object[]>();
	public XMBeanNotificationsListener(XMBeanNotifications notifications,
					   XMBean xmbean,
					   String[] columnNames) {
	    this.notifications = notifications;
	    this.xmbean = xmbean;
	    this.columnNames = columnNames;
	    register();
	}
    
	public synchronized List<Object[]> getData() {
	    return data;
	}

	public synchronized void clear() {
	    data.clear();
	    received = 0;
	}

	public boolean isRegistered() {
	    return !unregistered;
	}

	public synchronized void unregister() {
	    try {
		xmbean.getMBeanServerConnection().
		    removeNotificationListener(xmbean.getObjectName(),this,null,null);
	    }catch(Exception e) {
		System.out.println("Error removing listener :"+ 
				   e.getMessage());
	    }
	    unregistered = true;
	}

	public long getReceivedNotifications() {
	    return received;
	}

	public synchronized void register() {
	    clear();
	    try {
		xmbean.getMBeanServerConnection().
		    addNotificationListener(xmbean.getObjectName(),this,null,null);
		unregistered = false;
	    }catch(Exception e) {
		System.out.println("Error adding listener :"+ 
				   e.getMessage());
	    }
	}
    
	public synchronized void handleNotification(Notification e,
						    Object handback) {
	    try {
		if(unregistered) return;
		Date receivedDate = new Date(e.getTimeStamp());
		String time = timeFormater.format(receivedDate);
		
		Object userData = e.getUserData();
		Component comp = null;
		UserDataCell cell = null;
		if((comp = XDataViewer.createNotificationViewer(userData)) 
		   != null) {
		    XDataViewer.registerForMouseEvent(comp, mouseListener);
		    cell = new UserDataCell(userData, comp);
		}
	    
		Object[] rowData = {time, 
				    e.getType(), 
				    (cell == null ? userData : cell), 
				    new Long(e.getSequenceNumber()), 
				    e.getMessage(), 
				    e, 
				    e.getSource()};
		received++;
		data.add(0, rowData);
	    
		notifications.fireNotificationReceived(this,
						       xmbean,
						       rowData,
						       received);  
	    }
	    catch (Exception ex) {
		ex.printStackTrace();
		System.out.println("Error when handling notification :"+ 
				   ex.toString());
	    }
	}
    }
}


    

