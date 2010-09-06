/*
 * @(#)ConnectDialog.java	1.19 04/06/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.net.MalformedURLException;
import java.io.IOException;
import javax.swing.*;
import javax.swing.Timer;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;

import javax.management.remote.JMXServiceURL;
import javax.management.remote.JMXConnector;

// Sun specific
import sun.management.ConnectorAddressLink;
import sun.jvmstat.monitor.*;

public class ConnectDialog extends JDialog
		implements ActionListener, DocumentListener, FocusListener, ItemListener, KeyListener {

    private int LOCAL_TAB;
    private int REMOTE_TAB;
    private int ADVANCED_TAB;
    
    JConsole jConsole;
    JTextField hostName, jmxUrl, portNumber, userNameTF, passwordTF, 
	userNameUrl, passwordUrl;
    JButton addButton, cancelButton;
    JLabel statusBar;
    JTabbedPane pan;

    // The table of managed VM (local tab)
    JTable vmTable;
    ManagedVmTableModel vmModel = null;

    public ConnectDialog(JConsole jConsole) {
	super(jConsole, Resources.getText("JConsole: Connect to Agent"), true);

	this.jConsole = jConsole;
	Container cp = getContentPane();
	
	((JComponent)cp).setBorder(new EmptyBorder(10, 10, 4, 10));
	JPanel jvmPanel = 
	    new JPanel(new VariableGridLayout(0, 1, 4, 4, false, true));
	JPanel freePanel = 
	    new JPanel(new VariableGridLayout(0, 1, 4, 4, false, true));
	jmxUrl = new JTextField(20);
	jmxUrl.addFocusListener(this);
	jmxUrl.addFocusListener(this);
	jmxUrl.addActionListener(this);
	freePanel.add(new LabeledComponent(Resources.getText("JMX URL: "),
					   jmxUrl));
	jmxUrl.setText(JConsole.ROOT_URL);

	userNameUrl = new JTextField(20);
	userNameUrl.addActionListener(this);
	userNameUrl.getDocument().addDocumentListener(this);
	userNameUrl.addFocusListener(this);
	freePanel.add(new LabeledComponent(Resources.getText("User Name: "),
					   userNameUrl));
	
	passwordUrl = new JPasswordField(20);
	passwordUrl.addActionListener(this);
	passwordUrl.getDocument().addDocumentListener(this);
	passwordUrl.addFocusListener(this);
	freePanel.add(new LabeledComponent(Resources.getText("Password: "),
					   passwordUrl));
	
	LabeledComponent.layout(freePanel);

	hostName = new JTextField(20);
	hostName.addActionListener(this);
	hostName.getDocument().addDocumentListener(this);
	hostName.addFocusListener(this);
	jvmPanel.add(new LabeledComponent(Resources.getText("Host or IP: "), 
					  hostName));

	portNumber = new JTextField(6);
	portNumber.addActionListener(this);
	portNumber.addKeyListener(this);
	portNumber.addFocusListener(this);
	jvmPanel.add(new LabeledComponent(Resources.getText("Port: "),
                                             portNumber));

	userNameTF = new JTextField(20);
	userNameTF.addActionListener(this);
	userNameTF.getDocument().addDocumentListener(this);
	userNameTF.addFocusListener(this);
	jvmPanel.add(new LabeledComponent(Resources.getText("User Name: "),
                                          userNameTF));

	passwordTF = new JPasswordField(20);
	passwordTF.addActionListener(this);
	passwordTF.getDocument().addDocumentListener(this);
	passwordTF.addFocusListener(this);
	jvmPanel.add(new LabeledComponent(Resources.getText("Password: "),
                                          passwordTF));

	LabeledComponent.layout(jvmPanel);
	
	JPanel bottomPanel = new JPanel(new BorderLayout());
	cp.add(bottomPanel, BorderLayout.SOUTH);

	JPanel buttonPanel = new JPanel();
	bottomPanel.add(buttonPanel, BorderLayout.NORTH);
	buttonPanel.add(addButton = new JButton(Resources.getText("Connect")));
	buttonPanel.add(cancelButton = 
			new JButton(Resources.getText("Cancel")));

	statusBar = new JLabel(" ", JLabel.CENTER);
	bottomPanel.add(statusBar, BorderLayout.SOUTH);

	addButton.addActionListener(this);
	cancelButton.addActionListener(this);

	pan = new JTabbedPane();

	Dimension preferredSize;
	int tabIndex = 0;

	// 
	// If the VM supports the local attach mechanism (is: Sun
	// implementation) then the Local tab is created.
	// 
	if (isLocalAttachAvailable()) {
            int tableWidth = 400;
	    preferredSize = new Dimension(tableWidth, 300);

	    vmModel = new ManagedVmTableModel();
	    vmTable = new JTable(vmModel);
	    vmTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
	    vmTable.setColumnSelectionAllowed(false);
            TableColumn pidColumn = vmTable.getColumnModel().getColumn(0);
            pidColumn.setMaxWidth(getLabelWidth("999999"));

            TableColumn cmdLineColumn = vmTable.getColumnModel().getColumn(1);
            cmdLineColumn.setMinWidth(tableWidth - pidColumn.getMaxWidth());
            int width = vmModel.getCmdLineSize();
            cmdLineColumn.setPreferredWidth(width);

	    // 
	    // If the user clicks on the table then the process list is
	    // updated. Also when the user switches back to the local
	    // tab the process list is updated.
	    // 
	    vmTable.getTableHeader().addMouseListener(new MouseAdapter() {
		public void mouseClicked(MouseEvent evt) {
		    refresh();
		}
	    });
	    pan.addChangeListener(new ChangeListener() {
        	public void stateChanged(ChangeEvent evt) {
            	    JTabbedPane pane = (JTabbedPane)evt.getSource();
		    if (pane.getSelectedIndex() == LOCAL_TAB) {
			refresh();
		    }
		}
	    });

	    JScrollPane scrollPane = new JScrollPane(vmTable);
	    pan.add(scrollPane, Resources.getText("Local"));
	    LOCAL_TAB = tabIndex++;
	} else {
	    LOCAL_TAB = -99; // not used 
	    preferredSize = new Dimension(300, 250);
	}

	pan.add(jvmPanel, Resources.getText("Remote"));
	REMOTE_TAB = tabIndex++;

	pan.add(freePanel, Resources.getText("Advanced"));
	ADVANCED_TAB = tabIndex++;

	cp.add(pan);
	setPreferredSize(preferredSize);
	pack();
	setLocationRelativeTo(jConsole);
    }

    // a label used solely for calculating the width 
    private static JLabel label = new JLabel();
    public static int getLabelWidth(String text) {
        label.setText(text);
        return (int) label.getPreferredSize().getWidth() + 1;
    }

    public void setConnectionParameters(String url,
					String host, 
					int port, 
					String userName, 
					String password, 
					String msg) {
	if(url != null && url.length() != 0) {
	    pan.setSelectedIndex(ADVANCED_TAB);
	    jmxUrl.setText(url);
	    userNameUrl.setText((userName != null) ? userName : "");
	    passwordUrl.setText((userName != null && password != null) ? 
				password : "");
	    statusBar.setText((msg != null) ? msg : "");
	    if (getPreferredSize().width > getWidth()) {
		pack();
	    }
	    if (userName != null && (password == null || msg != null)) {
		// No password or possibly failed login
		passwordUrl.requestFocus();
		passwordUrl.selectAll();
	    } else {
		jmxUrl.selectAll();
	    }
	    return;
	}

	hostName.setText(host);
	portNumber.setText(port + "");
	userNameTF.setText((userName != null) ? userName : "");
	passwordTF.setText((userName != null && password != null) ? password : "");
	
	statusBar.setText((msg != null) ? msg : "");
	if (getPreferredSize().width > getWidth()) {
	    pack();
	}
	if (userName != null && (password == null || msg != null)) {
	    // No password or possibly failed login
	    passwordTF.requestFocus();
	    passwordTF.selectAll();
	} else {
	    hostName.requestFocus();
	    hostName.selectAll();
	}
    }

    public void actionPerformed(ActionEvent ev) {
	//
	// if the Local tab is selected when the add button is pressed do not
	// hide the connect dialog - instead we keep it visible until a
	// connection has been established to all selected VMs.
	//
	if (!(ev.getSource() == addButton && pan.getSelectedIndex() == LOCAL_TAB)) {
	    setVisible(false);
	}

	statusBar.setText("");

	if (ev.getSource() != cancelButton) {
	    if(pan.getSelectedIndex() == REMOTE_TAB) {
		String host = hostName.getText().trim();
		String port = portNumber.getText().trim();
		String userName = userNameTF.getText().trim();
		String password  = passwordTF.getText();
		
		if (userName.length() == 0 || password.length() == 0) {
		    password  = null;
		}
		if (userName.length() == 0) {
		    userName = null;
		}
		
		if (host.length() > 0 && port.length() > 0) {
		    try {
			int p = Integer.parseInt(port.trim());
			jConsole.addHost(host, p, userName, password);
			hostName.setText("");
			portNumber.setText("");
			userNameTF.setText("");
			passwordTF.setText("");
			return;
		    } catch (Exception ex) {
			statusBar.setText(ex.toString());
		    }
		}
		setVisible(true);
	    } else {
		if(pan.getSelectedIndex() == ADVANCED_TAB) {
		    String url = jmxUrl.getText().trim();
		    String msg = null;
		    
		    if (url.length() > 0) {
			String userName = userNameUrl.getText().trim();
			String password  = passwordUrl.getText();
			
			if (userName.length() == 0 || password.length() == 0) {
			    password  = null;
			}
			if (userName.length() == 0) {
			    userName = null;
			}
			try {
			    ConnectionParameters parameters = 
				jConsole.createParameters(new JMXServiceURL(url));
			    if(password != null && userName != null) {
				if(parameters.map == null)
				    parameters.map = new HashMap();
				
				String[] credentials = {userName, password};
				parameters.map.put(JMXConnector.CREDENTIALS, 
						   credentials);
			    }
			    jConsole.addUrl(parameters.jmxUrl, 
					    parameters.map,
					    userName,
					    password,
					    false);
			    jmxUrl.setText(JConsole.ROOT_URL);
			    userNameUrl.setText("");
			    passwordUrl.setText("");
			    return;
			} catch (MalformedURLException ex) {
			    msg = Resources.getText("Invalid URL") + 
						    ex.getMessage();
			} catch (IOException ex) {
			    msg = Resources.getText("Connection failed");
			} catch (Exception ex) {
			    msg = ex.toString();
			}
			
			statusBar.setText(msg);
		    }
		    setVisible(true);
		} else {
		    // 
		    // Try to connect to all selected VMs. If a connection
		    // cannot be established for some reason (the process has
		    // terminated for example) then keep the dialog open showing
		    // the connect error.
		    //
		    if (pan.getSelectedIndex() == LOCAL_TAB) {
			Exception exc = null;
	   		int selected[] = vmTable.getSelectedRows();
	   		for (int i=0; i<selected.length; i++) {
			    ManagedVirtualMachine vm = vmModel.vmAt(selected[i]);
			    try {
                    		ProxyClient proxyClient =
				    ProxyClient.getProxyClient(vm.vmid(), vm.connectorAddress());
                    	        jConsole.addVmid(vm.vmid(), proxyClient);
			    } catch (java.io.IOException ioe) {
                    		exc = ioe;
			    }
			}	
           	        if (selected.length > 0 && exc == null) {
               		    setVisible(false);
		        } else {
			    if (exc != null) {
               			statusBar.setText(exc.getMessage());
				vmModel.refresh();
			    }
			}
		    }
		}
	    }
	}
    }

    public void itemStateChanged(ItemEvent ev) {
    }

    public void insertUpdate(DocumentEvent e) {
	final String str = hostName.getText();
	if (str.endsWith(":")) {
	    // Need to delay this to make focus stick
	    SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			hostName.setText(str.substring(0, str.length() - 1));
			portNumber.requestFocus();
		    }
		});
	}
    }

    public void removeUpdate(DocumentEvent e) {
    }

    public void changedUpdate(DocumentEvent e) {
    }

    public void focusGained(FocusEvent e) {
	Object source = e.getSource();
	Component opposite = e.getOppositeComponent();

	if (!e.isTemporary() &&
	    source instanceof JTextField &&
	    opposite instanceof JComponent &&
	    SwingUtilities.getWindowAncestor(opposite) == this) {

	    ((JTextField)source).selectAll();
	}
    }

    public void focusLost(FocusEvent e) {
    }

    public void keyTyped(KeyEvent e) {
	char c = e.getKeyChar();      
	if (c == KeyEvent.VK_ESCAPE) {
	    setVisible(false);
	} else if (!(Character.isDigit(c) ||
		     c == KeyEvent.VK_BACK_SPACE ||
		     c == KeyEvent.VK_DELETE)) {
	    getToolkit().beep();
	    e.consume();
	}
    }

    public void setVisible(boolean b) {
	boolean wasVisible = isVisible();
	super.setVisible(b);
	if (b && !wasVisible) {
	    // Need to delay this to make focus stick
	    SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    if(pan.getSelectedIndex() == REMOTE_TAB) {
			hostName.requestFocus();
			hostName.selectAll();
		    } else {
			jmxUrl.requestFocus();
			jmxUrl.selectAll();
		    }
		}
	    });
	}
    }

    public void keyPressed(KeyEvent e) {
    }

    public void keyReleased(KeyEvent e) {
    }

    // Refresh the list of managed VMs
    public void refresh() {
	if (vmModel != null) {
	    vmModel.refresh();

            // adjust the preferred size
            int width = vmModel.getCmdLineSize();
	    vmTable.getColumnModel().getColumn(1).setPreferredWidth(width);
            pack();

	    // if there's only one process then select the row
	    if (vmModel.getRowCount() == 1) {
		vmTable.addRowSelectionInterval(0, 0);
            }

	}
    }

    // Indicates if this VM can attach to processes
    private static boolean isLocalAttachAvailable() {
	try {
	    Class.forName("sun.jvmstat.monitor.MonitoredHost");
	    return true;
	} catch (NoClassDefFoundError x) {	
	    return false;
	} catch (ClassNotFoundException x) {
	    return false;
	}
    }

    // Represents the list of managed VMs as a tabular data model.
    private static class ManagedVmTableModel extends AbstractTableModel {
	private static String[] columnNames = {
            Resources.getText("Column.PID"),
            Resources.getText("Column.Class and Arguments") 
        };
	
	private ArrayList<ManagedVirtualMachine> vmList;
        private int longestCmdLineLength = 0;
        private String longestCmdLine = "";
	    
        public int getColumnCount() { 
	    return columnNames.length; 
	}
		
	public String getColumnName(int col) {
	    return columnNames[col];
	}
	
        public synchronized int getRowCount() { 
	    return vmList.size();
	}
	
        public synchronized Object getValueAt(int row, int col) {
	    assert col >= 0 && col <= 1;
	    ManagedVirtualMachine vm = vmList.get(row);
	    if (col == 0) {
		return Integer.toString(vm.vmid());
	    } else {
		return vm.commandLine();
	    }     
	}
	    
	public ManagedVmTableModel() {	
	    refresh();
	}

	
	public synchronized ManagedVirtualMachine vmAt(int pos) {
	    return vmList.get(pos);
	}
	
	public synchronized void refresh() {	        
	    vmList = getManagedVirtualMachines();
		
	    // data has changed
	    fireTableDataChanged();
	}
       
        // return the width of the column displaying the classname 
        // and command line arguments
        public int getCmdLineSize() {
            return getLabelWidth(longestCmdLine);
        }

        private ArrayList<ManagedVirtualMachine> getManagedVirtualMachines() {
	    Set activeVms;
	    MonitoredHost host;
	    try {
	        host = MonitoredHost.getMonitoredHost(new HostIdentifier((String)null));
	        activeVms = host.activeVms();
	    } catch (java.net.URISyntaxException sx) {
	        throw new InternalError(sx.getMessage());
	    } catch (sun.jvmstat.monitor.MonitorException mx) {
	        throw new InternalError(mx.getMessage());
	    }

	    ArrayList l = new ArrayList<ManagedVirtualMachine>();
	    for (Object vm: activeVms) {
	        try { 
		    int vmid = (Integer)vm;
	            String address = ConnectorAddressLink.importFrom(vmid);
		    if (address == null) {
		        // not managed
		        continue;
		    }	
		    VmIdentifier vmId = new VmIdentifier(Integer.toString(vmid));
		    String cmdLine = 
		        MonitoredVmUtil.commandLine(host.getMonitoredVm(vmId));
                    int len = cmdLine.length();
                    if (len > longestCmdLineLength) { 
                        longestCmdLineLength = len;
                        longestCmdLine = cmdLine;
                    }
		    l.add(new ManagedVirtualMachine(vmid, address, cmdLine));
	        } catch (Exception x) {
	        }
 	    }
	    return l;
	}
    }
   
    // Represents a managed virtual machine 
    private static class ManagedVirtualMachine {
	private int vmid;
	private String address;
	private String cmdLine;
	
	ManagedVirtualMachine(int vmid, String address, String cmdLine) {
            this.vmid = vmid;
            this.address = address;
	    this.cmdLine = cmdLine;
	}
	
	public int vmid() {
            return vmid;
	}
	
	public String connectorAddress() {
            return address;
	}	
	
	public String commandLine() {
	    return cmdLine;
	}
    }
}
