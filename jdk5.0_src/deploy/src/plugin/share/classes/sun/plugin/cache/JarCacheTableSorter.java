/*
 * @(#)JarCacheTableSorter.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

import java.util.*;
import javax.swing.table.TableModel;
import javax.swing.event.TableModelEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.InputEvent;
import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

public class JarCacheTableSorter extends JarCacheTableModel{
    int selectedColumn = -1;
    boolean ascending = true;

    public JarCacheTableSorter(String [] names) {
	super(names);
    }

    public int compareRowsByColumn(JarCacheEntry row1, JarCacheEntry row2) {
        // Check for nulls.
        Object obj1 = row1.getValue(super.getColumnName(selectedColumn));
        Object obj2 = row2.getValue(super.getColumnName(selectedColumn));

        if (obj1 instanceof java.lang.Number) {
            Number n1 = (Number)obj1;
            double d1 = n1.doubleValue();
            Number n2 = (Number)obj2;
            double d2 = n2.doubleValue();

            if (d1 < d2) {
                return -1;
            } else if (d1 > d2) {
                return 1;
            } else {
                return 0;
            }
        } else if (obj1 instanceof java.util.Date) {
            Date d1 = (Date)obj1;
            long n1 = d1.getTime();
            Date d2 = (Date)obj2;
            long n2 = d2.getTime();

            if (n1 < n2) {
                return -1;
            } else if (n1 > n2) {
                return 1;
            } else {
                return 0;
            }
        } else if (obj1 instanceof String) {
            String s1 = (String)obj1;
            String s2    = (String)obj2;
            int result = s1.compareToIgnoreCase(s2);

            if (result < 0) {
                return -1;
            } else if (result > 0) {
                return 1;
            } else {
                return 0;
            }
        } else if (obj1 instanceof Boolean) {
            Boolean bool1 = (Boolean)obj1;
            boolean b1 = bool1.booleanValue();
            Boolean bool2 = (Boolean)obj2;
            boolean b2 = bool2.booleanValue();

            if (b1 == b2) {
                return 0;
            } else if (b1) { // Define false < true
                return 1;
            } else {
                return -1;
            }
        } else {
            String s1 = obj1.toString();
            String s2 = obj2.toString();
            int result = s1.compareToIgnoreCase(s2);

            if (result < 0) {
                return -1;
            } else if (result > 0) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    public void sort() {
	if(selectedColumn != -1) {
	    Arrays.sort(super.tableRows, new Comparator() {
		public int compare(Object o1, Object o2) {
		    int result = compareRowsByColumn((JarCacheEntry)o1, (JarCacheEntry)o2);
		    return ( result != 0 && ascending )? result : -result;
		}
	    });
	}

	//inform the JTable that the table data has changed
	super.fireTableDataChanged();
    }

    public void sortByColumn(int column) {
	if(column != selectedColumn) {
	    ascending = true;
	}else {
	    ascending = !ascending;
	}
	selectedColumn = column;
        sort();
    }


    public void refresh() {
	super.refresh();
	sort();
    }

    public void removeRows(int[] rows) {
	super.removeRows(rows);
	sort();
    }

    // There is no-where else to put this. 
    // Add a mouse listener to the Table to trigger a table sort 
    // when a column heading is clicked in the JTable. 
    public void addMouseListenerToHeaderInTable(JTable table) { 
        final JarCacheTableSorter sorter = this; 
        final JTable tableView = table; 
        tableView.setColumnSelectionAllowed(false); 
        MouseAdapter listMouseListener = new MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                TableColumnModel columnModel = tableView.getColumnModel();
                int viewColumn = columnModel.getColumnIndexAtX(e.getX()); 
                int column = tableView.convertColumnIndexToModel(viewColumn);		
                if (e.getClickCount() == 1 && column != -1) {
                    sorter.sortByColumn(column); 
                }
            }
        };
        JTableHeader th = tableView.getTableHeader(); 
        th.addMouseListener(listMouseListener); 
    }
}
