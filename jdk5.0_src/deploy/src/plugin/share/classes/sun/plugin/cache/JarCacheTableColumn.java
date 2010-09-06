/*
 * @(#)JarCacheTableColumn.java	1.8 03/12/19
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
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import com.sun.deploy.util.DialogFactory;

public class JarCacheTableColumn extends TableColumn {

    /**
     * The size of the text output field.
     */
    private int fieldSize = 0;

    /**
     * Should the column be left aligned on text output.
     */
    private boolean leftAligned = true;

    /**
     * Is the column visible in the display and saved text file.
     */
    private boolean visible = true;

    /**
     * The tooltip text to provide for this column.
     */
    private String toolTipText = null;

    /**
     * The preferred sort order.
     */
    private boolean ascending = true;


    /**
     * Constructor
     *
     * @param header The header value
     * @param toolTipText The value of the toolTip for this column
     * @param width The exact width of this column
     * @param fieldSize The size of the output field for this column
     * @param leftAligned The alignment preference for this column
     * @param visible The visibility of this column.
     * @param ascending The preferred sort order for this column.
     */

    public JarCacheTableColumn(String header, String toolTipText, int width, int fieldSize, boolean leftAligned, boolean visible, boolean ascending) {
	setHeaderValue(header);
	setToolTipText("<html><body bgcolor=yellow><font face=Helvetica size=2>" + toolTipText + "</font></body></html>");
	setMinWidth(width);
	setMaxWidth(width);
	setFieldSize(fieldSize);
	setResizable(false);
	setLeftAligned(leftAligned);
	setVisible(visible);
    }

    public JarCacheTableColumn(String header, String toolTipText, boolean reSize ) {
	setHeaderValue(header);
	setToolTipText("<html><body bgcolor=yellow><font face=Helvetica size=2>" + toolTipText + "</font></body></html>");
	setResizable(reSize);
	setLeftAligned(JarCacheTableColumnModel.LEFT_ALIGNED);
	setVisible(JarCacheTableColumnModel.VISIBLE);
	int width = JarCacheEntry.getTextWidth(JarCacheTable.tableFont, header);
	setMinWidth(width);
	setMaxWidth(width);
	setPreferredWidth(width);
    }


    /**
     * Is this column visible?
     *
     * @return True if the column is visible, false otherwise.
     */
    public boolean isVisible() {
	return visible;
    }

    /**
     * Set this column's visibility.
     *
     * @param True if the column is visible in the display and saved text file.
     */
    public void setVisible(boolean visible) {
	this.visible = visible;
    }

    /**
     * Get the field size for this column.
     *
     * @return The field size for this column.
     */
    public int getFieldSize() {
	return fieldSize;
    }

    /**
     * Set the field size for this column.
     *
     * @param fieldSize The size of this column field.
     */
    public void setFieldSize(int fieldSize) {
	this.fieldSize = fieldSize;
    }


    /**
     * Is the field output to be left aligned?
     *
     * @return True for left alignment, false otherwise.
     */
    public boolean isLeftAligned() {
	return leftAligned;
    }

    /**
     * Set the left alignment boolean.
     *
     * @param leftAligned True if the field is to be left aligned on output.
     */
    public void setLeftAligned(boolean leftAligned) {
	this.leftAligned = leftAligned;
    }


    /**
     * Get the tooltip text for this column.
     *
     * @return The text for the tooltip.
     */
    public String getToolTipText() {
	return toolTipText;
    }

    /**
     * Set the tooltip text for this column.
     *
     * @param The text for the tooltip.
     */
    public void setToolTipText(String toolTipText) {
	this.toolTipText = toolTipText;
    }
}
