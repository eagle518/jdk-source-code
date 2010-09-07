/*
 * @(#)JreDialog.java	1.25 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;
import com.sun.deploy.config.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.DialogTemplate;


public class JreDialog extends JDialog 
    implements ActionListener, ChangeListener, ListSelectionListener {

    private final JButton _findBtn;
    private final JButton _addBtn;
    private final JButton _removeBtn;
    private final JButton _okBtn;
    private final JButton _cancelBtn;
    private final JreTableModel _sysTableModel;
    private final JreTableModel _userTableModel;
    private final JTabbedPane _tabbedPane;
    private final JTable _userTable;
    private final JTable  _sysTable;
    private final JScrollPane _userTab;
    private final JScrollPane _systemTab;

    /** Creates new form AdvancedAdvanced */
    public JreDialog(Frame parent, boolean modal) {
        super(parent, modal);  
        _findBtn = makeButton("deploy.jre.find.button");
        _addBtn = makeButton("deploy.jre.add.button");
        _removeBtn = makeButton("deploy.jre.remove.button");

	// OK and Cancel buttons are default action buttons -
	// they do not need mnemonics.
        _okBtn =  new JButton(
            ResourceManager.getMessage("deploy.jre.ok.button"));
	_okBtn.addActionListener(this);
        _cancelBtn =  new JButton(
            ResourceManager.getMessage("deploy.jre.cancel.button"));
	_cancelBtn.addActionListener(this);

        _tabbedPane = new JTabbedPane();
        _userTableModel = new JreTableModel(false);
	_userTable = new JreTable(_userTableModel);
        _sysTableModel = new JreTableModel(true);
        _sysTable = new JreTable(_sysTableModel);
        _userTab = new JScrollPane(_userTable);        
        _systemTab = new JScrollPane(_sysTable);    
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {

        setTitle(ResourceManager.getMessage("deploy.jre.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
		closeDialog();
            }
        });

        _findBtn.setToolTipText(ResourceManager.getMessage(
		"deploy.jre.find_btn.tooltip"));
        _addBtn.setToolTipText(ResourceManager.getMessage(
		"deploy.jre.add_btn.tooltip"));
        _removeBtn.setToolTipText(ResourceManager.getMessage(
		"deploy.jre.remove_btn.tooltip"));

	JButton [] btns = {_findBtn, _addBtn, _removeBtn};
	DialogTemplate.resizeButtons(btns);

        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BorderLayout());
        topPanel.setBorder(new TitledBorder(ResourceManager.getMessage(
		 "deploy.jre.versions")));

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

	// Even decision buttons
	JButton [] decisionBtns = {_okBtn, _cancelBtn};
	DialogTemplate.resizeButtons(decisionBtns);
        
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
	_userTable.getColumnModel().getColumn(3).setCellRenderer(new PathRenderer());

        // Set an editor that will show badBorder if the border isn't valid.
	_userTable.getColumnModel().getColumn(3).setCellEditor(new PathEditor());

        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", new AbstractAction() {
            public void actionPerformed(ActionEvent ae) {
                closeDialog();
            }
        });

        //Set table header not focusable . 6345247
        _userTable.getTableHeader().setFocusable(false); 
        _sysTable.getTableHeader().setFocusable(false); 
    }

    private JButton makeButton(String key) {
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

    private void enableButtons() {
	Component c = _tabbedPane.getSelectedComponent();
	if (c != null) {
	    if (c.equals(_userTab)) {
                boolean selected = false;
		boolean registered = false;
     
                for (int i=0; i<_userTableModel.getRowCount(); i++) {
                    if (_userTable.isRowSelected(i)) {
                        selected = true;
			if (_userTableModel.getJRE(i).isRegistered()) {
			    registered = true;
			}
                    }    
                }
		// if one of the selected is registered, cannot remove
		// Note the current running jvm on Unix is "registered"
                _removeBtn.setEnabled(selected && !registered);
	        _findBtn.setEnabled(true);
	        _addBtn.setEnabled(true);
            } else {
                _removeBtn.setEnabled(false);
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
	JreTableModel model = (JreTableModel) table.getModel();
	
        if (src == _findBtn) {
            findJREs();
	} else if (src == _addBtn){ 
	    model.add(new JREInfo(null, null, null, null, null, Config.getOSName(), 
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

    private void apply() {
	JTable[] tables = {_userTable, _sysTable};

	for (int i = 0; i < tables.length; i++ ) {
	    // flushes any changes in the table.
	    if (tables[i].equals(_userTable) && tables[i].isEditing()) {
		tables[i].getCellEditor().stopCellEditing();
	    }
	    JreTableModel model = (JreTableModel) tables[i].getModel();
	    
	    model.validateAndSave();
	}
    }

    /**
     * Gives the user a chance to choose a path to search for JREs from.
     */  
    private void findJREs() {
        JTable table = getSelectedTable();
	JreTableModel model = (JreTableModel) table.getModel();
        try {
            JREInfo [] jres = JreFindDialog.search(this);

            if (jres != null) {
                for (int counter = 0; counter < jres.length; counter++) {
		    //jre from JreFindDialog should already be validated.
                    model.add(jres[counter], true, true);
                }
            }    
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
    }  

    /**
     * Customerized JTable to show tooltips
     */
    static class JreTable extends JTable {
	private boolean _systemTable = false;

	public JreTable(JreTableModel model) {
	    super(model);

	    _systemTable = model.isSystem();
	    TableColumnModel columnModel =getColumnModel();
	    //set preferred column width
	    columnModel.getColumn(0).setPreferredWidth(60);
	    columnModel.getColumn(1).setPreferredWidth(60);
	    columnModel.getColumn(2).setPreferredWidth(80);
	    columnModel.getColumn(3).setPreferredWidth(120);
	    columnModel.getColumn(4).setPreferredWidth(120);
	    columnModel.getColumn(5).setPreferredWidth(50);
	}

	private String[] columnToolTips = {
	    ResourceManager.getMessage("jretable.platform.tooltip"),
	    ResourceManager.getMessage("jretable.product.tooltip"),
	    ResourceManager.getMessage("jretable.location.tooltip"),
	    ResourceManager.getMessage("jretable.path.tooltip"),
	    ResourceManager.getMessage("jretable.vmargs.tooltip"),
	    ResourceManager.getMessage("jretable.enable.tooltip")
	};
	
	// table header tooltips
	protected JTableHeader createDefaultTableHeader() {
	    return new JTableHeader(columnModel) {
		public String getToolTipText(MouseEvent e) {
		    String tip = null;
		    Point p = e.getPoint();
		    int i = columnModel.getColumnIndexAtX(p.x);
		    int index = columnModel.getColumn(i).getModelIndex();
		    return columnToolTips[index];
		}
	    };
	}
	
	// table cell tooltips
	public String getToolTipText(MouseEvent e) {
	    String tip = null;
	    Point p = e.getPoint();
	    int row = rowAtPoint(p);
	    int col = columnAtPoint(p);
	    int realColIndex = convertColumnIndexToModel(col);
	    
	    // show tooltips for Location and Runtime parameters
	    if (realColIndex == 2 || realColIndex == 4  ) {
		tip = (String)getValueAt(row, col);
	    } else if (realColIndex == 3 && _systemTable) {
		//show tooltip for path column in system tab
		tip = (String)getValueAt(row, col);
	    } else { 
		tip = super.getToolTipText(e);
	    }

	    // turn off tip if it is empty string
	    if ("".equals(tip)) return null;
	    
	    return tip;
	}
    }
}
