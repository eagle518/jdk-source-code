/*
 * @(#)XTextFieldEditor.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import java.awt.Component;
import java.util.EventObject;
import java.awt.event.*;
import java.awt.dnd.DragSourceDropEvent;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;

public class XTextFieldEditor extends XTextField implements TableCellEditor {

    protected EventListenerList listenerList = new EventListenerList();
    protected ChangeEvent changeEvent = new ChangeEvent(this);

    //edition stopped ou JMenuItem selection & JTextField selection
    public void  actionPerformed(ActionEvent e) {
	super.actionPerformed(e);
	if ((e.getSource() instanceof JMenuItem) || 
	    (e.getSource() instanceof JTextField)) {
	    fireEditingStopped();
	}
    }

    //edition stopped on drag & drop success
    protected void dropSuccess() {
	fireEditingStopped();
    }
	
    //TableCellEditor implementation

    public void addCellEditorListener(CellEditorListener listener) {
	listenerList.add(CellEditorListener.class,listener);
    }

    public void removeCellEditorListener(CellEditorListener listener) {
	listenerList.remove(CellEditorListener.class, listener);
    }

    protected void fireEditingStopped() {
	CellEditorListener listener;
	Object[] listeners = listenerList.getListenerList();
	for (int i=0;i< listeners.length;i++) {
	    if (listeners[i] == CellEditorListener.class) {
		listener = (CellEditorListener) listeners[i+1];
		listener.editingStopped(changeEvent);
	    }
	}
    }

    protected void fireEditingCanceled() {
	CellEditorListener listener;
	Object[] listeners = listenerList.getListenerList();
	for (int i=0;i< listeners.length;i++) {
	    if (listeners[i] == CellEditorListener.class) {
		listener = (CellEditorListener) listeners[i+1];
		listener.editingCanceled(changeEvent);
	    }
	}
    }

    public void cancelCellEditing() {
	fireEditingCanceled();
    }

    public boolean stopCellEditing() {
	cancelCellEditing();
	return true;
    }

    public boolean isCellEditable(EventObject event) {
	return true;
    }

    public boolean shouldSelectCell(EventObject event) {
	return true;
    }

    public Object getCellEditorValue() {
	Object object = getValue();
	
	if (object instanceof XObject) {
	    return ((XObject) object).getObject();
	}
	else {
	    return object;
	}
    }

    public Component getTableCellEditorComponent(JTable table, 
						 Object value, 
						 boolean isSelected,
						 int row,
						 int column) {
	XTable mytable = (XTable) table;
	String className = mytable.getClassName(row);
	try {
	    init(value,Utils.getClass(className));
	}
	catch(Exception e) {}
	
	return this;
    }

}
	
	

    
	

    

