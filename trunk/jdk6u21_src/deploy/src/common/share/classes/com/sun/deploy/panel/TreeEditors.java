/*
 * @(#)TreeEditors.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;

import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeCellEditor;
import javax.swing.tree.DefaultTreeCellEditor;
import javax.swing.tree.TreePath;
import javax.swing.*;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.awt.event.KeyEvent;


/*
 * This class will provide editing of the nodes in the tree.  Existance of an
 * editor waves the neccessity of having a mouse listener for the Tree.
 * Also, if in the future, we would like one of the nodes to have a text field, or 
 * text area as an element, with this approach all we'll have to do is implement
 * another editor and renderer.
 */
public class TreeEditors {

    private static DefaultTreeCellEditor _radioEditor = null;
    private static DefaultTreeCellEditor _checkBoxEditor = null;
    private static DefaultTreeCellEditor _textFieldEditor = null;

    private static TreeEditors _instance = null;
    private static TreeEditors getInstance() {
	if (_instance == null) {
	    _instance = new TreeEditors();
	}
	return _instance;
    }

    public static final class DelegateEditor implements TreeCellEditor {

        public DelegateEditor( JTree tree ) {
            this.tree = tree;
        }

        public Component getTreeCellEditorComponent(JTree tree, Object value,
					 boolean isSelected, boolean expanded,
					 boolean leaf, int row) {
            return currentEditor.getTreeCellEditorComponent(tree, value, 
					isSelected, expanded, leaf, row );
        }

        public void addCellEditorListener(CellEditorListener l) {
            vListeners.add( l );
        }

        public void removeCellEditorListener(CellEditorListener l) {
            vListeners.remove( l );
        }

        public Object getCellEditorValue() {
            return (currentEditor != null) ? 
		    currentEditor.getCellEditorValue() : null;
        }

        public boolean isCellEditable(EventObject anEvent) {
            // If there is an editor then 'yes'
            setCurrentEditor( anEvent );
            return (currentEditor != null);
        }

        public boolean shouldSelectCell(EventObject anEvent) {
            return (currentEditor == null) ? 
		    true : currentEditor.shouldSelectCell( anEvent );
        }

        public boolean stopCellEditing() {
            return (currentEditor != null) ? 
		    currentEditor.stopCellEditing() : true;
        }

        public void cancelCellEditing() {
            if ( currentEditor != null ) {
                currentEditor.cancelCellEditing();
            }
        }

        private void setCurrentEditor( EventObject ev ) {
            TreeCellEditor newEditor = pickEditor( ev );

            if ( currentEditor != null ) {
                currentEditor.removeCellEditorListener( listener );
            }
            currentEditor = newEditor;
            if ( currentEditor != null ) {
                currentEditor.addCellEditorListener( listener );
            }
        }

        private TreeCellEditor pickEditor( EventObject ev ) {
            TreeCellEditor editor = null;
            
            /*
             * MouseEvent would start editing when user clicks on the 
             * editable node.  Node is editable if it has an EDITOR.
             */
            if ( ev instanceof MouseEvent ) {
                MouseEvent mev = (MouseEvent)ev;
                TreePath path = tree.getPathForLocation(mev.getX(), mev.getY());
                if ( path.getLastPathComponent() instanceof IProperty ) {
		    IProperty prop = (IProperty)path.getLastPathComponent();
		    String lockedName = prop.getPropertyName();
		    if (prop instanceof RadioProperty) {
			lockedName = ((RadioProperty) prop).getGroupName();
		    }
		    if (!Config.isLocked(lockedName)) {
                        editor = TreeEditors.getInstance().getEditor(tree, prop);
		    }
                }
            } else {
                /*
                 * This will handle all other cases, and will start editing
                 * the node when we programmatically call 
                 * JTree.startEditingAtPath(..)
                 */
                TreePath path = tree.getSelectionPath();
                if (path.getLastPathComponent() instanceof TextFieldProperty) {
                    IProperty prop = (IProperty)path.getLastPathComponent();
                    if (!Config.isLocked(prop.getPropertyName())){
                        editor = TreeEditors.getInstance().getEditor(tree, prop);
                    }
                }
            }
            return editor;
        }

        private Vector vListeners = new Vector();
        private TreeCellEditor currentEditor;
        private JTree tree;
        private CellEditorListener listener = new CellEditorListener() {
            // Tunnel events to real listeners
             public void editingStopped(ChangeEvent e) {
                 Vector l = (Vector)vListeners.clone();
                 for ( Iterator i = l.iterator(); i.hasNext(); ) {
                    ((CellEditorListener)i.next()).editingStopped( e );
                }
             }

            public void editingCanceled(ChangeEvent e) {
               Vector l = (Vector)vListeners.clone();
               for ( Iterator i = l.iterator(); i.hasNext(); ) {
                    ((CellEditorListener)i.next()).editingCanceled( e );
                }
            }
        };

    }

    private DefaultTreeCellEditor getEditor(JTree tree, IProperty prop) {
	DefaultTreeCellRenderer renderer =
			TreeRenderers.getRenderer();
	if (prop instanceof RadioProperty) {
	    if (_radioEditor == null) {
		_radioEditor = new RadioEditor(tree, renderer);
	    }
	    return _radioEditor;
	} else if (prop instanceof ToggleProperty) {
            if (_checkBoxEditor == null) { 
                _checkBoxEditor = new CheckBoxEditor(tree,renderer);
            } 
	    return _checkBoxEditor;
	} else if (prop instanceof TextFieldProperty) {
	    if (_textFieldEditor == null) {
		_textFieldEditor = new TextFieldEditor(tree,renderer);
	    }
	    return _textFieldEditor;
	} 
	return null;
    }

    

    // base class for all editors
    private class DeployEditor extends DefaultTreeCellEditor {

	public DeployEditor(JTree tree, DefaultTreeCellRenderer renderer) {
	    super(tree, renderer);
	}

        public void addCellEditorListener(CellEditorListener l) {
            vListeners.add( l );
        }

        public void removeCellEditorListener(CellEditorListener l) {
            vListeners.add( l );
        }

        public boolean isCellEditable(EventObject anEvent) {
            return true;
        }

        public boolean shouldSelectCell(EventObject anEvent) {
            if (anEvent instanceof MouseEvent) { 
                MouseEvent e = (MouseEvent)anEvent;
                return e.getID() != MouseEvent.MOUSE_DRAGGED;
            }
            return true;
        }

        public boolean stopCellEditing() {
            return true;
        }

        public void cancelCellEditing() {}

        protected void editingStopped() {
            Vector l = (Vector)vListeners.clone();
            for ( Iterator i = l.iterator(); i.hasNext(); ) {
                ((CellEditorListener)i.next()).editingStopped( changeEvent );
            }
        }

        protected void editingCancelled() {
            Vector l = (Vector)vListeners.clone();
            for ( Iterator i = l.iterator(); i.hasNext(); ) {
                ((CellEditorListener)i.next()).editingCanceled( changeEvent );
            }
        }

        private Vector vListeners = new Vector();
        private ChangeEvent changeEvent = new ChangeEvent( this );
    }
    

    /*
     * This is an editor for the RadioProperty node.
     */
    private class RadioEditor extends DeployEditor {
        public RadioEditor(JTree tree, DefaultTreeCellRenderer renderer) {
	    super(tree, renderer);
            button = new JRadioButton();

            /*  
             * Use MouseListener to cancel editing on mouseExited event - 
             * this is for the case when user clicks on the radio button
             * with the mouse and then draggs mouse to a different tree
             * component.
             */
            button.addMouseListener(new java.awt.event.MouseListener(){
                public void mouseClicked(java.awt.event.MouseEvent em){}
                public void mousePressed(java.awt.event.MouseEvent em){}
                public void mouseReleased(java.awt.event.MouseEvent em){
                    editingStopped();
                }
                public void mouseExited(java.awt.event.MouseEvent em){
                    editingCancelled();
                }
                public void mouseEntered(java.awt.event.MouseEvent em){}
            });                        
        }
                
        /*
         * When user clicks on a JTree, the event is received by JTree Editor, which
         * will figure out what was clicked, and here we are telling it what to do.
         */
        public Component getTreeCellEditorComponent(JTree tree, Object value,
			    boolean isSelected, boolean expanded,
			    boolean leaf, int row) {
            IProperty prop = (IProperty)value;
            button.setSelected(prop.isSelected());            
            button.setText(prop.getDescription());
	    button.setBorder(BorderFactory.createEmptyBorder(1,1,1,1));
            return button;
        }
        /*
         * Get the value
         */
        public Object getCellEditorValue() {
            String r = "";
            if ( button.isSelected() ){
                r = button.getText();                               
            }
            return r;
        }        
        
        private JRadioButton button;
    }
    
    /*
     * This is an editor for the ToggleProperty node.
     */
    private class CheckBoxEditor extends DeployEditor {
        public CheckBoxEditor(JTree tree, DefaultTreeCellRenderer renderer) {
	    super(tree, renderer);
            cb = new JCheckBox();
            cb.addActionListener( new ActionListener() {
                public void actionPerformed( ActionEvent ev ) {
                    editingStopped();
                }
            } );
        }

        /*
         * Since ToggleProperty has only one component - check box,
         * we return it as tree cell editor component.
         */
        public Component getTreeCellEditorComponent(JTree tree, Object value,
					 boolean isSelected, boolean expanded,
					 boolean leaf, int row) {
            IProperty prop = (IProperty)value;
            cb.setSelected(prop.isSelected());
            cb.setText(prop.getDescription());
	    cb.setBorder(BorderFactory.createEmptyBorder(1,1,1,1));
            return cb;
        }

        /*
         * If checkbox if selected, return "true", or else return "false"
         * as a currently selected value.
         */
        public Object getCellEditorValue() {
            return ( cb.isSelected() ) ? "true" : "false";
        }

        private JCheckBox cb;
    }
    
    private class TextFieldEditor extends DeployEditor {
        public TextFieldEditor(JTree tree, DefaultTreeCellRenderer renderer) {
            super(tree, renderer); 
	    panel.setLayout(new BorderLayout());
            path.setColumns(22);
            panel.add(path, BorderLayout.CENTER);
            panel.add(browse_btn, BorderLayout.EAST);

            /*
             * Add action listener to the text field.  When user hits "Enter" key
             * the editing stopped event will occur, indicating that editing stopped.
             * This is the only event that will set the text field to the newly typed
             * value.  
             */
            path.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent ae){
                        editingStopped();
                }
            });
            
            browse_btn.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent evt) {
                    JFileChooser chooser = new JFileChooser();
                    chooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
                    chooser.setDialogTitle(getMessage("deploy.advanced.browse.title"));
                    chooser.setApproveButtonText(getMessage("deploy.advanced.browse.select"));
                    String tooltip = getMessage("deploy.advanced.browse.select_tooltip");
                    chooser.setApproveButtonToolTipText(tooltip);
                    char mnemonic = getMessage("deploy.advanced.browse.select_mnemonic").charAt(0);
                    chooser.setApproveButtonMnemonic(mnemonic);
                    File dir = new File(path.getText());
                    chooser.setCurrentDirectory(dir);                    
                    if (chooser.showDialog(panel, null) == JFileChooser.APPROVE_OPTION) {  
                        String file="";
                        try {
                            file = chooser.getSelectedFile().getCanonicalPath();
                        } catch (IOException ioe) {
                            file = chooser.getSelectedFile().getPath();
                        }
                        path.setText(file);
                    }
                    
                    editingStopped();
                }
            }); 
            
            /*
             * Overwrite the focus traversal keys for the panel.
             * Here we replace FORWARD_TRAVERSAL_KEYS from usual "TAB"
             * to VK_DOWN - to provide experience similar to the rest of the
             * tree.
             */
            Set newKeySet = Collections.synchronizedSortedSet(new TreeSet());
            newKeySet.add(AWTKeyStroke.getAWTKeyStroke(KeyEvent.VK_RIGHT, 0, true));            
            panel.setFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, newKeySet);
            
            // clear the set.
            newKeySet.clear();
            
            /*
             * Here we replace the BACKWARD_TRAVERSAL_KEYS from usual "SHIFT+TAB"
             * to VK_UP.
             */
            newKeySet.add(AWTKeyStroke.getAWTKeyStroke(KeyEvent.VK_LEFT, 0, true));
            panel.setFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, newKeySet);
            
            
            /*
             * Stop editing when user presses "TAB" key.
             */
            KeyStroke ks;
            ks = KeyStroke.getKeyStroke(KeyEvent.VK_TAB, 0, true);
            panel.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(ks, "StopEditingAction");
            panel.getActionMap().put("StopEditingAction", new AbstractAction(){
                public void actionPerformed(ActionEvent ae){                    
                    editingStopped();                    
                }
            });
        }
            
        
        /*
         * When user clicks on a JTree, the event is received by JTree Editor, which
         * will figure out what was clicked, and here we are telling it what to do.
         */        
        public Component getTreeCellEditorComponent(JTree tree, Object value,
					 boolean isSelected, boolean expanded,
					 boolean leaf, int row) {
            if (isSelected) {
                panel.setBackground(renderer.getBackgroundSelectionColor());
                panel.setBorder(BorderFactory.createLineBorder(
                                renderer.getBorderSelectionColor()));
            } else {
                panel.setBackground(renderer.getBackgroundNonSelectionColor());
                panel.setBorder(BorderFactory.createEmptyBorder(1,1,1,1));
            }

            IProperty prop = (IProperty)value;
            path.setText( prop.getValue() );
            path.setFont(tree.getFont());
            return panel;
        }

        /*
         * return text in JTextField
         */
        public Object getCellEditorValue() {
            return path.getText();
        }        
        
        private JPanel panel = new JPanel();
        private JButton browse_btn = new JButton(getMessage("deploy.advanced.browse.browse_btn"));        
        private JTextField path = new JTextField("");  
    }
    
    /*
     * Get localized string from the resource file.
     */
    private static String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }       
}
