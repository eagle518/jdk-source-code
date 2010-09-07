/*
 * @(#)TreeRenderers.java	1.12 04/10/15
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JFileChooser;
import javax.swing.JTextField;
import javax.swing.JButton;
import javax.swing.border.Border;
import javax.swing.border.LineBorder;
import javax.swing.BorderFactory;
import javax.swing.JTree;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Color;
import java.awt.Component;
import java.awt.Insets;
import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Rectangle;
import javax.swing.BoxLayout;
import javax.swing.Box;
import javax.swing.JComponent;


/*
 * This class contains a single renderer used to paint 
 * the nodes in the advanced options tree.
 */
public class TreeRenderers {

    public static DefaultTreeCellRenderer getRenderer() {
        return _renderer;
    }

    private static final DefaultTreeCellRenderer _renderer = 
	new DefaultTreeCellRenderer() 
    {
        public Component getTreeCellRendererComponent(JTree tree, 
		Object value, boolean selected, boolean expanded,
		boolean leaf, int row, boolean hasFocus) 
	{
            if (value instanceof ToggleProperty) {
		// CheckBox
		ToggleProperty prop = (ToggleProperty) value;
		_cbPanel.removeAll();

                if (selected) {
		    _checkBox.setForeground(getTextSelectionColor());
		    _checkBox.setBackground(getBackgroundSelectionColor());
		    _cbPanel.setBackground(getBackgroundSelectionColor());
		    _checkBox.setBorder(BorderFactory.createLineBorder(
                                    getBorderSelectionColor()));
                } else {
                    _checkBox.setForeground(getTextNonSelectionColor());
		    _checkBox.setBackground(getBackgroundNonSelectionColor());
		    _cbPanel.setBackground(getBackgroundNonSelectionColor());
		    _checkBox.setBorder(_invisBorder);
                }

                // Select the check box if "checked" value is set to true.
                _checkBox.setSelected(prop.isSelected());
                _checkBox.setText(prop.getDescription());
                _checkBox.setFont(tree.getFont());
                _checkBox.setEnabled(
			!Config.isLocked(prop.getPropertyName()));
                _checkBox.setRequestFocusEnabled(hasFocus);
		_checkBox.setOpaque(false);
                _cbPanel.setOpaque(true);
                _cbPanel.setToolTipText(prop.getTooltip());
		_cbPanel.add(_checkBox);
                return _cbPanel;
            } else if (value instanceof RadioProperty) {
		// RadioButton
		RadioProperty prop = (RadioProperty) value;
		_radioPanel.removeAll();
		// _radioPanel.setBorder(_emptyBorder);
                if (selected) {
		    _radio.setForeground(getTextSelectionColor());
		    _radio.setBackground(getBackgroundSelectionColor());
		    _radioPanel.setBackground(getBackgroundSelectionColor());
		    _radio.setBorder(BorderFactory.createLineBorder(
				    getBorderSelectionColor()));
                } else {
		    _radio.setForeground(getTextNonSelectionColor());
		    _radio.setBackground(getBackgroundNonSelectionColor());
		    _radioPanel.setBackground(getBackgroundNonSelectionColor());
		    _radio.setBorder(_invisBorder);
                }

                _radio.setText(prop.getDescription());
                _radio.setFont(tree.getFont());
                _radio.setSelected(prop.isSelected());
                _radio.setEnabled(!Config.isLocked(prop.getGroupName()));
                _radio.setRequestFocusEnabled(hasFocus);
                _radio.setOpaque(false);

		_radioPanel.setOpaque(true);
                _radioPanel.setToolTipText(prop.getTooltip());
		_radioPanel.add(_radio);
                return _radioPanel;
	    } else if (value instanceof TextFieldProperty) {
		// JTextField:
		TextFieldProperty prop = (TextFieldProperty) value;
		_textPanel.removeAll();
	        if (selected) {
		    _textPanel.setBackground(getBackgroundSelectionColor());
		    _textPanel.setBorder(BorderFactory.createLineBorder(
			getBorderSelectionColor()));
	        } else {
		    _textPanel.setBackground(
			getBackgroundNonSelectionColor());
		    _textPanel.setBorder(_invisBorder);
	        }
    
	        _path.setColumns(22);
	        _textPanel.add(_path, BorderLayout.CENTER);
	        _textPanel.add(_browse, BorderLayout.EAST);
	        _textPanel.setOpaque(true);
    
	        boolean enab = !Config.isLocked(prop.getPropertyName());
	        _path.setEnabled(enab);
	        _browse.setEnabled(enab);
	        _path.setText(prop.getValue());
	        _path.setFont(tree.getFont());
    
	        return _textPanel;
	    } else {
		// Label
		_labelPanel.removeAll();
		// _labelPanel.setBorder(_emptyBorder);
                if (selected) {
		    _label.setForeground(getTextSelectionColor());
		    _label.setBackground(getBackgroundSelectionColor());
		    _label.setBorder(BorderFactory.createLineBorder(
				getBorderSelectionColor()));
                } else {
		    _label.setForeground(getTextNonSelectionColor());
		    _label.setBackground(getBackgroundNonSelectionColor());
		    _label.setBorder(_invisBorder);
                }
	        _label.setFont(tree.getFont());
                _label.setText(String.valueOf( value ));
                _label.setOpaque(true);
		_labelPanel.add(_label);
	        return _labelPanel;
	    }
        }

	// for JToggleProperty:
        private JCheckBox _checkBox = new JCheckBox() {
	    public void paintBorder(Graphics g) {
		// it seems the checkbox paintBorder() is not working for this		
		// with borderSelectionColor line border, so we do it here:
		getBorder().paintBorder(this, g, 0, 0, 
					getWidth(), getHeight());
	    }
	    public void paint(Graphics g) {
		super.paint(g);
	    }
	};
        private JPanel _cbPanel = new JPanel(new BorderLayout());


	// for JRadioProperty:
	private final JRadioButton _radio = new JRadioButton() {
	    public void paintBorder(Graphics g) {
	        // it seems the JRadio paintBorder() is not working for this
		// borderSelectionColor line border, so we do it ourselves
		getBorder().paintBorder(this, g, 0, 0, 
					getWidth(), getHeight());
	    }
	};
        private JPanel _radioPanel = new JPanel(new BorderLayout());

	// for TextfieldProperty:
        private JPanel _textPanel = new JPanel(new BorderLayout());
	private JButton _browse = new JButton(
	    ResourceManager.getMessage("deploy.advanced.browse.browse_btn"));
	private JTextField _path = new JTextField("");

	// for Label:
        private final JLabel _label = new JLabel();
	private JPanel _labelPanel = new JPanel(new BorderLayout());

	// for anyone
	private Border _emptyBorder = BorderFactory.createEmptyBorder();
	private Border _invisBorder = 
			BorderFactory.createEmptyBorder(1,1,1,1);
    };
}
