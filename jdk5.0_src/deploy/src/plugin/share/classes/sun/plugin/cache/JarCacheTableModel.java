/*
 * @(#)JarCacheTableModel.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.util.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.text.MessageFormat;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import sun.plugin.resources.ResourceHandler;
import com.sun.deploy.util.DialogFactory;

public class JarCacheTableModel extends AbstractTableModel {
    private LinkedList list;
    protected Object [] tableRows;
    private String [] columnNames;

    public JarCacheTableModel(String [] names) {
	list = JarCacheEntry.getEntries();
	tableRows = list.toArray();
	columnNames = names;
    }
    
    public int getRowCount() { 
	return list.size(); 
    }
    
    public int getColumnCount() { 
	return columnNames.length; 
    }
    
    public String getColumnName(int col) {
      if(col < columnNames.length)
	return columnNames[col];
      else
	return null;
    }

    public void removeRows(int[] rows) {
	if(rows.length > 0) {
	    for(int i=0;i<rows.length;i++) {
		JarCacheEntry entry = (JarCacheEntry)tableRows[rows[i]];
		if(entry.delete() == true) {
		    int index = list.indexOf(tableRows[rows[i]]);
		    list.remove(index);
		} else {
		    String msg = ResourceHandler.getMessage("cache_viewer.delete.text");
		    String title = ResourceHandler.getMessage("cache_viewer.delete.caption");
		    MessageFormat formatter = new MessageFormat(msg);
		    msg = formatter.format(new Object[] {entry.getName()});
		    DialogFactory.showErrorDialog(msg, title);
		}
	    }
	    //Update the array aswell
	    tableRows = list.toArray();
	}
    }

    public void refresh() {
	list = JarCacheEntry.getEntries();
	tableRows = list.toArray();
    }

    //check boundaries for rows/columns
    public Object getValueAt(int row, int col) {
	Object retVal = null;
	if(row < getRowCount() && col < getColumnCount()) {
	    JarCacheEntry entry = (JarCacheEntry)tableRows[row];
	    retVal = entry.getDisplayValue(columnNames[col]);
	}
	return retVal;
    }
}



