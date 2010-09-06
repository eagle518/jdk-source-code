/*
 * @(#)CacheTable.java	1.10 04/03/09
 * 
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Date;
import java.io.File;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import javax.swing.border.*;

import com.sun.javaws.cache.Cache;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

class CacheTable extends JTable {

    private static final TableCellRenderer _defaultRenderer = 
					   new DefaultTableCellRenderer();

    private static final int MIN_ROW_HEIGHT = 36;

    private boolean _system;

    private int _filter = 0;
    // private static long t0, t1, t2, t3, t4;

    public CacheTable(final CacheViewer cv, boolean bSystem) {

	_system = bSystem;
	setShowGrid(false);
	setIntercellSpacing(new Dimension(0,0));
	setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
	int height = getRowHeight();
	if (height < MIN_ROW_HEIGHT) {
	    setRowHeight(MIN_ROW_HEIGHT);
	}

	setPreferredScrollableViewportSize(new Dimension(640, 280));
	addMouseListener(new MouseAdapter() {
	    public void mousePressed(MouseEvent e) {
		if (e.isPopupTrigger()) {
		    int y = e.getY();
		    int row = y / getRowHeight();

		    getSelectionModel().clearSelection();
		    getSelectionModel().addSelectionInterval(row, row);

		    // popup application popup menu
		    cv.popupApplicationMenu(CacheTable.this, e.getX(), y);
		}
	    }

	    public void mouseReleased(MouseEvent e) {
		if (e.isPopupTrigger()) {
		    int y = e.getY();
		    int row = y / getRowHeight();

		    getSelectionModel().clearSelection();
		    getSelectionModel().addSelectionInterval(row, row);

		    // popup application popup menu
		    cv.popupApplicationMenu(CacheTable.this, e.getX(), y);
		}
	    }

	    public void mouseClicked(MouseEvent e) {
		Point p = e.getPoint();
		if (e.getClickCount() == 2) {
		    if (e.getButton() == MouseEvent.BUTTON1) {
			// double click
			int column = 
			    getColumnModel().getColumnIndexAtX(p.x);
			if (column < 3) {
			    cv.launchApplication();
			}
		    }
		}
	    }
	});

	reset();
    }

    public void setFilter(int filter) {
	if (filter != _filter) {
	   _filter = filter;
	    reset();
	}
    }

    public void reset() {
	TableModel old = getModel();
	if (old instanceof CacheTableModel) {
	    ((CacheTableModel) old).removeMouseListenerFromHeaderInTable(this);
	}
	CacheTableModel ctm = new CacheTableModel(_system, _filter);
	setModel(ctm);
	for (int i=0; i< getModel().getColumnCount(); i++) {
	    TableColumn column = getColumnModel().getColumn(i);
	    column.setHeaderRenderer(new CacheTableHeaderRenderer());
	    int width = ctm.getPreferredWidth(i);
	    column.setPreferredWidth(width);
	    column.setMinWidth(width);
	}
	
        setDefaultRenderer(JLabel.class, ctm);
	ctm.addMouseListenerToHeaderInTable(this);
    }

    public CacheObject getCacheObject(int row) {
	return ((CacheTableModel) getModel()).getCacheObject(row);
    }

    public String [] getAllHrefs() {
	ArrayList al = new ArrayList();
	TableModel model = getModel();
	if (model instanceof CacheTableModel) {
	    for (int i=0; i<model.getRowCount(); i++) {
		String href = ((CacheTableModel)model).getRowHref(i);
		if (href != null) {
		    al.add(href);
		}
	    }
	}
	return (String []) al.toArray(new String[0]);
    }

    private class CacheTableHeaderRenderer extends DefaultTableCellRenderer {

        public Component getTableCellRendererComponent(JTable table,
                                                       Object value,
                                                       boolean isSelected,
                                                       boolean hasFocus,
                                                       int row,
                                                       int column) {

            if (table != null) {
                JTableHeader header = table.getTableHeader();
                if (header != null) {
                    setForeground(header.getForeground());
                    setBackground(header.getBackground());
                    setFont(header.getFont());
                }
            }

            setText((value == null) ? "" : value.toString());
            setBorder(UIManager.getBorder("TableHeader.cellBorder"));
	    setHorizontalAlignment(JLabel.CENTER);
	    String tooltip = CacheObject.getHeaderToolTipText(column);
	    if (tooltip != null && tooltip.length() > 0) {
		setToolTipText(tooltip);
	    }
            return this;
        }
    }

    private class CacheTableModel extends AbstractTableModel implements 
		TableCellRenderer {
        private boolean _system;
        private CacheObject [] _rows;
	private int _filter;
	private int _sortColumn;
	private boolean _sortAscending;
	private MouseListener _mouseListener = null;
        
        public CacheTableModel(boolean bSystem, int filter) {
            _system = bSystem;
	    _filter = filter;
            _rows = new CacheObject[0];
	    _sortColumn = -1;
	    _sortAscending = true;
	    refresh();
            fireTableDataChanged();
        }   
     
        public Component getTableCellRendererComponent(JTable table, 
	    	Object value, boolean isSelected, boolean hasFocus, 
	    	int row, int column) {
 
            if (value instanceof Component) {
                Component c = (Component) value;
		if (isSelected) {
                    c.setForeground(table.getSelectionForeground());
                    c.setBackground(table.getSelectionBackground());
		} else {
                    c.setForeground(table.getForeground());
                    c.setBackground(table.getBackground());
		}
	    	CacheObject.hasFocus(c, hasFocus);
                return c;
            }
            return _defaultRenderer.getTableCellRendererComponent(table, value,
                isSelected, hasFocus, row, column);
        }

        public void refresh() {
            ArrayList al = new ArrayList();
            Iterator it = Cache.getJnlpCacheEntries(_system);
	    while (it.hasNext()) {
		CacheObject o = new CacheObject(
			(DiskCacheEntry) it.next(), this);
		if (o.inFilter(_filter) && (o.getLaunchDesc() != null)) {
		    al.add(o);
		}
	    }
            _rows = (CacheObject []) al.toArray(new CacheObject[0]);
	    if (_sortColumn != -1) {
	        sort();
	    }
        }   

        CacheObject getCacheObject(int row) {
	    return _rows[row];
	}
        
        public Object getValueAt(int row, int column) {
            return _rows[row].getObject(column);
        }   
     
        public int getRowCount() {
            return _rows.length;
        }   

	public String getRowHref(int row) {
	    return _rows[row].getHref();
	}
     
        public int getColumnCount() {
            return CacheObject.getColumnCount();
        }   
       
        public boolean isCellEditable(int row, int column) {
	    return _rows[row].isEditable(column);
        }
        
        public Class getColumnClass(int column) {
            return CacheObject.getClass(column);
        }
     
        public String getColumnName(int column) {
            return CacheObject.getColumnName(column);
        }

        public void setValueAt(Object value, int row, int col) {
	    _rows[row].setValue(col, value);
        }
     
        public int getPreferredWidth(int column) {
            return CacheObject.getPreferredWidth(column);
        }

	public void removeMouseListenerFromHeaderInTable(JTable table) {
	    if (_mouseListener != null) {
          	table.getTableHeader().removeMouseListener(_mouseListener);
	    }
	}

        public void addMouseListenerToHeaderInTable(JTable table) { 
            final JTable tableView = table; 
            tableView.setColumnSelectionAllowed(false); 
	    final ListSelectionModel lsm = tableView.getSelectionModel();
            _mouseListener = new MouseAdapter() {
                public void mouseClicked(MouseEvent e) {
                    TableColumnModel columnModel = tableView.getColumnModel();
                    int viewColumn = columnModel.getColumnIndexAtX(e.getX()); 
		    int selected = lsm.getMinSelectionIndex();
		    lsm.clearSelection();
                    int col = tableView.convertColumnIndexToModel(viewColumn); 
                    if (e.getClickCount() == 1 && col >= 0) {
                        int shiftPressed = 
			    e.getModifiers() & InputEvent.SHIFT_MASK; 
                        _sortAscending = (shiftPressed == 0); 
			_sortColumn = col;
			runSort(lsm, selected);
                    }
                }
            };
            tableView.getTableHeader().addMouseListener(_mouseListener); 
        }

	public void sort () {

	    boolean needs_repaint = false;

	    if (_sortAscending) {
                for (int i = 0; i < getRowCount(); i++) {
                    for (int j = i+1; j < getRowCount(); j++) {
                        if (_rows[i].compareColumns(_rows[j], _sortColumn) > 0) 
			{
			    needs_repaint=true;
			    CacheObject tmp = _rows[i];
			    _rows[i] = _rows[j];
			    _rows[j] = tmp;
                        }
                    }
                }
	    } else {
                for (int i = 0; i < getRowCount(); i++) {
                    for (int j = i+1; j < getRowCount(); j++) {
                        if (_rows[j].compareColumns(_rows[i], _sortColumn) > 0) 
			{
			    needs_repaint=true;
			    CacheObject tmp = _rows[i];
			    _rows[i] = _rows[j];
			    _rows[j] = tmp;
                        }
                    }
                }
	    }
	    if (needs_repaint) {
                fireTableDataChanged();
	    }
        }

	private void runSort(final ListSelectionModel lsm, final int selected) {
	    if (CacheViewer.getStatus() != CacheViewer.STATUS_SORTING) {
                new Thread(new Runnable() {
                    public void run() {
			CacheViewer.setStatus(CacheViewer.STATUS_SORTING);
			try {
			    CacheObject sel = null;
			    if (selected >= 0) {
				sel = _rows[selected];
			    }
		            sort(); 
			    if (sel != null) {
				for (int i=0; i<_rows.length; i++) {
				    if (_rows[i] == sel) {
				        lsm.addSelectionInterval(i, i);
				        break;
				    }
				}
			    }
			} finally {
			    CacheViewer.setStatus(CacheViewer.STATUS_OK);
			}
                    }
                }).start();
	    }
	}
    }
}


