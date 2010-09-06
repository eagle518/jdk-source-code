/*
 * @(#)JarCacheTable.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.util.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import com.sun.deploy.resources.ResourceManager;

public class JarCacheTable extends JTable {

    public static final Font tableFont = ResourceManager.getUIFont();

    private DefaultTableCellRenderer tableCellRenderer = new JarCacheTableCellRenderer(tableFont);
//  private JarCacheTableModel tableModel = null;
    private JarCacheTableColumnModel tableColumnModel = new JarCacheTableColumnModel();
    private JarCacheTableColumn [] tableColumns = null;
    private JarCacheTableSorter sorter = null;
    private String[] colNames = null;
//    private boolean mouseDrag = false;
//    private boolean columnDrag = false;
    
    // Constructor
    public JarCacheTable() {
	sizeColumnsToFit(JTable.AUTO_RESIZE_LAST_COLUMN);
	setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

	setColumnModel(tableColumnModel);

	tableColumns = tableColumnModel.getColumnArray();
	colNames = new String[tableColumns.length];
	for ( int i=0; i<tableColumns.length; i++ ) {
	    colNames[i] = tableColumns[i].getHeaderValue().toString();
	}

//	tableModel = new JarCacheTableModel(colNames);

	// Add a sorter
	sorter = new JarCacheTableSorter(colNames);
	sorter.addMouseListenerToHeaderInTable(this);
	setModel(sorter);

	int size = tableColumnModel.getColumnCount();
	TableColumn column = null;
	for ( int i=0; i<size; i++ ) {
	    column = columnModel.getColumn(i);
	    if ( tableColumns[i].isVisible() ) {
		column.setMinWidth(tableColumns[i].getMinWidth());
		column.setPreferredWidth(tableColumns[i].getPreferredWidth());
		column.setResizable(tableColumns[i].getResizable());
	    }
	    else {
		column.setMinWidth(0);
		column.setMaxWidth(0);
	    }
	}

	setHeaderRenderers();

	//Action listener to enable deleting the rows using delete key
	ActionListener deleteRows = new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		removeRows();			
	    }
	};
	this.registerKeyboardAction( deleteRows, 
		KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0), JComponent.WHEN_IN_FOCUSED_WINDOW );


	//setShowHorizontalLines(false);
	//setShowVerticalLines(false);
	getTableHeader().setReorderingAllowed(false);
	setPreferredScrollableViewportSize(getPreferredSize());
    }

    public void setHeaderRenderers() {
	TableColumnModel model = getColumnModel();
	int count = model.getColumnCount();
	for ( int i=0; i<count; i++ ) {
	    model.getColumn(i).setHeaderRenderer(new JarCacheTableHeaderRenderer());
	}
    }

    //Throws null pointer exception if the following method is not commented
    /*public TableColumnModel getColumnModel() {
	return tableColumnModel;
    }*/

    public void refresh() {
	sorter.refresh();
    }


    public JarCacheTableColumnModel getJarCacheTableColumnModel() {
	return tableColumnModel;
    }

    public JarCacheTableColumn [] getJarCacheTableColumns() {
	return tableColumns;
    }

    public TableCellRenderer getCellRenderer(int row, int column) {
	return tableCellRenderer;
    }

    public Font getFont() {
	return tableFont;
    }

    public void removeRows() {
	sorter.removeRows(getSelectedRows());
    }

    public void adjustColumnSize(Container comp){
	int size = tableColumnModel.getColumnCount();
	int minWidth=0;
	int width=0;
	TableColumn column = null;

	for ( int i=0; i<size-1; i++ ) {
	    column = columnModel.getColumn(i);
	    if ( tableColumns[i].isVisible() ) {
		minWidth += tableColumns[i].getWidth();
		width += column.getPreferredWidth();
	    }
	}

	//if(minWidth > width) {
	    width = minWidth;
	//}
	
	Dimension dim = comp.getSize();
	Insets insets = comp.getInsets();

	column = columnModel.getColumn(size-1);
	int lastColWidth = (int)(dim.getWidth()-width-insets.left-insets.right-3);
	column.setPreferredWidth(lastColWidth);
    }
}

class JarCacheTableCellRenderer extends DefaultTableCellRenderer  {
    private Font tableFont = null;

    public JarCacheTableCellRenderer(Font tableFont) {
	this.tableFont = tableFont;
    }

    public Component getTableCellRendererComponent(JTable table,
						   Object value, 
						   boolean isSelected, 
						   boolean hasFocus, 
						   int row,
						   int column) {
	setText(value == null ? "" : " " + value.toString());

	if ( isSelected ) {
	    setForeground(table.getSelectionForeground());
            setBackground(table.getSelectionBackground());
   	} else {
            setForeground(table.getForeground());
            setBackground(table.getBackground());
	}

	setHorizontalAlignment(SwingConstants.LEFT);
	setFont(tableFont);
	return this; 
    } 
} 

//
// Default Table Header Renderer
//
class JarCacheTableHeaderRenderer extends DefaultTableCellRenderer {
    JToolTip toolTip = new JToolTip();

    public JarCacheTableHeaderRenderer() {
	toolTip.setBackground(Color.blue);
	toolTip.setComponent(this);
    }

    public Component getTableCellRendererComponent(JTable table,
						   Object value,
						   boolean isSelected,
						   boolean hasFocus,
						   int row,
						   int column) {
	JTableHeader header = table.getTableHeader();
	if (header != null) {
	    setForeground(header.getForeground());
	    setBackground(header.getBackground());
	    setFont(header.getFont());
	    setHorizontalAlignment(SwingConstants.LEFT);
	}
	
	setText((value == null) ? "" : " " + value.toString());
	setBorder(UIManager.getBorder("TableHeader.cellBorder"));
	setToolTipText(((JarCacheTable)table).getJarCacheTableColumns()[column].getToolTipText());
	return this;
    }

    public JToolTip createToolTip() {
	return toolTip;
    }
}





