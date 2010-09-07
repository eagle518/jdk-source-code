/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.hotspot.igv.view;

import com.sun.hotspot.igv.graph.Figure;
import com.sun.hotspot.igv.data.Properties;
import com.sun.hotspot.igv.data.Properties.RegexpPropertyMatcher;
import com.sun.hotspot.igv.data.Property;
import java.awt.GridLayout;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.List;
import java.util.SortedSet;
import java.util.TreeSet;
import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JTextField;

/**
 *
 * @author Thomas Wuerthinger
 */
class FindPanel extends JPanel implements KeyListener {

    private JComboBox nameComboBox;
    private JTextField valueTextField;

    public FindPanel(List<Figure> figures) {
        createDesign();
        updateComboBox(figures);
    }

    protected void createDesign() {
        setLayout(new GridLayout());
        nameComboBox = new JComboBox();
        valueTextField = new JTextField();
        add(nameComboBox);
        add(valueTextField);
        valueTextField.addKeyListener(this);
    }

    public void updateComboBox(List<Figure> figures) {

        String sel = (String) nameComboBox.getSelectedItem();
        SortedSet<String> propertyNames = new TreeSet<String>();

        for (Figure f : figures) {
            Properties prop = f.getProperties();
            for (Property p : prop) {
                if (!propertyNames.contains(p.getName())) {
                    propertyNames.add(p.getName());
                }
            }
        }

        for (String s : propertyNames) {
            nameComboBox.addItem(s);
        }
        nameComboBox.setSelectedItem(sel);
    }

    public String getNameText() {
        return (String) nameComboBox.getSelectedItem();
    }

    public String getValueText() {
        return valueTextField.getText();
    }

    public void keyTyped(KeyEvent e) {
    }

    public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_ENTER) {
            find();
        }
    }

    public void find() {
        EditorTopComponent comp = EditorTopComponent.getActive();
        if (comp != null) {
            RegexpPropertyMatcher matcher = new RegexpPropertyMatcher(getNameText(), getValueText());
            comp.setSelection(matcher);
        }
    }

    public void keyReleased(KeyEvent e) {

    }
}
