/*
 * @(#)XMBeanInfo.java	1.12 04/06/21
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
import java.awt.Font;
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

import sun.tools.jconsole.Resources;

public class XMBeanInfo extends JTable {
    private final int NAME_COLUMN = 0;
    private final int VALUE_COLUMN = 1;

    private final String[] columnNames =  {Resources.getText("Name"),
					   Resources.getText("Value")};
    private Font normalFont, boldFont;
    private int rowMinHeight = -1;
    
    public XMBeanInfo() {
	super();
	setCellSelectionEnabled(false);
	setRowSelectionAllowed(false);
	setColumnSelectionAllowed(false);
	getTableHeader().setReorderingAllowed(false);
	((DefaultTableModel)getModel()).setColumnIdentifiers(columnNames);
	getColumnModel().getColumn(NAME_COLUMN).setPreferredWidth(140);
	getColumnModel().getColumn(NAME_COLUMN).setMaxWidth(140);
	setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
	addMouseListener(new NotifMouseListener());
    }
    
    public boolean isCellEditable(int row, int column) {
	return false;
    }
    
    public void emptyTable() {
	invalidate();
	while (getModel().getRowCount()>0)
	    ((DefaultTableModel) getModel()).removeRow(0);
	validate();
    }

    public void loadInfo(XMBean mbean, MBeanInfo mbeanInfo) {
	emptyTable();
	DefaultTableModel tableModel = (DefaultTableModel) getModel();

	MBeanNotificationInfo[] notifs = mbeanInfo.getNotifications();
	
	Object rowData[] = new Object[2];
	rowData[0] = Resources.getText("MBean Name");
	rowData[1] = mbean.getObjectName();
	tableModel.addRow(rowData);
	rowData[0] = Resources.getText("MBean Java Class");
	try {
	    rowData[1] = mbean.getClassName();
	}
	catch(Exception e) {
	    rowData[1] = Resources.getText("Unavailable");
	}
	tableModel.addRow(rowData);

	if(notifs != null) {
	    for(int i = 0; i < notifs.length; i++) {
		rowData[0] = Resources.getText("MBean Notification");
		rowData[1] = null;
		
		NotifCell cell = 
		    new NotifCell(notifs[i]);
		
		rowData[1] = cell;
		
		tableModel.addRow(rowData);
	    }
	}
	
	tableModel.newDataAvailable(new TableModelEvent(tableModel));
	return;
    }
    
    public Component prepareRenderer(TableCellRenderer renderer,
				     int row, int column) {
	Component comp = super.prepareRenderer(renderer, row, column);
	
	if (normalFont == null) {
	    normalFont     = comp.getFont();
	    boldFont       = normalFont.deriveFont(Font.BOLD);
	}
	
	if (column == VALUE_COLUMN && 
	    (getNotifCell(row, VALUE_COLUMN) != null)) {
	    comp.setFont(boldFont);
	} else {
	    comp.setFont(normalFont);
	}
	
	return comp;
    }
    public TableCellRenderer getCellRenderer(int row, int column) {
	DefaultTableCellRenderer renderer;
	NotifCell cell = getNotifCell(row, column);
	if(cell != null && cell.isInited())
	    renderer = (DefaultTableCellRenderer) cell.getRenderer();
	else
	    renderer = (DefaultTableCellRenderer)super.getCellRenderer(row,
								       column);
	
	if(cell != null)
	    renderer.setToolTipText(Resources.getText("Double click to "+
						      "expand/collapse") +
				    ". " + cell.toString());
	return renderer;
    }
    
    private NotifCell getNotifCell(int row, int column) {
	Object obj = ((DefaultTableModel) getModel()).getValueAt(row,column);
	if(obj instanceof NotifCell) return (NotifCell) obj;
	return null;
    }
    
    void updateNotifCell(int row,
			 int col) {
	Object obj = getModel().getValueAt(row, VALUE_COLUMN);
	
	if(obj instanceof NotifCell) {
	    NotifCell cell = (NotifCell) obj;
	    if(!cell.isInited()) {	
		if(rowMinHeight == -1)
		    rowMinHeight = getRowHeight(row);
		
		cell.init(super.getCellRenderer(row, col),
			  rowMinHeight);
	    }
	    
	    cell.switchState();    
	    setRowHeight(row, 
			 cell.getHeight());

	    invalidate();
	    repaint();
	}
    }
    
    class NotifCellRenderer extends  DefaultTableCellRenderer {
	Component comp;
	NotifCellRenderer(Component comp) {
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
    
    class NotifCell {
	TableCellRenderer minRenderer;
	NotifCellRenderer maxRenderer;
	int minHeight;
	boolean minimized = true;
	boolean init = false;
	String name;
	String description;
	String[] types;
	NotifCell(MBeanNotificationInfo info) {
	    types = info.getNotifTypes();
	    name = info.getName();
	    description = info.getDescription();
	}
       
	public String toString() {
	    return description;
	}
	
	boolean isInited() {
	    return init;
	}
	
	void init(TableCellRenderer minRenderer,
		  int minHeight) {
	    Component comp = 
		XArrayDataViewer.loadArray(types);
	    this.minRenderer = minRenderer;	    
	    this.maxRenderer = new NotifCellRenderer(comp);	    
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
		    int row = XMBeanInfo.this.getSelectedRow();
		    int col = XMBeanInfo.this.getSelectedColumn();
		    if(col != VALUE_COLUMN) return;
		    if(col == -1 || row == -1) return;
		    
		    XMBeanInfo.this.updateNotifCell(row, col);
		}
	    }   
	}
    }

}
	
	
