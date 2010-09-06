/*
 * @(#)TableSorter.java	1.7 04/05/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import java.util.*;
import java.awt.event.*;
import javax.swing.table.*;
import javax.swing.event.*; 

// Imports for picking up mouse events from the JTable. 

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.InputEvent;
import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumnModel;

public class TableSorter extends DefaultTableModel implements MouseListener {
    private boolean ascending = true;
    private TableColumnModel columnModel;
    private JTable tableView;
    private Vector<TableModelListener> listenerList;
    private int sortColumn = 0;
    
    private int[] invertedIndex;
	
    public TableSorter() {
	super();
	listenerList = new Vector<TableModelListener>();
    }
	
    public TableSorter(Object[] columnNames, int numRows) {
	super(columnNames,numRows);
	listenerList = new Vector<TableModelListener>();
    }
	
    public void newDataAvailable(TableModelEvent e) {
	super.newDataAvailable(e);
	invertedIndex = new int[getRowCount()];
	for (int i=0;i<invertedIndex.length;i++) {
	    invertedIndex[i] = i;
	}
	sort(this.sortColumn);
    }
	
    public void addTableModelListener(TableModelListener l) {
	listenerList.add(l);
	super.addTableModelListener(l);
    }
	
    public void removeTableModelListener(TableModelListener l) {
	listenerList.remove(l);
	super.removeTableModelListener(l);
    }
	
    private void removeListeners() {
	for(TableModelListener tnl : listenerList)
	    super.removeTableModelListener(tnl);
    }
	
    private void restoreListeners() {
	for(TableModelListener tnl : listenerList)
	    super.addTableModelListener(tnl);
    }
    
    public int compare(Object o1, Object o2) {
	if (o1==null)
	    return 1;
	if (o2==null)
	    return -1;
	//two object of the same class and that are comparable
	else if ((o1.getClass().equals(o2.getClass())) && 
		 (o1 instanceof Comparable)) {
	    return (((Comparable) o1).compareTo(o2));
	}
	else {
	    return o1.toString().compareTo(o2.toString());
	}
    }

    public void sort(int column) {
	// remove registered listeners
	removeListeners();
	// do the sort
	//n2sort(column);
	quickSort(0,getRowCount()-1,column);
	// restore registered listeners
	restoreListeners();
	this.sortColumn = column;
    }
	
    private synchronized boolean compareS(Object s1, Object s2) {
	if (ascending)
	    return (compare(s1,s2) > 0);
	else
	    return (compare(s1,s2) < 0);
    }
  
    private synchronized boolean compareG(Object s1, Object s2) {
	if (ascending)
	    return (compare(s1,s2) < 0);
	else
	    return (compare(s1,s2) > 0);
    }
	
    private synchronized void quickSort(int lo0,int hi0, int key) {
	int lo = lo0;
	int hi = hi0;
	Object mid;
		
	if ( hi0 > lo0)
	    {
		mid = getValueAt( ( lo0 + hi0 ) / 2 , key);
				
		while( lo <= hi )
		    {
			/* find the first element that is greater than 
			 * or equal to the partition element starting 
			 * from the left Index.
			 */
			while( ( lo < hi0 ) && 
			       ( compareS(mid,getValueAt(lo,key)) ))
			    ++lo;
						
			/* find an element that is smaller than or equal to
			 * the partition element starting from the right Index.
			 */
			while( ( hi > lo0 ) && 
			       ( compareG(mid,getValueAt(hi,key)) ))
			    --hi;
						
			// if the indexes have not crossed, swap
			if( lo <= hi )
			    {
				swap(lo, hi, key);
				++lo;
				--hi;
			    }
		    }
				
				/* If the right index has not reached the 
				 * left side of array
				 * must now sort the left partition.
				 */
		if( lo0 < hi )
		    quickSort(lo0, hi , key);
				
				/* If the left index has not reached the right
				 * side of array
				 * must now sort the right partition.
				 */
		if( lo <= hi0 )
		    quickSort(lo, hi0 , key);
	    }
    }
	
    public void n2sort(int column) {
        for (int i = 0; i < getRowCount(); i++) {
            for (int j = i+1; j < getRowCount(); j++) {
                if (compare(getValueAt(i,column),getValueAt(j,column)) == -1) {
                    swap(i, j, column);
                }
            }
        }
    }
	
    private Vector getRow(int row) {
	return (Vector) dataVector.elementAt(row);
    }
	
    private void setRow(Vector data, int row) {
	dataVector.setElementAt(data,row);
	if(tableView instanceof XMBeanAttributes)
	    ((XMBeanAttributes)tableView).
		updateRowHeight(data.elementAt(1), row);
    }
	
    public void swap(int i, int j, int column) {
	Vector data = getRow(i);
	setRow(getRow(j),i);
	setRow(data,j);

	int a = invertedIndex[i];
	invertedIndex[i] = invertedIndex[j];
	invertedIndex[j] = a;
    }

    public void sortByColumn(int column) {
        sortByColumn(column, !ascending);
    }

    public void sortByColumn(int column, boolean ascending) {
        this.ascending = ascending;
        sort(column);
    }
    
    public int[] getInvertedIndex() {
	return invertedIndex;
    }
    
    // Add a mouse listener to the Table to trigger a table sort 
    // when a column heading is clicked in the JTable. 
    public void addMouseListenerToHeaderInTable(JTable table) { 
	tableView = table;
	columnModel = tableView.getColumnModel();
        JTableHeader th = tableView.getTableHeader(); 
        th.addMouseListener(this); 
    }
    
    public void mouseClicked(MouseEvent e) {
	int viewColumn = columnModel.getColumnIndexAtX(e.getX()); 
	int column = tableView.convertColumnIndexToModel(viewColumn); 
	if (e.getClickCount() == 1 && column != -1) {
	    tableView.invalidate();
	    sortByColumn(column); 
	    tableView.validate();
	}
    }
    
    public void mousePressed(MouseEvent e) {
    }
    
    public void mouseEntered(MouseEvent e) {	
    }
    
    public void mouseExited(MouseEvent e) {
    }
    
    public void mouseReleased(MouseEvent e) {
    }   
}
