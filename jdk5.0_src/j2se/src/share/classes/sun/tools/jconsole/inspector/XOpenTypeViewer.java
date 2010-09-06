/*
 * @(#)XOpenTypeViewer.java	1.15 04/06/21
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
import javax.swing.border.*;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.FlowLayout;
import java.awt.Component;
import java.awt.EventQueue;
import java.awt.Color;
import java.awt.Font;
import java.awt.Rectangle;
import java.awt.event.*;
import java.awt.Insets;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.util.*;
import java.io.*;

import java.lang.reflect.Array;

// jaw import
import javax.management.*;
import javax.management.openmbean.*;

import sun.tools.jconsole.Resources;
import sun.tools.jconsole.BorderedComponent;
import sun.tools.jconsole.LabeledComponent;
import sun.tools.jconsole.VariableGridLayout;

public class XOpenTypeViewer extends JPanel implements ActionListener {
    JButton prev, incr, decr, tabularPrev, tabularNext;
    JLabel compositeLabel, tabularLabel;
    JScrollPane container;
    XOpenTypeData current;
    XOpenTypeDataListener listener = new XOpenTypeDataListener();
    
    class XOpenTypeDataListener extends MouseAdapter {
	XOpenTypeDataListener() {
	}
    
	public void mousePressed(MouseEvent e) {
	    if(e.getButton() == MouseEvent.BUTTON1) {
		if(e.getClickCount() >= 2) {
		    XOpenTypeData elem = getSelectedViewedOpenType();
		    if(elem != null) {
			try {
			    elem.viewed(XOpenTypeViewer.this);
			}catch(Exception ex) {
			    //Nothing to change, the element 
			    //can't be displayed
			}
		    }
		}
	    }
	}
	
	private XOpenTypeData getSelectedViewedOpenType() {
	    int row = XOpenTypeViewer.this.current.getSelectedRow();
	    int col = XOpenTypeViewer.this.current.getSelectedColumn();
	    Object elem = 
		XOpenTypeViewer.this.current.getModel().getValueAt(row, col);
	    if(elem instanceof XOpenTypeData)
		return (XOpenTypeData) elem;
	    else
		return null;
	}
    }

    static interface XViewedArrayOpenType {
	public void incrElement();
	public void decrElement();
	public boolean canDecrement();
	public boolean canIncrement();
    }
    
    static interface XViewedTabularOpenType {
	public void incrElement();
	public void decrElement();
	public boolean canDecrement();
	public boolean canIncrement();
    }

    static abstract class XOpenTypeData extends JTable {
	XOpenTypeData parent;
	private Color defaultColor;
	protected int col1Width = -1;
	protected int col2Width = -1;
	private boolean init;
	private Font normalFont, boldFont;
	protected XOpenTypeData(XOpenTypeData parent) {
	    this.parent = parent;
	    getTableHeader().setReorderingAllowed(false);
	}
    
	public XOpenTypeData getViewedParent() {
	    return parent;
	}
	public String getToolTip(int row, int col) {   
	    if(col == 1) {
		Object value = getModel().getValueAt(row, col);
		if (value != null) {
		    if(isClickableElement(value))
			return Resources.getText("Double click to visualize") 
			    + ". " + value.toString();
		    else
			return value.toString();
		}
		
	    }
	    return null;
	}
	
	public TableCellRenderer getCellRenderer(int row, int column) {
	    DefaultTableCellRenderer tcr = 
		(DefaultTableCellRenderer)super.getCellRenderer(row,column);
	    tcr.setToolTipText(getToolTip(row,column));
	    return tcr;
	}
	
	public void renderKey(String key,  Component comp) {
	    comp.setFont(normalFont);
	}
	
	public Component prepareRenderer(TableCellRenderer renderer,
					 int row, int column) {
	    Component comp = super.prepareRenderer(renderer, row, column);
	  
	    if (normalFont == null) {
		normalFont     = comp.getFont();
		boldFont       = normalFont.deriveFont(Font.BOLD);
	    }
	    
	    if(column == 0) {
		String key = 
		    (String) ((DefaultTableModel )getModel()).getValueAt(row, 
									 column);
		renderKey(key, comp);
	    } else {
		Object obj = ((DefaultTableModel)getModel()).
		    getValueAt(row, 
			       column);
		
		if(isClickableElement(obj))
		    comp.setFont(boldFont);
		else
		    comp.setFont(normalFont);
	    }
	    
	    return comp;
	}
	
	protected boolean isClickableElement(Object obj) {
	    if(obj instanceof XOpenTypeData)
		if(obj instanceof XArrayOpenType) {
		    if(((XArrayOpenType) obj).getNbElements() != 0)
			return true;
		} else
		    if(obj instanceof XTabularData)
			return true;
		    else
			if(obj instanceof XCompositeData)
			    return true;
	    return false;
	}
	
	protected void updateColumnWidth() {
	    if(!init) {
		TableColumnModel colModel = getColumnModel();
		if(col2Width == -1) {
		    col1Width = col1Width * 7;
		    if(col1Width < 
		       (int) getPreferredScrollableViewportSize().getWidth())
			col1Width = (int)
			    getPreferredScrollableViewportSize().getWidth();
		    colModel.getColumn(0).setPreferredWidth(col1Width);
		    return;
		}
	    
		//Get the column at index pColumn, and set its
		//preferred width.
		col1Width = (col1Width * 7) + 7;

		col1Width = Math.max(col1Width, 50);

		col2Width = (col2Width * 7) + 7;
		if(col1Width + col2Width < (int) 
		   getPreferredScrollableViewportSize().getWidth())
		    col2Width = (int) getPreferredScrollableViewportSize().
			getWidth() - col1Width;
	    
		colModel.getColumn(0).setPreferredWidth(col1Width);
		colModel.getColumn(1).setPreferredWidth(col2Width);
		init = true;
		}
	}

	public abstract void viewed(XOpenTypeViewer viewer) throws Exception;

	protected void initTable(String[] columnNames,
				 int scrollx,
				 int scrolly) {
	    setCellSelectionEnabled(true);
	    setRowSelectionAllowed(false);
	    setColumnSelectionAllowed(false);
	    ((DefaultTableModel)getModel()).setColumnIdentifiers(columnNames);
	
	    setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
	    // setPreferredSize(new Dimension(300, 
	    //			   200));
	    setPreferredScrollableViewportSize(new Dimension(350, 
							     200));
	}
    
	protected void initTable(String[] columns) {
	    initTable(columns, 450, 150);
	}
    
	protected void emptyTable() {
	    invalidate();
	    while (getModel().getRowCount()>0)
		((DefaultTableModel) getModel()).removeRow(0);
	    validate();
	}

	public boolean isCellEditable(int row, int col) {
	    return false;
	}
    }
    
    static class XTabularData extends XCompositeData 
	implements XViewedTabularOpenType { 
	TabularData tabular;
	TabularType type;
	int currentIndex = 0;
	Object[] elements;
	int size;
	private Font normalFont, italicFont;
	public XTabularData(XOpenTypeData parent,
			    TabularData tabular) {
	    super(parent, accessFirstElement(tabular));
	    elements = tabular.values().toArray();
	    size = elements.length;
	    this.tabular = tabular;
	    type = tabular.getTabularType();
	}
	
	private static CompositeData accessFirstElement(TabularData tabular) {
	    if(tabular.values().size() == 0) return null;
	    return (CompositeData) tabular.values().toArray()[0];
	}
	
	public void renderKey(String key,  Component comp) {
	    if (normalFont == null) {
		normalFont     = comp.getFont();
		italicFont       = normalFont.deriveFont(Font.ITALIC);
	    }
	    for(Object k : type.getIndexNames()) {
		if(key.equals(k))
		    comp.setFont(italicFont);
		
	    }
	}

	public void incrElement() {
	    currentIndex++;
	    loadCompositeData((CompositeData)elements[currentIndex]);
	}
	
	public void decrElement() {
	    currentIndex--;
	    loadCompositeData((CompositeData)elements[currentIndex]);	
	}
    
	public boolean canDecrement() {
	    if(currentIndex == 0)
		return false;
	    else 
		return true;
	}
    
	public boolean canIncrement(){
	    if(size == 0 ||
	       currentIndex == size -1)
		return false;
	    else 
		return true;
	}
	public String toString() {
	    return type.getDescription();
	}
    }
    
    static class XCompositeData extends XOpenTypeData {

	static final XCompositeData EMPTY_COMPOSITE= new XCompositeData();
	protected final String[] columnNames =  {Resources.getText("Name"),
						 Resources.getText("Value")};
	CompositeData composite;
    
	public XCompositeData() {
	    super(null);
	    initTable(columnNames);
	}
    
	//In sync with array, no init table.
	public XCompositeData(XOpenTypeData parent) {
	    super(parent);
	}

	public XCompositeData(XOpenTypeData parent,
			      CompositeData composite) {
	    super(parent);
	    initTable(columnNames);
	    if(composite != null) {
		this.composite = composite;
		loadCompositeData(composite);
	    }
	}
    
	public void viewed(XOpenTypeViewer viewer) throws Exception {
	    viewer.setOpenType(this);
	    updateColumnWidth();
	}
	
	
	public String toString() {
	    return getName();
	}

	public String getName() {
	    return composite.getCompositeType().getTypeName();
	}
    
	public String getDescription() {
	    return composite.getCompositeType().getDescription();
	}
    
	protected Object formatKey(String key) {
	    return key;
	}
	
	private void load(CompositeData data) {
	    CompositeType type = data.getCompositeType();
	
	    Set keys = type.keySet();
	    Iterator it = keys.iterator();
	    Object[] rowData = new Object[2];
	    while(it.hasNext()) {
		String key = (String) it.next();
		Object val = data.get(key);
		OpenType openType = type.getType(key);
		rowData[0] = formatKey(key);
		if(openType instanceof CompositeType) {
		    CompositeData c = (CompositeData) val;
		    CompositeType t = c.getCompositeType();
		    rowData[1] = new XCompositeData(this, c);
		} else 
		    if(openType instanceof ArrayType) {
			ArrayType arrayType = (ArrayType) openType;
			int dim = arrayType.getDimension();
			String elemType = 
			    arrayType.getElementOpenType().getTypeName();
			rowData[1] = new XArrayOpenType(this,
							arrayType,
							val);
		    
		    } else 
			if(openType instanceof SimpleType) {
			    rowData[1] = val;
			} else 
			    if(openType instanceof TabularType) {
				rowData[1] = 
				    new XTabularData(this, (TabularData) val);
			    }
		//Update column width	    
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
		((DefaultTableModel) getModel()).addRow(rowData);
	    }
	}
    
	protected void loadCompositeData(CompositeData data) {
	    composite = data;
	    emptyTable();
	    load(data);
	    DefaultTableModel tableModel = (DefaultTableModel) getModel();
	
	    tableModel.newDataAvailable(new TableModelEvent(tableModel));
	    return;
	}
    }
    static class XArrayOpenType extends XCompositeData 
	implements XViewedArrayOpenType {
	private String name;
	private ArrayType type;
	private int dimension;
	private int size;
	private OpenType elemType;
	private Object val;
	private int currentIndex;
	private CompositeData[] elements;
	private final String[] arrayColumns = {Resources.getText("Value")};
	XArrayOpenType(XOpenTypeData parent,
		       ArrayType type,
		       Object val) {
	    super(parent);
	    this.type = type;
	    this.val = val;
	    String[] columns = null;
	    dimension = type.getDimension();
	    if(dimension > 1) return;
	
	    elemType = type.getElementOpenType();
	
	    if(elemType instanceof CompositeType) 
		columns = columnNames;
	    else
		columns = arrayColumns;
	
	    initTable(columns);
	    loadArray();
	}
	public void viewed(XOpenTypeViewer viewer) throws Exception {
	    if(size == 0)
		throw new Exception(Resources.getText("Empty array"));
	    if(dimension > 1)
		throw new Exception(Resources.getText("Dimension is not " + 
						      "supported:") + 
				    dimension);
	    super.viewed(viewer);
	}
    
	public int getNbElements() {
	    return size;
	}

	public void incrElement() {
	    currentIndex++;
	    loadCompositeData(elements[currentIndex]);
	}
    
	public void decrElement() {
	    currentIndex--;
	    loadCompositeData(elements[currentIndex]);	
	}
    
	public boolean canDecrement() {
	    if(currentIndex == 0)
		return false;
	    else 
		return true;
	}
    
	public boolean canIncrement(){
	    if(currentIndex == size -1)
		return false;
	    else 
		return true;
	}
    
	private void loadArray() {
	    if(elemType instanceof CompositeType) { 
		elements = (CompositeData[]) val;
		size = elements.length;
		if(size != 0)
		    loadCompositeData(elements[0]);
	    } else
		if(elemType instanceof SimpleType)
		    load();
	}
    
	public String getName() {
	    return elements[currentIndex].getCompositeType().getTypeName() +
		"[" + currentIndex + "]";
	}
    
	public String getDescription() {
	    return elements[currentIndex].getCompositeType().getDescription();
	}

	private void load() {
	    Object[] rowData = new Object[1];
	    size = Array.getLength(val);
	    for(int i = 0; i < size; i++) {
		rowData[0] = Array.get(val, i);
		String str = rowData[0].toString();
		if(str.length() > col1Width)
		    col1Width = str.length();
		((DefaultTableModel) getModel()).addRow(rowData);
	    }
	}

    
	public String toString() {
	    if(dimension > 1)
		return Resources.getText("Dimension is not supported:") + 
		    dimension;
	    else
		return elemType.getTypeName()+"[" + size + "]";
	
	}
    }
    
    public static boolean isViewableValue(Object value) {
	return (value instanceof CompositeData) ||
	       (value instanceof TabularData);
    }
    
    public static Component loadOpenType(Object value) {
	Component comp = null;
	if(isViewableValue(value)) {
	    XOpenTypeViewer open = 
		new XOpenTypeViewer(value);
	    comp = open;
	}
	return comp;
    }
    
    private XOpenTypeViewer(Object value) {
	XOpenTypeData comp = null;
	if(value instanceof CompositeData) {
	    CompositeData composite = (CompositeData) value;
	    comp = new XCompositeData(null,
				      composite);
	} else
	    if(value instanceof TabularData) {
		comp = new XTabularData(null,
					(TabularData) value);
	    }
	setupDisplay(comp);
	try {
	    comp.viewed(this);
	}catch(Exception e) {
	    //Nothing to change, the element can't be displayed
	    System.out.println("Exception viewing openType : " + e);
	}
    }

    void setOpenType(XOpenTypeData data) {
	if(current != null)
	    current.removeMouseListener(listener);
	
	current = data;
	
	if(current.getViewedParent() == null) {
	    prev.setEnabled(false);
	    compositeLabel.setEnabled(false);
	}
	else {
	    prev.setEnabled(true);
	    compositeLabel.setEnabled(true);
	}

	//Check if it is an arry
	// TODO
	
	//Set the listener to handle dbl click
	current.addMouseListener(listener);

	if(!(data instanceof XViewedTabularOpenType)) {
	    tabularPrev.setEnabled(false);
	    tabularNext.setEnabled(false);
	    tabularLabel.setEnabled(false);
	} else {
	   
	    XViewedTabularOpenType tabular = (XViewedTabularOpenType) data;
	    if(tabular.canIncrement()) {
		tabularNext.setEnabled(true);
		 tabularLabel.setEnabled(true);
	    } else
		tabularNext.setEnabled(false);
	    
	    if(tabular.canDecrement()) {
		tabularPrev.setEnabled(true);
		tabularLabel.setEnabled(true);
	    }else
		tabularPrev.setEnabled(false);

	    if(!tabular.canIncrement() &&
	       !tabular.canDecrement())
		tabularLabel.setEnabled(false);
	}
	//Handle array buttons
	if(!(data instanceof XViewedArrayOpenType)) {
	    incr.setEnabled(false);
	    decr.setEnabled(false);
	} else {
	    XViewedArrayOpenType array = (XViewedArrayOpenType)  data;
	    if(array.canIncrement())
		incr.setEnabled(true);
	    else
		incr.setEnabled(false);
	    
	    if(array.canDecrement())
		decr.setEnabled(true);
	    else
		decr.setEnabled(false);
	}

	container.invalidate();
	container.setViewportView(current);
	container.validate();
    }

    public void actionPerformed(ActionEvent event) {
	if (event.getSource() instanceof JButton) {
	    JButton b = (JButton) event.getSource();
	    
	    if (b == prev) {
		
		XOpenTypeData parent = current.getViewedParent();
		try {
		    parent.viewed(this);
		}catch(Exception e) {
		    //Nothing to change, the element can't be displayed
		}
	    } else
		if(b == incr) {
		    ((XViewedArrayOpenType)current).incrElement();
		    try {
			current.viewed(this);
		    }catch(Exception e) {
			//Nothing to change, the element can't be displayed
		    }
		} else
		    if(b == decr) {
			((XViewedArrayOpenType)current).decrElement();
			try {
			    current.viewed(this);
			}catch(Exception e) {
			    //Nothing to change, the element can't be displayed
			}
		    } else
			if(b == tabularNext) {
			    ((XViewedTabularOpenType)current).incrElement();
			    try {
				current.viewed(this);
			    }catch(Exception e) {
				//Nothing to change, the element can't be displayed
			    }
			} else
			    if(b == tabularPrev) {
				((XViewedTabularOpenType)current).decrElement();
				try {
				    current.viewed(this);
				}catch(Exception e) {
				//Nothing to change, the element can't be displayed
				}
			    }
	}
    }

    private void setupDisplay(XOpenTypeData data) {
	setBackground(Color.white);
	container = 
	    new JScrollPane(data, 
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
			    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	
	JPanel buttons = new JPanel(new FlowLayout(FlowLayout.LEFT));
	tabularPrev =  new JButton(Resources.getText("<"));
	tabularNext =  new JButton(Resources.getText(">"));
	JPanel tabularButtons = new JPanel(new FlowLayout(FlowLayout.LEFT));
	tabularButtons.add(tabularPrev);
	tabularPrev.addActionListener(this);
	tabularButtons.add(tabularLabel = new JLabel(Resources.getText("Tabular Navigation")));
	tabularButtons.add(tabularNext);
	tabularNext.addActionListener(this);
	tabularButtons.setBackground(Color.white);

	prev = new JButton(Resources.getText("<<"));
	prev.addActionListener(this);
	buttons.add(prev);
	buttons.add(compositeLabel = new JLabel(Resources.getText("Composite Navigation")));
	
	incr = new JButton(Resources.getText(">"));
	incr.addActionListener(this);
	decr = new JButton(Resources.getText("<"));
	decr.addActionListener(this);
	
	JPanel array = new JPanel();
	array.setBackground(Color.white);
	array.add(decr);
	array.add(incr);
	
	buttons.add(array);
	setLayout(new BorderLayout());
	buttons.setBackground(Color.white);

	JPanel navigationPanel = new JPanel(new BorderLayout());
	navigationPanel.setBackground(Color.white);
	navigationPanel.add(tabularButtons, BorderLayout.NORTH);
	navigationPanel.add(buttons, BorderLayout.WEST);
	add(navigationPanel, BorderLayout.NORTH);
	
	add(container, BorderLayout.WEST);
	Dimension d = new Dimension((int)container.getPreferredSize().
				    getWidth() + 20, 
				    (int)container.getPreferredSize().
				    getHeight() + 20);
	setPreferredSize(d);
    }
}
