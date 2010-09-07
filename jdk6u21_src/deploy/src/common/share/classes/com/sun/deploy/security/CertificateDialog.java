/*
 * @(#)CertificateDialog.java	1.45 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Window;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.Principal;
import java.security.MessageDigest;
import java.text.MessageFormat;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.JTextArea;
import javax.swing.JViewport;
import javax.swing.ListSelectionModel;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import sun.security.x509.SerialNumber;
import sun.misc.HexDumpEncoder;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.resources.ResourceManager;


public class CertificateDialog {

    public static void showCertificates(JDialog parent, Certificate[] certs, 
            int start, int end) {
        
        // Construct new modal dialog
        final JDialog details = new JDialog(parent, 
	    ResourceManager.getMessage("cert.dialog.caption"), true);
        details.getContentPane().setLayout(new BorderLayout());
        details.getContentPane().add(getComponents(parent, certs, start, end), 
                BorderLayout.CENTER);
        
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.TRAILING));
        JButton closeButton = new JButton(getMessage("cert.dialog.close")); 
        closeButton.setMnemonic(getAcceleratorKey("cert.dialog.close")); 
        closeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e){
                details.setVisible(false);
            }
        });
        buttonPanel.add(closeButton);
        
        details.getContentPane().add(buttonPanel, BorderLayout.SOUTH);
        
        details.pack();
        details.setLocationRelativeTo(parent);
        details.setResizable(false);
        
        // Show dialog
	if (Trace.isAutomationEnabled() == false) {
            details.setVisible(true);
        }	    
    }
    
    private static JPanel getComponents(JDialog parent, Certificate[] certs, 
            int start, int end){
        
        if (certs.length > start && certs[start] instanceof X509Certificate) {        	   
            final JTable certInfoTable = new JTable();
            final JTextArea textArea = new JTextArea();

            // Create borders
            Border etchBorder = BorderFactory.createEtchedBorder();
    
            // Build cert chain tree view
            final JTree certChainTree = buildCertChainTree(certs, start, end);

            certChainTree.addTreeSelectionListener(new TreeSelectionListener() {
                public void valueChanged(TreeSelectionEvent e) 
                {
                    DefaultMutableTreeNode node = 
                            (DefaultMutableTreeNode) certChainTree.
                             getLastSelectedPathComponent();
    
                    if (node != null) {
                        CertificateInfo certInfo = (CertificateInfo) 
                        node.getUserObject();
                        // Show certificate in the cert info panel
                        showCertificateInfo(
                                certInfo.getCertificate(), 
                                certInfoTable, textArea);
                    }
                }
            });	    
    
            // Build cert info table
            showCertificateInfo((X509Certificate) certs[start], certInfoTable, 
                    textArea);
    
           // Select one row at a time in cert info table
            certInfoTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    
            // Hook up selection model with cert info table
            ListSelectionModel rowSM = certInfoTable.getSelectionModel();
            rowSM.addListSelectionListener(new ListSelectionListener() {
                public void valueChanged(ListSelectionEvent e){
                    int row = certInfoTable.getSelectedRow();
                    if (row >= 0) {
                        String value = (String)certInfoTable.getValueAt(row, 1);
			        
                        // Update text area when selection changes
                        textArea.setText(value);
                        textArea.repaint();
                    }
                }   	    
            });
   
            // Create JTextArea for rhs	    
            textArea.setLineWrap(false);
            textArea.setEditable(false);
            textArea.setRows(10);
            textArea.setColumns(40);
                    
            // Get font from ResourceManager, and create new font 
            // with name="Monospace" - display in a fixed-width font.
            Font f = ResourceManager.getUIFont();
            Font fixedWidthFont = 
                    new Font("Monospaced", Font.PLAIN, f.getSize());
            textArea.setFont(fixedWidthFont);
            int fontHeight = f.getSize();
                		   
            // Select last row by default
            certInfoTable.setRowSelectionInterval(8, 8);
    
            // Create cert info panel
            JPanel panelInfo = new JPanel();
            panelInfo.setLayout(new BorderLayout());
    
            // Workaround JTable problem 
            Dimension dim = certInfoTable.getPreferredScrollableViewportSize();
            int h = Math.max(145, 8 + (9 * fontHeight));
            dim.setSize(dim.getWidth(), h);
            certInfoTable.setPreferredScrollableViewportSize(dim);
    
            JScrollPane scrollPane = new JScrollPane(certInfoTable);
            scrollPane.setBorder(BorderFactory.createCompoundBorder(
			BorderFactory.createEmptyBorder(0, 0, 5, 0),
			scrollPane.getBorder()));
    
            panelInfo.add(scrollPane, BorderLayout.CENTER);
            panelInfo.add(new JScrollPane(textArea), BorderLayout.SOUTH);
            panelInfo.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 0));
    
            // Add tree to the panel
            JPanel panel = new JPanel();
            panel.setLayout(new BorderLayout());
    
            // Workaround JTree problem
            dim = certChainTree.getPreferredScrollableViewportSize();
            dim.setSize(200.0, 100.0);
            scrollPane = new JScrollPane(certChainTree);
            scrollPane.setPreferredSize(dim);
    
            panel.add(scrollPane, BorderLayout.WEST);	
            panel.add(panelInfo, BorderLayout.EAST);  	
            
            return panel;
        }	
        return new JPanel();
    }

    /**
     * Break down DN string into multi-line
     */
    private static String formatDNString(String dnString)
    {
	int len = dnString.length();
	int last = 0;
	boolean inQuote = false;

	StringBuffer buffer = new StringBuffer();

	for (int i=0; i < len; i++)
	{
	    char ch = dnString.charAt(i);

	    // Check if we are in quote
	    if (ch == '\"' || ch == '\'')
		inQuote = !inQuote;

	    if (ch == ',' && inQuote == false)
		buffer.append(",\n");
	    else
		buffer.append(ch);
	}

	return buffer.toString();
    }

    /**
     * Method to reflect certificate chain in the tree view
     */
    private static JTree buildCertChainTree(Certificate cert[], 
					    int start, int end) {
	DefaultMutableTreeNode root = null;
	DefaultMutableTreeNode currentNode = null;
	
	for (int i=start; i < cert.length && i < end; i++)
	{
	    DefaultMutableTreeNode childNode = new DefaultMutableTreeNode(
		new CertificateInfo((X509Certificate) cert[i]));

	    if (root == null)
	    {
		root = childNode;
		currentNode = childNode;
	    }
	    else
	    {
		currentNode.add(childNode);
		currentNode = childNode;
	    }
	}
	
	JTree tree = new JTree(root);

	// Disable HTML rendering for node in JTree
	DefaultTreeCellRenderer noHTMLRenderer = new DefaultTreeCellRenderer();
        noHTMLRenderer.putClientProperty("html.disable", Boolean.TRUE);
	tree.setCellRenderer(noHTMLRenderer);

	// Allow single node selection only
	tree.getSelectionModel().setSelectionMode(
	    TreeSelectionModel.SINGLE_TREE_SELECTION);

	// Display angled line
	tree.putClientProperty("JTree.lineStyle", "Angled");

	return tree;
    }

    /**
     * Converts a byte to hex digit and writes to the supplied buffer
     */
    private static void byte2hex(byte b, StringBuffer buf) {
        char[] hexChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
                            '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        int high = ((b & 0xf0) >> 4);
        int low = (b & 0x0f);
        buf.append(hexChars[high]);
        buf.append(hexChars[low]);
    }

    /**
     * Converts a byte array to hex string
     */
    private static String toHexString(byte[] block) {
        StringBuffer buf = new StringBuffer();
        int len = block.length;
        for (int i = 0; i < len; i++) {
             byte2hex(block[i], buf);
             if (i < len-1) {
                 buf.append(":");
             }
        }
        return buf.toString();
    }

    /**
     * Gets the requested finger print of the certificate.
     */
    private static String getCertFingerPrint(String mdAlg, X509Certificate cert)
        throws Exception
    {
        byte[] encCertInfo = cert.getEncoded();
        MessageDigest md = MessageDigest.getInstance(mdAlg);
        byte[] digest = md.digest(encCertInfo);
        return toHexString(digest);
    }

    /**
     * Method to reflect table data based on the certificate
     */
    private static void showCertificateInfo(X509Certificate cert, 
				JTable certInfoTable, JTextArea textArea) {

	String certVersion = "V" + cert.getVersion();

	String certSerialNumber = "[xxxxx-xxxxx]";
	String md5 = null;
	String sha1 = null;

	try
	{
	    SerialNumber serial = new SerialNumber(cert.getSerialNumber());

	    /**
	     * NOTE - ANDY XXX - serial.getNumber() with non-1.4+ java ???
	     */
	    certSerialNumber = "[" + serial.getNumber() + "]";

	    md5 = getCertFingerPrint("MD5", cert);
	    sha1 = getCertFingerPrint("SHA1", cert);
 	}
	catch(Throwable e)
	{
	}

	String certSigAlg = "[" + cert.getSigAlgName() + "]";
	String certIssuer = formatDNString(cert.getIssuerDN().toString());
	String certValidity = "[From: " + cert.getNotBefore() + 
				",\n To: " + cert.getNotAfter() + "]";
	String certSubject = formatDNString(cert.getSubjectDN().toString());

	HexDumpEncoder encoder = new HexDumpEncoder();
	String certSig = encoder.encodeBuffer(cert.getSignature());

	Object[][] data = {
	    { getMessage("cert.dialog.field.Version"), certVersion },
	    { getMessage("cert.dialog.field.SerialNumber"), certSerialNumber },
	    { getMessage("cert.dialog.field.SignatureAlg"), certSigAlg },
	    { getMessage("cert.dialog.field.Issuer"), certIssuer },
	    { getMessage("cert.dialog.field.Validity"), certValidity },
	    { getMessage("cert.dialog.field.Subject"), certSubject },
	    { getMessage("cert.dialog.field.Signature"), certSig },
	    { getMessage("cert.dialog.field.md5Fingerprint"), md5}, 
	    { getMessage("cert.dialog.field.sha1Fingerprint"), sha1} };

	String[] columnNames = { getMessage("cert.dialog.field"), 
				 getMessage("cert.dialog.value") };
 

	certInfoTable.setModel(new DefaultTableModel(data, columnNames) {
	    public boolean isCellEditable(int row, int col) { 
		return false; 
	    }	
	});

	// Select last row by default
	certInfoTable.setRowSelectionInterval(8, 8);
	certInfoTable.repaint();
	textArea.repaint();
    }


    private static String getMessage(String key)
    {
	return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) { 
        return ResourceManager.getAcceleratorKey(key); 
    } 
}


class CertificateInfo 
{
    X509Certificate cert;

    CertificateInfo(X509Certificate cert)
    {
	this.cert = cert;
    }

    public X509Certificate getCertificate()
    {
	return cert;
    }

    /**
     * Extrace CN from DN in the certificate.
     *
     * @param cert X509 certificate
     * @return CN
     */
    private String extractAliasName(X509Certificate cert)
    {
	String subjectName = getMessage("security.dialog.unknown.subject");
	String issuerName = getMessage("security.dialog.unknown.issuer");

	// Extract CN from the DN for each certificate
	try 
	{
       	    Principal principal = cert.getSubjectDN();
       	    Principal principalIssuer = cert.getIssuerDN();

	    // Extract subject name
	    String subjectDNName = principal.getName();
	    String issuerDNName = principalIssuer.getName();

	    // Extract subject name
	    subjectName = extractFromQuote(subjectDNName, "CN=");

	    if (subjectName == null)
		subjectName = extractFromQuote(subjectDNName, "O=");

	    if (subjectName == null)
		subjectName = getMessage("security.dialog.unknown.subject");

	    // Extract issuer name
	    issuerName = extractFromQuote(issuerDNName, "CN=");

	    if (issuerName == null)
		issuerName = extractFromQuote(issuerDNName, "O=");

	    if (issuerName == null)
		issuerName = getMessage("security.dialog.unknown.issuer");
	}
	catch (Exception e) 
	{
	    Trace.printException(e);
	}

	// Add Subject name and Issuer name in the return string
	MessageFormat mf = new MessageFormat(
	    getMessage("security.dialog.certShowName"));
	Object[] args = {subjectName, issuerName};
	return mf.format(args);
    }

    /** 
     * Extract from quote
     */
    private String extractFromQuote(String s, String prefix)
    {
	if ( s == null)
	    return null;

	// Search for issuer name
	//
	int x = s.indexOf(prefix);
	int y = 0;

	if (x >= 0)
	{
	    x = x + prefix.length();

	    // Search for quote
	    if (s.charAt(x) == '\"')
	    {
		// if quote is found, search another quote

		// skip the first quote
		x = x + 1;
		
		y = s.indexOf('\"', x);
	    }
	    else
	    {
		// quote is not found, search for comma
		y = s.indexOf(',', x);
	    }

	    if (y < 0)
		return s.substring(x);			
	    else
		return s.substring(x, y);			
	}
	else
	{
	    // No match
	    return null;
	}
    }

    private static String getMessage(String key) {
	return ResourceManager.getMessage(key);
    }

    public String toString() {
	return extractAliasName((X509Certificate) cert);
    }
}
