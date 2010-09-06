/*
 * @(#)JConsole.java	1.39 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyVetoException;
import java.io.*;
import java.net.MalformedURLException;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.HashMap;

import java.net.URLDecoder;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.management.remote.JMXServiceURL;
import javax.management.remote.JMXConnector;

// Sun implementation specific
import sun.management.ConnectorAddressLink;

public class JConsole extends JFrame
    implements ActionListener, InternalFrameListener {

    private final static String title =
	Resources.getText("J2SE 5.0 Monitoring & Management Console");
    public final static String ROOT_URL =
	"service:jmx:";
    
    private static int updateInterval = 4000;
    
    private boolean mdiCreated = false;

    private JMenuBar menuBar;
    private JMenuItem hotspotMI, connectMI, exitMI;
    private JMenu windowMenu;
    private JMenuItem tileMI, cascadeMI, minimizeAllMI, restoreAllMI;

    private JToolBar toolBar;
    private JButton connectButton;
    private JDesktopPane desktop;
    private JPanel statusPanel;
    private JLabel statusLabel;
    private JDialog connectDialog;
    private CreateMBeanDialog createDialog;

    private ArrayList<VMInternalFrame> windows = 
	new ArrayList<VMInternalFrame>();

    private int frameLoc = 5;
    
    public JConsole(boolean hotspot) {
	super(title);
	setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	menuBar = new JMenuBar();
	setJMenuBar(menuBar);

	// TODO: Use Actions !

	JMenu connectionMenu = new JMenu(Resources.getText("Connection"));
	//connectionMenu.setMnemonic(KeyEvent.VK_V);
	menuBar.add(connectionMenu);
	if(hotspot) {
	    hotspotMI = new JMenuItem("Hotspot MBeans...");
	    hotspotMI.setAccelerator(KeyStroke.
				     getKeyStroke(KeyEvent.VK_H,
						  InputEvent.CTRL_MASK));
	    hotspotMI.addActionListener(this);
	    connectionMenu.add(hotspotMI);

	    connectionMenu.addSeparator();
	}

	connectMI = new JMenuItem(Resources.getText("New Connection..."));
	connectMI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
							InputEvent.CTRL_MASK));
	connectMI.addActionListener(this);
	connectionMenu.add(connectMI);

	connectionMenu.addSeparator();

	exitMI = new JMenuItem(Resources.getText("Exit"));
	exitMI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F4,
						     InputEvent.ALT_MASK));
	exitMI.addActionListener(this);
	connectionMenu.add(exitMI);

	JToolBar toolBar = new JToolBar();
	connectButton = new JButton(Resources.getText("Connect..."));
	connectButton.addActionListener(this);
	toolBar.add(connectButton);

	statusPanel = new JPanel();
	statusPanel.add(statusLabel = new JLabel());

	Container cp = getContentPane();
	//cp.add(toolBar,     BorderLayout.NORTH);
	cp.add(statusPanel, BorderLayout.SOUTH);
    }
    
    public List<VMInternalFrame> getInternalFrames() {
	return windows;
    }

    private void createMDI() {
	// Restore title - we now show connection name on internal frames
	setTitle(title);

	Container cp = getContentPane();
	Component oldCenter =
	    ((BorderLayout)cp.getLayout()).
	    getLayoutComponent(BorderLayout.CENTER);

	windowMenu = new JMenu(Resources.getText("Window"));
	//windowMenu.setMnemonic(KeyEvent.VK_W);
	menuBar.add(windowMenu);

	cascadeMI = new JMenuItem(Resources.getText("Cascade"));
	cascadeMI.addActionListener(this);
	windowMenu.add(cascadeMI);

	tileMI = new JMenuItem(Resources.getText("Tile"));
	tileMI.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T,
						     InputEvent.CTRL_MASK));
	tileMI.addActionListener(this);
	windowMenu.add(tileMI);

	minimizeAllMI = new JMenuItem(Resources.getText("Minimize All"));
	minimizeAllMI.addActionListener(this);
	windowMenu.add(minimizeAllMI);

	restoreAllMI = new JMenuItem(Resources.getText("Restore All"));
	restoreAllMI.addActionListener(this);
	windowMenu.add(restoreAllMI);

	windowMenu.addSeparator();

	desktop = new JDesktopPane();
	desktop.setBackground(new Color(235, 245, 255));

	cp.add(desktop, BorderLayout.CENTER);

	if (oldCenter instanceof VMPanel) {
	    addFrame((VMPanel)oldCenter);
	}

	mdiCreated = true;
    }


    public void actionPerformed(ActionEvent ev) {
	Object src = ev.getSource();
	if (src == hotspotMI) {
	    showCreateMBeanDialog();
	}

	if (src == connectButton || src == connectMI) {
	    VMPanel vmPanel = null;
	    if (mdiCreated) {
		JInternalFrame vmIF = desktop.getSelectedFrame();
		if (vmIF instanceof VMInternalFrame) {
		    vmPanel = ((VMInternalFrame)vmIF).getVMPanel();
		}
	    } else {
		BorderLayout layout = 
		    (BorderLayout)getContentPane().getLayout();
		vmPanel = 
		    (VMPanel)layout.getLayoutComponent(BorderLayout.CENTER);
	    }
	    String hostName = "";
	    String url = "";
	    if (vmPanel != null) {
		hostName = vmPanel.getHostName();
		if(vmPanel.getUrl() != null)
		    url = vmPanel.getUrl();
	    }
	    showConnectDialog(url, hostName, 0, null, null, null);
	} else if (src == tileMI) {
	    tileWindows();
	} else if (src == cascadeMI) {
	    cascadeWindows();
	} else if (src == minimizeAllMI) {
	    for (VMInternalFrame vmIF : windows) {
		try {
		    vmIF.setIcon(true);
		} catch (PropertyVetoException ex) {
		    // Ignore
		}
	    }
	} else if (src == restoreAllMI) {
	    for (VMInternalFrame vmIF : windows) {
		try {
		    vmIF.setIcon(false);
		} catch (PropertyVetoException ex) {
		    // Ignore
		}
	    }
	} else if (src == exitMI) {
	    System.exit(0);
	} else if (src instanceof JMenuItem) {
	    JMenuItem mi = (JMenuItem)src;
	    VMInternalFrame vmIF = (VMInternalFrame)mi.
		getClientProperty("JConsole.vmIF");
	    if (vmIF != null) {
		try {
		    vmIF.setIcon(false);
		    vmIF.setSelected(true);
		} catch (PropertyVetoException ex) {
		    // Ignore
		}
		vmIF.moveToFront();
	    }
	}
    }

    void vmPanelDied(VMPanel vmPanel) {
	if (windows.size() == 0) {
	    JComponent cp = (JComponent)getContentPane();
	    Component comp = ((BorderLayout)cp.getLayout()).
		getLayoutComponent(BorderLayout.CENTER);
	    if (comp == vmPanel) {
		cp.remove(vmPanel);
		cp.repaint();
	    }
	} else {
	    Iterator<VMInternalFrame> iterator = windows.iterator();
	    while (iterator.hasNext()) {
		VMInternalFrame vmIF = iterator.next();
		VMPanel p = vmIF.getVMPanel();
		if (p == vmPanel) {
		    removeVMInternalFrame(vmIF);
		    iterator.remove();
		    break;
		}
	    }
	}
	showConnectDialog(vmPanel.getUrl(), 
			  vmPanel.getHostName(), 
			  vmPanel.getPort(),
			  vmPanel.getUserName(), 
			  vmPanel.getPassword(),
			  null);
    }


    public void tileWindows() {
	if (!mdiCreated) {
	    return;
	}
	int w = -1;
	int h = -1;
	int n = 0;
	for (VMInternalFrame vmIF : windows) {
	    if (!vmIF.isIcon()) {
		n++;
		if (w == -1) {
		    try {
			vmIF.setMaximum(true);
			w = vmIF.getWidth();
			h = vmIF.getHeight();
		    } catch (PropertyVetoException ex) {
			// Ignore
		    }
		}
	    }
	}
	if (n > 0 && w > 0 && h > 0) {
	    int rows = (int)Math.ceil(Math.sqrt(n));
	    int cols = n / rows;
	    if (rows * cols < n) cols++;
	    int x = 0;
	    int y = 0;
	    w /= cols;
	    h /= rows;
	    int col = 0;
	    for (VMInternalFrame vmIF : windows) {
		if (!vmIF.isIcon()) {
		    try {
			vmIF.setMaximum(n==1);
		    } catch (PropertyVetoException ex) {
			// Ignore
		    }
		    if (n > 1) {
			vmIF.setBounds(x, y, w, h);
		    }
		    if (col < cols-1) {
			col++;
			x += w;
		    } else {
			col = 0;
			x = 0;
			y += h;
		    }
		}
	    }
	}
    }

    public void cascadeWindows() {
	int n = 0;
	int w = -1;
	int h = -1;
	for (VMInternalFrame vmIF : windows) {
	    if (!vmIF.isIcon()) {
		try {
		    vmIF.setMaximum(false);
		} catch (PropertyVetoException ex) {
		    // Ignore
		}
		n++;
		vmIF.pack();
		if (w == -1) {
		    try {
			w = vmIF.getWidth();
			h = vmIF.getHeight();
			vmIF.setMaximum(true);
			w = vmIF.getWidth() - w;
			h = vmIF.getHeight() - h;
			vmIF.pack();
		    } catch (PropertyVetoException ex) {
			// Ignore
		    }
		}		    
	    }
	}
	int x = 0;
	int y = 0;
	int dX = (n > 1) ? (w / (n - 1)) : 0;
	int dY = (n > 1) ? (h / (n - 1)) : 0;
	for (VMInternalFrame vmIF : windows) {
	    if (!vmIF.isIcon()) {
		vmIF.setLocation(x, y);
		vmIF.moveToFront();
		x += dX;
		y += dY;
	    }
	}
    }
    
    void addHost(String hostName, int port,
		 String userName, String password) {
	addHost(hostName, port, userName, password, false);
    }

    void addVmid(final int vmid, final ProxyClient proxyClient) {
	addVmid(vmid, proxyClient, false);
    }

    void addVmid(final int vmid, final ProxyClient proxyClient, final boolean tile) {
        new Thread("JConsole.addVmid") {
		public void run() {
		    try {
			try {
			    SwingUtilities.invokeAndWait(new Runnable() {
				    public void run() {
					addFrame(proxyClient);
				    }
				});
			    if (tile) {
				SwingUtilities.invokeLater(new Runnable() {
					public void run() {
					    tileWindows();
					}
				    });
			    }
			} catch (InvocationTargetException ex) {
			    // Ignore
			} catch (InterruptedException ex) {
			    // Ignore
			}
		    } catch (final SecurityException ex) {
			failed(ex);
		    }
		}

		private void failed(final Exception ex) {
		    SwingUtilities.invokeLater(new Runnable() {
			    public void run() {
				String msg = (ex instanceof IOException) ? 
				    Resources.getText("Connection failed")
				    : ex.getMessage();
				System.err.println(msg);
			    }
			});
		}
	    }.start();
    }    
    
    void addUrl(final JMXServiceURL url, 
		final Map map, 
		final String userName,
		final String password,
		final boolean tile) {
	new Thread("JConsole.addUrl") {       
		public void run() {
		    try {
			final ProxyClient proxyClient =
			    ProxyClient.getProxyClient(url, map);
			
			try {
			    SwingUtilities.invokeAndWait(new Runnable() {
				    public void run() {
					addFrame(proxyClient);
				    }
				});
			    if (tile) {
				SwingUtilities.invokeLater(new Runnable() {
					public void run() {
					    tileWindows();
					}
				    });
			    }
			} catch (InvocationTargetException ex) {
			    // Ignore
			} catch (InterruptedException ex) {
			    // Ignore
			}
		    } catch (final SecurityException ex) {
			failed(ex);
		    } catch (final IOException ex) {
			failed(ex);
		    }
		}
		
		private void failed(final Exception ex) {
		    SwingUtilities.invokeLater(new Runnable() {
			    public void run() {
				String msg = (ex instanceof IOException) ? 
				    Resources.getText("Connection failed") : 
				    ex.getMessage();
				showConnectDialog(url.toString(), 
						  null, 
						  -1, 
						  userName, 
						  password, 
						  msg);
			    }
			});
		}
	    }.start();
    }

    void addHost(final String hostName, final int port,
		 final String userName, final String password,
		 final boolean tile) {
	new Thread("JConsole.addHost") {
		public void run() {
		    try {
			final ProxyClient proxyClient =
			    ProxyClient.getProxyClient(hostName, port,
						       (userName != null) ? 
						       userName : "",
						       (password != null) ? 
						       password : "");

			try {
			    SwingUtilities.invokeAndWait(new Runnable() {
				    public void run() {
					addFrame(proxyClient);
				    }
				});
			    if (tile) {
				SwingUtilities.invokeLater(new Runnable() {
					public void run() {
					    tileWindows();
					}
				    });
			    }
			} catch (InvocationTargetException ex) {
			    // Ignore
			} catch (InterruptedException ex) {
			    // Ignore
			}
		    } catch (final SecurityException ex) {
			failed(ex);
		    } catch (final IOException ex) {
			failed(ex);
		    }
		}

		private void failed(final Exception ex) {
		    SwingUtilities.invokeLater(new Runnable() {
			    public void run() {
				String msg = (ex instanceof IOException) ? 
				    Resources.getText("Connection failed")
				    : ex.getMessage();
				showConnectDialog(null, 
						  hostName, 
						  port, 
						  userName, 
						  password, 
						  msg);
			    }
			});
		}
	    }.start();
    }

    private void addFrame(ProxyClient proxyClient) {
	VMPanel vmPanel = new VMPanel(proxyClient, updateInterval);
	if (!mdiCreated) {
	    JComponent cp = (JComponent)getContentPane();
	    if (((BorderLayout)cp.getLayout()).
		getLayoutComponent(BorderLayout.CENTER) == null) {
		String str = vmPanel.getConnectionName();
		if (str.equals("localhost:0")) {
		    str = Resources.getText("Monitoring Self", str);
		}
		setTitle(title + ": " + str);
		cp.add(vmPanel, BorderLayout.CENTER);
		cp.revalidate();
		return;
	    } else {
		createMDI();
	    }
	}
	addFrame(vmPanel);
    }

    private void addFrame(VMPanel vmPanel) {
	VMInternalFrame vmIF = new VMInternalFrame(vmPanel);

	for (VMInternalFrame f : windows) {
	    try {
		f.setMaximum(false);
	    } catch (PropertyVetoException ex) {
		// Ignore
	    }
	}
	desktop.add(vmIF);
	vmIF.setLocation(frameLoc, frameLoc);
	frameLoc += 30;
	vmIF.setVisible(true);
	windows.add(vmIF);
	if (windows.size() == 1) {
	    try {
		vmIF.setMaximum(true);
	    } catch (PropertyVetoException ex) {
		// Ignore
	    }
	}
	vmIF.addInternalFrameListener(this);
	JMenuItem mi = new JMenuItem(vmPanel.getConnectionName());
	mi.putClientProperty("JConsole.vmIF", vmIF);
	mi.addActionListener(this);
	vmIF.putClientProperty("JConsole.menuItem", mi);
	windowMenu.add(mi);
    }


    private void showConnectDialog(String url, 
				   String hostName, 
				   int port,
				   String userName, 
				   String password,
				   String msg) {
	if (connectDialog == null) {
	    connectDialog = new ConnectDialog(this);
	    
	    statusLabel.setText("");
	}
	((ConnectDialog)connectDialog).setConnectionParameters(url,
							       hostName, 
							       port, 
							       userName, 
							       password, 
							       msg);

	((ConnectDialog)connectDialog).refresh();
	connectDialog.setVisible(true);
    }
    
    private void showCreateMBeanDialog() {
	if (createDialog == null) {
	    createDialog = new CreateMBeanDialog(this);
	}
	statusLabel.setText("");
	createDialog.setVisible(true);
    }

    private void removeVMInternalFrame(VMInternalFrame vmIF) {
	JMenuItem mi = (JMenuItem)vmIF.getClientProperty("JConsole.menuItem");
	windowMenu.remove(mi);
	mi.putClientProperty("JConsole.vmIF", null);
	vmIF.putClientProperty("JConsole.menuItem", null);
	desktop.remove(vmIF);
	desktop.repaint();
	vmIF.getVMPanel().disconnect();
	vmIF.dispose();
    }

    private boolean isProxyClientUsed(ProxyClient client) {
	for(VMInternalFrame frame : windows) {
	    ProxyClient cli = frame.getVMPanel().getProxyClient(false);
	    if(client == cli)
		return true;
	}
	return false;
    }

    // InternalFrameListener interface

    public void internalFrameClosing(InternalFrameEvent e) {
	VMInternalFrame vmIF = (VMInternalFrame)e.getInternalFrame();
	removeVMInternalFrame(vmIF);
	windows.remove(vmIF);
	ProxyClient client = vmIF.getVMPanel().getProxyClient(false);
	if(!isProxyClientUsed(client))
	    client.markAsDead();
    }

    public void internalFrameOpened(InternalFrameEvent e) {}
    public void internalFrameClosed(InternalFrameEvent e) {}
    public void internalFrameIconified(InternalFrameEvent e) {}
    public void internalFrameDeiconified(InternalFrameEvent e) {}
    public void internalFrameActivated(InternalFrameEvent e) {}
    public void internalFrameDeactivated(InternalFrameEvent e) {}


    void setTabTransparency(boolean transparent) {
	if (mdiCreated) {
	    for (VMInternalFrame vmIF : windows) {
		vmIF.getVMPanel().setTransparency(transparent);
	    }
	} else {
	    BorderLayout layout = (BorderLayout)getContentPane().getLayout();
	    VMPanel vmPanel = 
		(VMPanel)layout.getLayoutComponent(BorderLayout.CENTER);
	    if (vmPanel != null) {
		vmPanel.setTransparency(transparent);
	    }
	}
    }


    private static void usage() {
	System.err.println(Resources.getText("zz usage text", "jconsole"));
	System.exit(1);
    }

    private static void mainInit(final List<ConnectionParameters> connectors,
				 final List<String> hostNames, 
				 final List<Integer> ports, 
				 final int vmid, 
				 final ProxyClient proxyClient,
				 final boolean noTile,
				 final boolean hotspot) {


	// Always create Swing GUI on the Event Dispatching Thread
	SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    JConsole jConsole = new JConsole(hotspot);
		    jConsole.setSize(900, 750);
		    jConsole.setLocationRelativeTo(null);
		    jConsole.setVisible(true);
		
		    if( (hostNames != null && hostNames.size() > 1) ||
			(connectors != null && connectors.size() > 1) )
			jConsole.createMDI();
		    
		    if(hostNames != null) {
			for (int i = 0; i < hostNames.size(); i++) {
			    jConsole.addHost(hostNames.get(i), ports.get(i),
					     null, null,
					     (i == hostNames.size() - 1) ? 
					     !noTile : false);
			}
		    } 
		    
		    if(connectors != null) {
			for (int i = 0; i < connectors.size(); i++) {
			    ConnectionParameters p = connectors.get(i);
			    jConsole.addUrl(p.jmxUrl, 
					    p.map, 
					    null,
					    null,
					    (i == connectors.size() - 1) ? 
					    !noTile : false);
			}
		    }
		    
		    if (vmid >= 0) {
		        assert proxyClient != null;	
		        jConsole.addVmid(vmid, proxyClient, !noTile);
		    } 
		    
		    if(vmid < 0 && hostNames == null && connectors == null)
			jConsole.showConnectDialog(null,
						   "localhost", 
						   0, 
						   null, 
						   null, 
						   null);
		}
	    });
    }
    
    static ConnectionParameters createParameters(JMXServiceURL jmxUrl) 
	throws Exception {
	//The handling of URL attributes is done here
	//Not yet supported.
	//
	ConnectionParameters parameters = 
	    new ConnectionParameters();
	parameters.jmxUrl = jmxUrl;
	parameters.map = null;
	
	return parameters;
    }
    
    public static void main(String[] args) {
	boolean noTile = false, hotspot = false;
	int argIndex = 0;
	int vmid = -1;
	ProxyClient proxyClient = null;

	while (args.length - argIndex > 0 && args[argIndex].startsWith("-")) {
	    String arg = args[argIndex++];
	    if (arg.equals("-h") ||
		arg.equals("-help") ||
		arg.equals("-?")) {

		usage();
	    } else if (arg.startsWith("-interval=")) {
		try {
		    updateInterval = Integer.parseInt(arg.substring(10)) * 
			1000;
		} catch (NumberFormatException ex) {
		    usage();
		}
	    } else if (arg.equals("-notile")) {
		noTile = true;
	    } else if (arg.equals("-version")) {
                Version.print(System.err);
		System.exit(0);
	    } else if (arg.equals("-fullversion")) {
                Version.printFullVersion(System.err);
		System.exit(0);
	    } else if(arg.equals("-hotspot")) {
		hotspot = true;
	    }else {
		// Unknown switch
		usage();
	    }
	}
	
	ArrayList<ConnectionParameters> params = null;
	ArrayList<String> hostNames = null;
	ArrayList<Integer> ports = null;
	int args_remaining = args.length - argIndex;

	// if there's one argument we initially assume it's a 
	// vmid (process id). If it doesn't parse as a number then
	// it will be parsed as a hostname:port later on.
	if (args_remaining == 1) {
	    try {
		vmid = Integer.parseInt(args[argIndex]);
		args_remaining = 0;
		
		try {
		    // Use Sun implementation specific method to obtain 
		    // JMX connector address for the given process.
		    // If run on a non-Sun implementation an exception will
		    // be thrown.
		    String address = ConnectorAddressLink.importFrom(vmid);
		    if (address == null) {
		        System.err.println(vmid + " is not a managed VM.");
			System.exit(1);
		    }
		    proxyClient = ProxyClient.getProxyClient(vmid, address);							
	        } catch (IOException ioe) {
		    System.err.println("Unable to attach to " + vmid + 
		    	": " + ioe.getMessage());
		    System.exit(1);
		} catch (NoClassDefFoundError x) {		
		    vmid = -1;  // feature unavailable
		}	   			    
	    } catch (NumberFormatException ex) {
	    }
	}


	if (args_remaining > 0) {
	    hostNames = new ArrayList<String>();
	    ports     = new ArrayList<Integer>();
	    params = new ArrayList<ConnectionParameters>();
	    int size = args.length - argIndex;
	    for (int i = 0; i < size; i++) {
		
		String arg = args[argIndex+i];
		try {
		    JMXServiceURL url = new JMXServiceURL(arg);
		    params.add(createParameters(url));
		    continue;
		}catch(Exception e) {
		}

		int p = arg.indexOf(':');
		
		if (p > 0) {
		    hostNames.add(arg.substring(0, p));
		    try {
			ports.add(Integer.parseInt(arg.substring(p+1)));
		    } catch (NumberFormatException ex) {
			usage();
		    }
		} else {
		    usage();
		}
	    }
	}

	mainInit(params, hostNames, ports, vmid, proxyClient, noTile, hotspot);
    }
}
