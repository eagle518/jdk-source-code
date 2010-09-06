/*
 * @(#)JavawsJreDialog.java	1.14 04/03/30
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.*;
import com.sun.deploy.util.Trace;


public class JavawsJreDialog extends JDialog 
	     implements ActionListener, ChangeListener, ListSelectionListener {

    private final JButton _chooseBtn;
    private final JButton _findBtn;
    private final JButton _addBtn;
    private final JButton _removeBtn;
    private final JButton _okBtn;
    private final JButton _cancelBtn;
    private final JavawsTableModel _sysTableModel;
    private final JavawsTableModel _userTableModel;
    private final JTabbedPane _tabbedPane;
    private final JTable _userTable;
    private final JTable  _sysTable;
    private final JScrollPane _userTab;
    private final JScrollPane _systemTab;

    /** Creates new form AdvancedAdvanced */
    public JavawsJreDialog(Frame parent, boolean modal) {
        super(parent, modal);

	//_hidden = new HashSet();
        _chooseBtn = makeButton("jnlp.jre.choose.button");
        _findBtn = makeButton("jnlp.jre.find.button");
        _addBtn = makeButton("jnlp.jre.add.button");
        _removeBtn = makeButton("jnlp.jre.remove.button");
        _okBtn = makeButton("jnlp.jre.ok.button");
        _cancelBtn = makeButton("jnlp.jre.cancel.button");
        _tabbedPane = new JTabbedPane();
        _userTableModel = new JavawsTableModel(false);
	_userTable = new JTable(_userTableModel);
        _sysTableModel = new JavawsTableModel(true);
        _sysTable = new JTable(_sysTableModel);
        _userTab = new JScrollPane(_userTable);        
        _systemTab = new JScrollPane(_sysTable);      

        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {
        setTitle(ResourceManager.getMessage("jnlp.jre.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
		closeDialog();
            }
        });

        _findBtn.setToolTipText(ResourceManager.getMessage(
		"jnlp.jre.find_btn.tooltip"));
        _addBtn.setToolTipText(ResourceManager.getMessage(
		"jnlp.jre.add_btn.tooltip"));
        _removeBtn.setToolTipText(ResourceManager.getMessage(
		"jnlp.jre.remove_btn.tooltip"));

        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BorderLayout());
        topPanel.setBorder(new TitledBorder(ResourceManager.getMessage(
		 "jnlp.jre.versions")));

	Dimension sz = new Dimension(500, 150);

	Border goodBorder = new LineBorder(_userTable.getForeground());
        _userTable.setBorder(goodBorder);
        _userTable.setPreferredScrollableViewportSize(sz);

        _sysTable.setBorder(goodBorder);
        _sysTable.setPreferredScrollableViewportSize(sz);

	_tabbedPane.addTab(
	    ResourceManager.getMessage("cert.dialog.user.level"), _userTab);          
        _tabbedPane.addTab(
	    ResourceManager.getMessage("cert.dialog.system.level"), _systemTab);

	_tabbedPane.setSelectedIndex(0);
	_tabbedPane.addChangeListener(this);
	topPanel.add(_tabbedPane, BorderLayout.NORTH);
        
        //buttonsPanel.setLayout(new java.awt.BorderLayout());
        Box buttonsBox = Box.createHorizontalBox();
        buttonsBox.add(_chooseBtn);
        buttonsBox.add(Box.createHorizontalGlue());
        buttonsBox.add(_findBtn);
        buttonsBox.add(Box.createHorizontalStrut(5));        
        buttonsBox.add(_addBtn);
        buttonsBox.add(Box.createHorizontalStrut(5));
        buttonsBox.add(_removeBtn);
        
        topPanel.add(buttonsBox, BorderLayout.SOUTH);
                
        getContentPane().add(topPanel, BorderLayout.NORTH);
        
        /*
         * assemble decisionPanel -
         *   ok button
         *   cancell button.
         */
        JPanel decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 10, 5));
        
        decisionPanel.add(_okBtn);
        decisionPanel.add(_cancelBtn);
        
        // Add decision panel to the contentPane.
        getContentPane().add(decisionPanel, BorderLayout.SOUTH);

        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(_okBtn);
        
	enableButtons();
        pack();

	_userTable.getSelectionModel().addListSelectionListener(this);
	_userTable.getColumnModel().getColumn(3).setCellRenderer (
                                                    new PathRenderer());

        // Set an editor that will show badBorder if the border isn't valid.
        _userTable.getColumnModel().getColumn(3).setCellEditor(
							new PathEditor());
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put( 
            KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE),"cancel"); 
        getRootPane().getActionMap().put("cancel", new AbstractAction() {
            public void actionPerformed(ActionEvent ae) {
                closeDialog();
            }
        });

    }

    public JButton makeButton(String key) {
	JButton b = new JButton(ResourceManager.getMessage(key));
	b.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
	b.addActionListener(this);
	return b;
    }

    public void valueChanged(ListSelectionEvent e) {
	enableButtons();
    }
 
    public void stateChanged(ChangeEvent ev) {
	enableButtons();
    }

    public void enableButtons() {
	Component c = _tabbedPane.getSelectedComponent();
	if (c != null) {
	    if (c.equals(_userTab)) {
                boolean valid_unselected = false;
                boolean selected = false;
		boolean registered = false;
     
                for (int i=0; i<_userTableModel.getRowCount(); i++) {
                    if (_userTable.isRowSelected(i)) {
                        selected = true;
			if (_userTableModel.getJRE(i).isRegistered()) {
			    registered = true;
			}
                    } else {
                        if (_userTableModel.isPathValid(i)) {
                            valid_unselected = true;
                        }
                    }    
                }
                _removeBtn.setEnabled(selected && !registered && 
				      valid_unselected);
	        _chooseBtn.setEnabled(selected);
	        _findBtn.setEnabled(true);
	        _addBtn.setEnabled(true);
            } else {
                _removeBtn.setEnabled(false);
	        _chooseBtn.setEnabled(false);
	        _findBtn.setEnabled(false);
	        _addBtn.setEnabled(false);
	    }
        }
    }

    /** Closes the dialog */
    private void closeDialog() {
	setVisible(false);
	dispose();
    }

    public void actionPerformed(ActionEvent ae) {
	JButton src = (JButton) ae.getSource();
        JTable table = getSelectedTable();
	JavawsTableModel model = (JavawsTableModel) table.getModel();
	
	if (src == _chooseBtn) {
	    int row = table.getSelectedRow();
            if (row != -1) {
                changePath(row);
	    }
	} else if (src == _findBtn) {
            findJREs();
	} else if (src == _addBtn){ 
	    model.add(new JREInfo(null, null, null, null, Config.getOSName(), 
			  Config.getOSArch(), true, false), false, true);
            int row = model.getRowCount() - 1;
	    table.requestFocus();
	    table.setRowSelectionInterval(row, row);
	} else if (src == _removeBtn) {
	    model.remove(_userTable.getSelectedRows());
	} else if (src == _okBtn) {
	    apply();
            
            /*
             * This is to enable "Apply" button in Control Panel
             */
            ControlPanel.propertyHasChanged();
	    closeDialog();
	} else if (src == _cancelBtn) {
	    closeDialog();
	}
    }

    private JTable getSelectedTable() {
	return ((_tabbedPane.getSelectedComponent() == _userTab) ? 
            	 _userTable : _sysTable);
    }

    public void apply() {

	// first the user ones ...
        JTable table = _userTable;

        // flushes any changes in the table.
	if (table.isEditing()) {
	    table.getCellEditor().stopCellEditing();
	}
	JavawsTableModel model = (JavawsTableModel) table.getModel();
        
        if (model.getRowCount() > 0 ) {
	    JREInfo.clear();
            
	    // Apply the new ones.
	    for (int n = 0; n < model.getRowCount(); n++) {                
                //Add JRE from table model to JREInfo.
	        JREInfo.addJRE(model.getJRE(n));
	    }
	}

	// next the system ones
        table = _sysTable;

        // flushes any changes in the table. (can't be any)
	if (table.isEditing()) {
	    table.getCellEditor().stopCellEditing();
	}
	model = (JavawsTableModel) table.getModel();

        if (model.getRowCount() > 0 ) {
	    // Apply the new ones.
	    for (int n = 0; n < model.getRowCount(); n++) {
	        JREInfo.addJRE(model.getJRE(n));
	    }
	}

        //  finally add back any hidden jres 
        for (Iterator n = model.getHiddenJREs().iterator(); n.hasNext(); ) {
            JREInfo.addJRE((JREInfo)n.next());
        }
    }

    /**
     * Brings up a file chooser for the user to choose a new path for
     * <code>row</code>.
     */  
    private void changePath(int row) {
        JTable table = getSelectedTable();
	JavawsTableModel model = (JavawsTableModel) table.getModel();
        JFileChooser fc = new JFileChooser();
	String path = null;
	if (row >= 0) path = model.getJRE(row).getPath();
        File f = getFirstValidParent(path);

        if (f != null && f.exists()) {
            fc.setSelectedFile(f);
        }

        // Bring up the file chooser.
        if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
    	    model.setValueAt(fc.getSelectedFile().getPath(), row, 3);
	}
    }    

    /**
     * Gives the user a chance to choose a path to search for JREs from.
     */  
    private void findJREs() {
        JTable table = getSelectedTable();
	JavawsTableModel model = (JavawsTableModel) table.getModel();
        try {
            JREInfo [] jres = JreFindDialog.search(this);

            if (jres != null) {
                for (int counter = 0; counter < jres.length; counter++) {
                    model.add(jres[counter], false, true);
                }
            }    
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
    }    

    /**
     * Convenience method that returns the first parent directory of
     * the entry at row that exists.
     */  
    private File getFirstValidParent(String path) {
        if (path != null) {
            File f = new File(path);
            // Find the first valid directory starting at path
            while (f != null && !f.exists()) {
                path = f.getParent();
                if (path != null) {
                    f = new File(path);
                } else {
                    f = null;
                }
            }
            return f;
        }
        return null;
    }

}


