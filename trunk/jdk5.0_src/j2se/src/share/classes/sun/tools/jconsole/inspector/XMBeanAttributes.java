/*
 * @(#)XMBeanAttributes.java	1.16 04/06/21
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
import java.awt.Color;
import java.awt.GridLayout;
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
import sun.tools.jconsole.MBeansTab;
import sun.tools.jconsole.Plotter;

/*IMPORTANT :
  There is a deadlock issue there if we don't synchronize well loadAttributes, 
  refresh attributes and empty table methods since a UI thread can call 
  loadAttributes and at the same time a JDMK notification can raise an 
  emptyTable. Since there are syncrhonization in the JDMK world it's 
  COMPULSORY to not call the JDMK world in synchronized blocks */
public class XMBeanAttributes extends XTable {
    private final static String[] columnNames =  
    {Resources.getText("Name"),
     Resources.getText("Value")};
    
    private boolean editable = true;
    
    private XMBean mbean;
    private MBeanInfo mbeanInfo;
    private MBeanAttributeInfo[] attributesInfo;
    private HashMap<String, Object> attributes;
    private HashMap<String, Object> unavailableAttributes;
    private HashMap<String, Object> viewableAttributes;
    private TableModelListener attributesListener;
    private MBeansTab mbeansTab;
    private XTable table;
    private CellEditor cellEditor = new CellEditor();
    private XDataViewer viewer;
    private int rowMinHeight = -1;
    private AttributesMouseListener mouseListener = new AttributesMouseListener();
    public XMBeanAttributes(XDataViewer viewer, MBeansTab mbeansTab) {
	super();
	this.mbeansTab = mbeansTab;
	((DefaultTableModel)getModel()).setColumnIdentifiers(columnNames);
	getModel().addTableModelListener(attributesListener = 
					 new AttributesListener(this)); 
	this.viewer = viewer;
	getColumnModel().getColumn(NAME_COLUMN).setPreferredWidth(40);
	
	addMouseListener(mouseListener);
	getTableHeader().setReorderingAllowed(false);
	setColumnEditors();
    }
    
    public synchronized Component prepareRenderer(TableCellRenderer renderer,
						  int row, int column) {
	//In case we have a repaint thread that is in the process of 
	//repainting an obsolete table, just ignore the call.
	//It can happen when MBean selection is switched at a very quick rate
	if(row >= getRowCount())
	    return null;
	else
	    return super.prepareRenderer(renderer, row, column);
    }
    
    void updateRowHeight(Object obj, int row) {
	ZoomedCell cell = null;
	if(obj instanceof ZoomedCell) {
	    cell = (ZoomedCell) obj;
	    if(cell.isInited())
		setRowHeight(row, cell.getHeight());
	    else
		if(rowMinHeight != - 1)
		    setRowHeight(row, rowMinHeight);
	} else
	    if(rowMinHeight != - 1)
		setRowHeight(row, rowMinHeight);
    }

    public synchronized TableCellRenderer getCellRenderer(int row, 
							  int column) {
	//In case we have a repaint thread that is in the process of 
	//repainting an obsolete table, just ignore the call.
	//It can happen when MBean selection is switched at a very quick rate
	if(row >= getRowCount())
	    return null;
	else {
	    ZoomedCell cell = null;
	    if(column == VALUE_COLUMN) {
		Object obj = getModel().getValueAt(row,
						   column);
		if(obj instanceof ZoomedCell) {
		    cell = (ZoomedCell) obj;
		    if(cell.isInited()) {
			DefaultTableCellRenderer renderer = 
			    (DefaultTableCellRenderer) cell.getRenderer();
			Color def = getDefaultColor();
			Color edit = getEditableColor();
			if(isCellEditable(row, column)) {
			    if(edit != null)
				renderer.setForeground(edit);
			} else { 
			    if(def != null) 
				renderer.setForeground(def);
			}
			
			return renderer;
		    }
		}
	    }
	    
	    if(column == NAME_COLUMN && isAttributeViewable(row, column) ) {
		DefaultTableCellRenderer renderer =  
		    (DefaultTableCellRenderer) super.getCellRenderer(row, 
								     column);
		renderer.setToolTipText(getToolTip(row,column));
	    }
	    
	    return super.getCellRenderer(row, column);
	}
    }
    
    private void setColumnEditors() {
	TableColumnModel tcm = getColumnModel();
	for (int i=0;i<columnNames.length;i++) {
	    if (isColumnEditable(i)) {
		TableCellEditor editor = getCellEditor(i);
		if (editor != null) {
		    TableColumn tc = tcm.getColumn(i);
		    tc.setCellEditor(getCellEditor(i));
		}
	    }
	}
    }
    
    public final boolean editCellAt(int row, int column, EventObject e) {
	boolean retVal = super.editCellAt(row, column, e);
	if (retVal) {
	    TableCellEditor editor = getColumnModel().getColumn(column).
		getCellEditor();
	    if (editor != null) {
		((JComponent) editor).requestFocus();
	    }
	}
	return retVal;
    }
    
    //overwritten method
    public boolean isCellEditable(int row, int col) {

	Object obj = getModel().getValueAt(row, col);
	if(obj instanceof ZoomedCell) {
	    ZoomedCell cell = (ZoomedCell) obj;
	    if(cell.isMaximized())
		return true;
	}
	
	boolean editable = (isTableEditable() && 
			    isColumnEditable(col) && 
			    isWritable(row));
	if (!editable) {
	    return false;
	}
	else {
	    String className = getClassName(row);
	    if (Utils.isEditableType(className)) {
		return true;
	    }
	    else {
		return false;
	    }
	}
    }
		
    
    //Table methods

    public TableCellEditor getCellEditor(int column) {
	return cellEditor;
    }
	

    public boolean isTableEditable() {
	return true;
    }
    
    public void setTableValue(Object value,int row) {
	setValueAt(value,row,3);
    }
	 
    public boolean isColumnEditable(int column) {
	if (column < getColumnCount()) {
	    return getColumnName(column).equals(Resources.getText("Value"));
	}
	else {
	    return false;
	}
    }		
	
    public String getClassName(int row) {
	int index = convertRowToIndex(row);
	if (index != -1) {
	    return attributesInfo[index].getType();
	}
	else {
	    return null;
	}
    }

       
    public String getValueName(int row) {
	int index = convertRowToIndex(row);
	if (index != -1) {
	    return attributesInfo[index].getName();
	}
	else {
	    return null;
	}
    }


    public Object getValue(int row) {
	return ((DefaultTableModel) getModel()).getValueAt(row, VALUE_COLUMN);
    }

    //tool tip only for editable column
    public String getToolTip(int row, int column) {
	if (isCellError(row, column)) {
	    return (String) unavailableAttributes.get(getValueName(row));
	}
	if (isColumnEditable(column)) {
	    Object value = getValue(row);
	    String tip = null;
	    if (value != null) {
		tip = value.toString();
		if(isAttributeViewable(row, VALUE_COLUMN))
		    tip = Resources.getText("Double click to expand/collapse")+
			". " + tip;
	    }
	    
	    return tip;
	}
	
	if(column == NAME_COLUMN) {
	    int index = convertRowToIndex(row);
	    if (index != -1) {
		String RW = "R/W";
		if (!attributesInfo[index].isReadable()) RW = "W";
		else if (!attributesInfo[index].isWritable()) RW = "R";
		
		String tip = Resources.getText("TipDescrTypeModifier",
                                               attributesInfo[index].getDescription(),
                                               Utils.getReadableClassName((attributesInfo[index].getType())),
                                               RW);
	    }
	}
	return null;
    }

    public synchronized boolean isWritable(int row) {
	int index = convertRowToIndex(row);
	if (index != -1) {
	    return (attributesInfo[index].isWritable());
	}
	else {
	    return false;
	}
    }
    
    /**
     * Override JTable method in order to make any call to this method
     * atomic with TableModel elements.
     */
    public synchronized int getRowCount() {
	return super.getRowCount();
    }
    
    public synchronized boolean isReadable(int row) {
	int index = convertRowToIndex(row);
	if (index != -1) {
	    return (attributesInfo[index].isReadable());
	}
	else {
	    return false;
	}
    }

    public synchronized boolean isCellError(int row, int col) {
	return (isColumnEditable(col) && 
		(unavailableAttributes.containsKey(getValueName(row))));
    }
    
    public synchronized boolean isAttributeViewable(int row, int col) {
	boolean isViewable = false;
	if(col == VALUE_COLUMN) {
	    Object obj = getModel().getValueAt(row, col);
	    if(obj instanceof ZoomedCell)
		isViewable = true;
	}
	
	return isViewable;
    }
    
    public void loadAttributes(XMBean mbean, MBeanInfo mbeanInfo) {
	// To Avoid deadlock with events comming from the JDMK side, 
	// we retrieve 
	// all JDMK stuff in a non synchronized block
	
	if(mbean == null) return;

	final MBeanAttributeInfo[] attributesInfo = mbeanInfo.getAttributes();
	final HashMap<String, Object> attributes = 
	    new HashMap<String, Object>(attributesInfo.length);
	final HashMap<String, Object> unavailableAttributes =  
	    new HashMap<String, Object>(attributesInfo.length);
	final HashMap<String, Object> viewableAttributes = 
	    new HashMap<String, Object>(attributesInfo.length);
	final HashMap<String, Component> viewers = 
	    new HashMap<String, Component>(attributesInfo.length);
	AttributeList list = null;
	
	try {
	    list = mbean.getAttributes(attributesInfo);
	}catch(Exception e) {
	    list = new AttributeList();
	    //Can't load all attributes, do it one after each other.
	    for(int i = 0; i < attributesInfo.length; i++) {
		String name = null;
		try {
		    name = attributesInfo[i].getName();
		    Object value = 
			mbean.getAttribute(name);
		    list.add(new Attribute(name, value));
		}catch(Exception ex) {
		    if(attributesInfo[i].isReadable()) {
			unavailableAttributes.put(name, 
						  Utils.getActualException(ex).
						  toString());
		    }
		}
	    }
	}
	try {
	    int att_length = list.size();
	    for (int i=0;i<att_length;i++) {
		Attribute attribute = (Attribute) list.get(i);
		if(isViewable(attribute)) {
		    viewableAttributes.put(attribute.getName(),
					   attribute.getValue());
		}
		else
		    attributes.put(attribute.getName(),attribute.getValue());
		
	    }
	    // if not all attributes are accessible,
	    // check them one after the other.
	    if (att_length < attributesInfo.length) {
		for (int i=0;i<attributesInfo.length;i++) {
		    MBeanAttributeInfo attributeInfo = attributesInfo[i];
		    if (!attributes.containsKey(attributeInfo.getName()) &&
			!viewableAttributes.containsKey(attributeInfo.
							getName()) && 
			!unavailableAttributes.containsKey(attributeInfo.
							   getName())) {
			if (attributeInfo.isReadable()) {
			    // getAttributes didn't help resolving the 
			    // exception.
			    // We must call it again to understand what 
			    // went wrong.
			    try {
				Object v = 
				    mbean.getAttribute(attributeInfo.
						       getName());
				//What happens if now it is ok?
				// Be pragmatic, add it to readable...
				attributes.put(attributeInfo.getName(),
					       v);
			    }catch(Exception e) {
				//Put the exception that will be displayed
				// in tooltip
				unavailableAttributes.put(attributeInfo.
							  getName(),
							  Utils.
							  getActualException(e)
							  .toString());
			    }
			}
		    }
		}	
	    }	    
	}
	catch(Exception e) {
	    //sets all attributes unavailable except the writable ones
	    for (int i=0;i<attributesInfo.length;i++) {	
		MBeanAttributeInfo attributeInfo = attributesInfo[i];
		if (attributeInfo.isReadable()) {
		    unavailableAttributes.put(attributeInfo.getName(), 
					      Utils.getActualException(e).
					      toString());
		}
	    }
	}	
	//end of retrieval

	//one update at a time
	synchronized(this) {

	    this.mbean = mbean;
	    this.mbeanInfo = mbeanInfo;
	    this.attributesInfo = attributesInfo;
	    this.attributes = attributes;
	    this.unavailableAttributes = unavailableAttributes;
	    this.viewableAttributes = viewableAttributes;
			
	    if (isEditing()) {
		getCellEditor().cancelCellEditing();
	    }
	    DefaultTableModel tableModel = 
		(DefaultTableModel) getModel();
	    
	    // don't listen to these events
	    tableModel.removeTableModelListener(attributesListener);
	    
	    // add attribute information
	    emptyTable();
	    
	    addTableData(tableModel,
			 attributesInfo,
			 attributes,
			 unavailableAttributes,
			 viewableAttributes);
			
	    // update the model with the new data
	    tableModel.newDataAvailable(new TableModelEvent(tableModel));
	    // re-register for change events
	    tableModel.addTableModelListener(attributesListener);
	}
    }
    
    void collapse(String attributeName, final Component c) {
	final int row = getSelectedRow();
	Object obj = getModel().getValueAt(row, VALUE_COLUMN);
	if(obj instanceof ZoomedCell) {
	    ZoomedCell cell = (ZoomedCell) obj;
	    cell.reset();
	    setRowHeight(row, 
			 cell.getHeight());
	    editCellAt(row, 
		       VALUE_COLUMN);
	    invalidate();
	    repaint();
	}
    }
    
    ZoomedCell updateZoomedCell(int row,
				int col) {
	Object obj = getModel().getValueAt(row, VALUE_COLUMN);
	ZoomedCell cell = null;
	if(obj instanceof ZoomedCell) {
	    cell = (ZoomedCell) obj;
	    if(!cell.isInited()) {
		Object elem = cell.getValue();
		String attributeName = 
		    (String) getModel().getValueAt(row, 
						   NAME_COLUMN);
		Component comp = 
		    viewer.createAttributeViewer(elem, 
						 mbean, 
						 attributeName, 
						 this);
		if(comp != null){
		    if(rowMinHeight == -1)
			rowMinHeight = getRowHeight(row);
		    
		    cell.init(super.getCellRenderer(row, col),
			      comp,
			      rowMinHeight);
		    
		    viewer.registerForMouseEvent(comp, mouseListener);
		    
		} else
		    return cell;
	    } 
	    
	    cell.switchState();
	    setRowHeight(row, 
			 cell.getHeight());
	    
	    if(!cell.isMaximized()) {
		//Back to simple editor.
		editCellAt(row, 
			   VALUE_COLUMN);
	    }
	    
	    invalidate();
	    repaint();
	}
	return cell;
    }

     public void refreshAttributes() {
	 loadAttributes(mbean, mbeanInfo);
     }
	

     public void emptyTable() {
	 synchronized(this) {
	     ((DefaultTableModel) getModel()).
		 removeTableModelListener(attributesListener); 
	     super.emptyTable();
	 }
     }
    
    private boolean isViewable(Attribute attribute) {
	Object data = attribute.getValue();
	return XDataViewer.isViewableValue(data);
	
    }

    synchronized void removeAttributes() {
	attributes.clear();
	unavailableAttributes.clear();
	viewableAttributes.clear();
	mbean = null;
    }

    //will be called in a synchronzed block
    protected void addTableData(DefaultTableModel tableModel,
				MBeanAttributeInfo[] attributesInfo,
				HashMap<String, Object> attributes,
				HashMap<String, Object> unavailableAttributes,
				HashMap<String, Object> viewableAttributes) {
	    
	Object rowData[] = new Object[2];
	int col1Width = 0;
	int col2Width = 0;
	for (int i=0;i<attributesInfo.length;i++) {
	    rowData[0] = (attributesInfo[i].getName());	
	    if (unavailableAttributes.containsKey(rowData[0]))
		rowData[1] = Resources.getText("Unavailable");
	    else
		if(viewableAttributes.containsKey(rowData[0])) {
		    rowData[1] = viewableAttributes.get(rowData[0]);
		    rowData[1] = new ZoomedCell(rowData[1]);
		}
		else
		    rowData[1] = attributes.get(rowData[0]);
	    
	    tableModel.addRow(rowData);
	    
	    //Update column width
	    //
	    String str = null;
	    if(rowData[0] != null) {
		str = rowData[0].toString();
		if(str.length() > col1Width)
		    col1Width = str.length();
	    }
	    if(rowData[1] != null) {
		str = rowData[1].toString();
		if(str.length() > col2Width)
		    col2Width = str.length();
	    }
	}
	updateColumnWidth(col1Width, col2Width);
    }
    
    private void updateColumnWidth(int col1Width, int col2Width) {
	TableColumnModel colModel = getColumnModel();
	
	//Get the column at index pColumn, and set its preferred width.
	col1Width = col1Width * 7;
	col2Width = col2Width * 7;
	if(col1Width + col2Width < 
	   (int) getPreferredScrollableViewportSize().getWidth())
	    col2Width = (int) getPreferredScrollableViewportSize().getWidth() 
		- col1Width;
	
	colModel.getColumn(NAME_COLUMN).setPreferredWidth(50);
    }

    class AttributesMouseListener extends MouseAdapter {
	
	public void mousePressed(MouseEvent e) {
	    if(e.getButton() == MouseEvent.BUTTON1) {
		if(e.getClickCount() >= 2) {

		    int row = XMBeanAttributes.this.getSelectedRow();
		    int col = XMBeanAttributes.this.getSelectedColumn();
		    if(col != VALUE_COLUMN) return;
		    if(col == -1 || row == -1) return;
		    
		    XMBeanAttributes.this.updateZoomedCell(row, col);
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
	    if(column == VALUE_COLUMN) {		
		Object obj = getModel().getValueAt(row,
						   column);
		if(obj instanceof ZoomedCell) {
		    ZoomedCell cell = (ZoomedCell) obj;
		    if(cell.getRenderer() instanceof MaximizedCellRenderer) {
			MaximizedCellRenderer zr = 
			    (MaximizedCellRenderer) cell.getRenderer();
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
    
    class MaximizedCellRenderer extends  DefaultTableCellRenderer {
	Component comp;
	MaximizedCellRenderer(Component comp) {
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
    
    class ZoomedCell {
	TableCellRenderer minRenderer;
	MaximizedCellRenderer maxRenderer;
	int minHeight;
	boolean minimized = true;
	boolean init = false;
	int type;
	Object value;
	ZoomedCell(Object value) {
	    type = XDataViewer.getViewerType(value);
	    this.value = value;
	}
	
	boolean isInited() {
	    return init;
	}
	
	Object getValue() {
	    return value;
	}
	
	void init(TableCellRenderer minRenderer,
		  Component maxComponent, 
		  int minHeight) {
	    this.minRenderer = minRenderer;
	    this.maxRenderer = new MaximizedCellRenderer(maxComponent);
	    
	    this.minHeight = minHeight;
	    init = true;
	}
	
	int getType() {
	    return type;
	}
	
	void reset() {
	    init = false;
	    minimized = true;
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
	
	public String toString() {

	    if(value == null) return null;

	    if(value.getClass().isArray()) {
		String name = 
		    Utils.getArrayClassName(value.getClass().getName());
		int length = Array.getLength(value);
		return name + "[" + length +"]";
	    }
	    
	    if(value instanceof CompositeData ||
	       value instanceof TabularData)
		return value.getClass().getName();
	    
	    return value.toString();
	}
	
	TableCellRenderer getRenderer() {
	    if(minimized) return minRenderer;
	    else return maxRenderer;
	}
    }

    class AttributesListener implements  TableModelListener {

	private Component component;

	public AttributesListener(Component component) {
	    this.component = component;
	}

	public void tableChanged(final TableModelEvent e) {
	    final TableModel model = (TableModel)e.getSource();
	    // only post changes to the draggable column
	    if (isColumnEditable(e.getColumn())) {
		mbeansTab.workerAdd(new Runnable() {
			public void run() {
			    try {
				Object tableValue = 
				    model.getValueAt(e.getFirstRow(),
						     e.getColumn());
				// if it's a String, try construct new value 
				// using the defined type.
				if (tableValue instanceof String) {
				    tableValue = 
					Utils.createObjectFromString(getClassName(e.getFirstRow()), // type
								     (String)tableValue);// value
				}
				String attributeName = 
				    getValueName(e.getFirstRow());
				Attribute attribute = 
				    new Attribute(attributeName,tableValue);
				mbean.setAttribute(attribute);
			    }
			    catch (Throwable ex) {
				ex = Utils.getActualException(ex);
				
				String message = (ex.getMessage() != null) ? ex.getMessage() : ex.toString();
				EventQueue.invokeLater(new ThreadDialog(component, 
									message+"\n",
									Resources.getText("Problem setting attribute"), 
									JOptionPane.ERROR_MESSAGE));
			    }
			    refreshAttributes();
			}
		    });
	    }
	}
    }
}

    

