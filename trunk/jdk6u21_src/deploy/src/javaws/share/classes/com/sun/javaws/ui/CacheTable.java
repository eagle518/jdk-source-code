/*
 * @(#)CacheTable.java	1.29 10/03/24
 * 
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import com.sun.javaws.jnl.LaunchDesc;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Date;
import java.util.Properties;
import java.util.Enumeration;
import java.io.File;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import javax.swing.border.*;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.resources.ResourceManager;

class CacheTable extends JTable {

    private static final TableCellRenderer defaultRenderer = 
                                           new DefaultTableCellRenderer();

    static final int JNLP_ROW_HEIGHT = 36;
    static final int RESOURCE_ROW_HEIGHT = 26;
    static final int JNLP_TYPE = 0;
    static final int RESOURCE_TYPE = 1;
    static final int DELETED_TYPE = 2;

    private final CacheViewer viewer;
    private final int tableType;
    private final boolean isSystem;


    public CacheTable(CacheViewer cv, int type, boolean isSystem) {

        this.viewer = cv;
        this.tableType = type;
        this.isSystem = isSystem;

        setShowGrid(false);
        setIntercellSpacing(new Dimension(0,0));
        setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

        int height = (tableType == JNLP_TYPE) ? 
            JNLP_ROW_HEIGHT : RESOURCE_ROW_HEIGHT;
        setRowHeight(height);

        setPreferredScrollableViewportSize(new Dimension(640, 280));
        addMouseListener(new MouseAdapter() {
            public void mousePressed(MouseEvent e) {
                if (e.isPopupTrigger()) {
                    int y = e.getY();
                    int row = y / getRowHeight();

                    if (row < getModel().getRowCount()) {
                        getSelectionModel().clearSelection();
                        getSelectionModel().addSelectionInterval(row, row);
                        // popup application popup menu
                        viewer.popupApplicationMenu(CacheTable.this, 
                                                    e.getX(), y);
                    }
                }
            }

            public void mouseReleased(MouseEvent e) {
                if (e.isPopupTrigger()) {
                    int y = e.getY();
                    int row = y / getRowHeight();

                    if (row < getModel().getRowCount()) {
                        getSelectionModel().clearSelection();
                        getSelectionModel().addSelectionInterval(row, row);
                        // popup application popup menu
                        viewer.popupApplicationMenu(CacheTable.this, 
                                                    e.getX(), y);
                    }
                }
            }

            public void mouseClicked(MouseEvent e) {
                Point p = e.getPoint();
                if (e.getClickCount() == 2) {
                    if (getSelectedRows().length == 1) {
                        if (e.getButton() == MouseEvent.BUTTON1) {
                            // double click
                            int column = 
                                getColumnModel().getColumnIndexAtX(p.x);
                            if (getSelectedRow() == rowAtPoint(p)
                                    && column < 3) {
                                // do the default action
                                if (tableType == JNLP_TYPE) {
                                    viewer.runApplication();
                                } else if (tableType == DELETED_TYPE) {
                                    viewer.importApplication();
                                } else {
                                    viewer.showInformation();
                                }   
                            }
                        }
                    }
                }
            }
        });

        addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                int keyCode = e.getKeyCode();
                int modifiers = e.getModifiers();
                if (keyCode == e.VK_F10 && ((modifiers & e.SHIFT_MASK) != 0)) {
                    int y = getRowHeight() * getSelectedRow() + 6;
                    int width = 100;
                    if (getModel() instanceof CacheTableModel) {
                        CacheTableModel ctm = (CacheTableModel) getModel();
                        width = ctm.getPreferredWidth(0);
                    }
                    int x = (2 * width) / 3;
                    viewer.popupApplicationMenu(CacheTable.this, x, y);
                } else if (keyCode == e.VK_ENTER) {
                    boolean online = ((modifiers & InputEvent.CTRL_MASK) == 0);
                    // do the default action
                    if (tableType == JNLP_TYPE) {
                        viewer.runApplication(online);
                    } else if (tableType == DELETED_TYPE) {
                        viewer.importApplication();
                    } else {
                        viewer.showInformation();
                    }
		    e.consume();
                } else if (keyCode == e.VK_DELETE || 
                           keyCode == e.VK_BACK_SPACE) {
                    viewer.delete();
                }
            }
        });

        reset();

    }

    public String getSizeLabelText() {
        long size = Cache.getCacheSize(isSystem);
        String str;
        if (size > 10240) {
            str =  (" " + (size)/1024 + " KB");
        } else {
            str = (" " + size/1024 + "." + (size%1024)/102 + " KB");
        }
        if (isSystem) {
            return ResourceManager.getString("viewer.size.system", str);
        }
        return ResourceManager.getString("viewer.size", str);
    }


    public void reset() {
	getSelectionModel().clearSelection();
	getSelectionModel().removeListSelectionListener(viewer);

        TableModel old = getModel();
        if (old instanceof CacheTableModel) {
            ((CacheTableModel) old).removeMouseListenerFromHeaderInTable(this);
        }
        CacheTableModel ctm = new CacheTableModel(tableType, isSystem);
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
        getSelectionModel().addListSelectionListener(viewer);
	getSelectionModel().clearSelection();
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

    // swing workaround to allow the background color to be applied to 
    // the whole area.
    public boolean getScrollableTracksViewportHeight() {
        if (getParent() instanceof JViewport) {
            return (((JViewport)getParent()).getHeight() >
                    getPreferredSize().height);
        }
        return false;
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
            String tooltip = 
                CacheObject.getHeaderToolTipText(column, tableType);
            if (tooltip != null && tooltip.length() > 0) {
                setToolTipText(tooltip);
            }
            return this;
        }
    }

    private class CacheTableModel extends AbstractTableModel implements 
                TableCellRenderer {
        private boolean isSystem;
        private int tableType;
        private CacheObject [] rows;
        private int sortColumn;
        private boolean sortAscending;
        private MouseListener mouseListener = null;
        
        public CacheTableModel(int tableType, boolean isSystem) {
            this.tableType = tableType;
            this.isSystem = isSystem;
            rows = new CacheObject[0];
            sortColumn = -1;
            sortAscending = true;
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
            return defaultRenderer.getTableCellRendererComponent(table, value,
                isSelected, hasFocus, row, column);
        }
        
        private boolean isEntryIPEqual(CacheObject a, CacheObject b) {
            String aIP = a.getCodebaseIP();
            String bIP = b.getCodebaseIP();

            if ((aIP == null && bIP == null) ||
                    (aIP != null && aIP.equals(bIP)) ||
                    (bIP != null && bIP.equals(aIP))) {
                return true;
            }

            return false;
        }
        
        /**
         * Determine whether the CacheObject o shall be displayed
         *
         * @return true if entry shall be displayed
         */
        private boolean validateEntry(ArrayList al, CacheObject o) {
            for (int i = 0; i < al.size(); i++) {
                CacheObject co = (CacheObject) al.get(i);
                if (co.getUrlString().equals(o.getUrlString())) {
                    if (!isEntryIPEqual(co, o)) {
                        // entry matches, but with different IP
                        // do not display duplicate entry in cache viewer
                        return false;
                    }
                }
            }
            return true;
        }

        public void refresh() {
            // cleanup duplicates first
            Cache.removeDuplicateEntries(isSystem, false);

            ArrayList al = new ArrayList();

            if (tableType == JNLP_TYPE) {
                Iterator it = Cache.getJnlpCacheEntries(isSystem);
                while (it.hasNext()) {
                    File cachedJNLPfile = (File) it.next();
                    File idxFile = new File (cachedJNLPfile.getPath() + ".idx");
                    CacheEntry ce = Cache.getCacheEntryFromFile(idxFile);
                    if (ce != null && ce.isValidEntry()) {
                        CacheObject o = new CacheObject(ce, this, tableType);
                        LaunchDesc ld = o.getLaunchDesc();
                        if (ld != null) {
                            // only Applications and Applets are shown in 
                            // this view Extensions (Installers and Librarys) 
                            // are only shown as resources view.
                            if (ld.isApplicationDescriptor()) {
                                // add the entry to the list al if it is not
                                // an old duplicate entry
                                if (validateEntry(al, o)) {
                                    al.add(o);
                                }
                            }
                        }
                    }
                }
            } else if (tableType == RESOURCE_TYPE) {
                File [] files = Cache.getCacheEntries(isSystem);
                for (int i=0; i<files.length; i++) {
                    CacheEntry ce = Cache.getCacheEntryFromFile(files[i]);
                    if (ce != null && ce.isValidEntry()) {
                        CacheObject o = new CacheObject(ce, this, tableType);
                        // add the entry to the list al if it is not
                        // an old duplicate entry
                        if (validateEntry(al, o)) {
                            al.add(o);
                        }
                    }
                }
            } else if (tableType == DELETED_TYPE) {
                Properties p = Cache.getRemovedApps();
                Enumeration e = p.propertyNames();
                while (e.hasMoreElements()) {
                    String url = (String) e.nextElement();
                    String title = p.getProperty(url);
                    al.add(new CacheObject(title, url, this));
                }
            }
            rows = (CacheObject []) al.toArray(new CacheObject[0]);
            if (sortColumn != -1) {
                sort();
            }
        }   

        CacheObject getCacheObject(int row) {
            return rows[row];
        }
        
        public Object getValueAt(int row, int column) {
            return rows[row].getObject(column);
        }   
     
        public int getRowCount() {
            return rows.length;
        }   

        public String getRowHref(int row) {
            return rows[row].getHref();
        }
     
        public int getColumnCount() {
            return CacheObject.getColumnCount(tableType);
        }   
       
        public boolean isCellEditable(int row, int column) {
            return rows[row].isEditable(column);
        }
        
        public Class getColumnClass(int column) {
            return CacheObject.getClass(column, tableType);
        }
     
        public String getColumnName(int column) {
            return CacheObject.getColumnName(column, tableType);
        }

        public void setValueAt(Object value, int row, int col) {
            rows[row].setValue(col, value);
        }
     
        public int getPreferredWidth(int column) {
            return CacheObject.getPreferredWidth(column, tableType);
        }

        public void removeMouseListenerFromHeaderInTable(JTable table) {
            if (mouseListener != null) {
                table.getTableHeader().removeMouseListener(mouseListener);
            }
        }

        public void addMouseListenerToHeaderInTable(JTable table) { 
            final JTable tableView = table; 
            tableView.setColumnSelectionAllowed(false); 
            final ListSelectionModel lsm = tableView.getSelectionModel();
            mouseListener = new MouseAdapter() {
                public void mouseClicked(MouseEvent e) {
                    TableColumnModel columnModel = tableView.getColumnModel();
                    int viewColumn = columnModel.getColumnIndexAtX(e.getX()); 
                    int selected = lsm.getMinSelectionIndex();
                    lsm.clearSelection();
                    int col = tableView.convertColumnIndexToModel(viewColumn); 
                    if (e.getClickCount() == 1 && col >= 0) {
                        int shiftPressed = 
                            e.getModifiers() & InputEvent.SHIFT_MASK; 
                        sortAscending = (shiftPressed == 0); 
                        sortColumn = col;
                        runSort(lsm, selected);
                    }
                }
            };
            tableView.getTableHeader().addMouseListener(mouseListener); 
        }

        public void sort () {

            boolean needs_repaint = false;

            if (sortAscending) {
                for (int i = 0; i < getRowCount(); i++) {
                    for (int j = i+1; j < getRowCount(); j++) {
                        if (rows[i].compareColumns(rows[j], sortColumn) > 0) 
                        {
                            needs_repaint=true;
                            CacheObject tmp = rows[i];
                            rows[i] = rows[j];
                            rows[j] = tmp;
                        }
                    }
                }
            } else {
                for (int i = 0; i < getRowCount(); i++) {
                    for (int j = i+1; j < getRowCount(); j++) {
                        if (rows[j].compareColumns(rows[i], sortColumn) > 0) 
                        {
                            needs_repaint=true;
                            CacheObject tmp = rows[i];
                            rows[i] = rows[j];
                            rows[j] = tmp;
                        }
                    }
                }
            }
            if (needs_repaint) {
                fireTableDataChanged();
            }
        }

        private void runSort(final ListSelectionModel lsm, final int selected) {
            // if (CacheViewer.getStatus() != CacheViewer.STATUS_SORTING) {
                new Thread(new Runnable() {
                    public void run() {
                        // CacheViewer.setStatus(CacheViewer.STATUS_SORTING);
                        try {
                            CacheObject sel = null;
                            if (selected >= 0) {
                                sel = rows[selected];
                            }
                            sort(); 
                            if (sel != null) {
                                for (int i=0; i<rows.length; i++) {
                                    if (rows[i] == sel) {
                                        lsm.clearSelection();
                                        lsm.addSelectionInterval(i, i);
                                        break;
                                    }
                                }
                            }
                        } finally {
                            // CacheViewer.setStatus(CacheViewer.STATUS_OK);
                        }
                    }
                }).start();
            // }
        }
    }

}


