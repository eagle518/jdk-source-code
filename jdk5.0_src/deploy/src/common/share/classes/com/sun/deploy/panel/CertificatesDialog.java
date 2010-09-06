/*
 * @(#)CertificatesDialog.java	1.21 04/03/30
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import javax.swing.JTable;
import javax.swing.border.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableModel;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.text.MessageFormat;
import java.io.*;
import java.security.Key;
import java.security.KeyStore;
import java.security.Principal;
import java.security.cert.*;
import javax.swing.LookAndFeel;
import com.sun.deploy.config.Config;
import com.sun.deploy.security.*;
import com.sun.deploy.util.*;
import com.sun.deploy.resources.ResourceManager;
import sun.misc.BASE64Encoder;
import sun.security.provider.X509Factory;


public class CertificatesDialog extends JDialog implements ChangeListener
{
    /** Creates new form Certificates */
    public CertificatesDialog(Frame parent, boolean modal) 
    {
        super(parent, modal);
	this.model = new CertificatesInfo();
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {
        getContentPane().setLayout(new BorderLayout());
        
        setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        setTitle(getMessage("cert.settings"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                closeDialog(evt);
            }
        });

        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));
        topPanel.setPreferredSize(new Dimension(500,300));
        topPanel.setBorder(BorderFactory.createRaisedBevelBorder());

        /* Create Panel to hold ComboBox.*/
        JPanel certTypePanel = new JPanel();
        certTypePanel.setLayout(new BoxLayout(certTypePanel, BoxLayout.X_AXIS));

        /*
         * Create comboPanel with label and combo box.
         */
        JPanel comboPanel = new JPanel();
        comboPanel.setLayout(new BoxLayout(comboPanel, BoxLayout.Y_AXIS));
        comboPanel.setBorder(BorderFactory.createEmptyBorder(10,10,5,10));
        comboPanel.add(Box.createHorizontalStrut(60));
        
        // Combo box
        certsComboBox = new JComboBox();
        certsComboBox.setModel(new DefaultComboBoxModel(certTypeName));
        certsComboBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                certsComboBoxActionPerformed(evt);
            }
        });        
        comboPanel.add(certsComboBox);
        
        // add comboPanel to topPanel
	certTypePanel.add(new JLabel(getMessage("cert.dialog.certtype")));
        certTypePanel.add(comboPanel);
	certTypePanel.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 5));
        topPanel.add(certTypePanel);
        // end comboPanel

        /*
         * Construct tabbedPane
         */

        tabbedPane = new JTabbedPane();
	userTab = new CertificateTabPanel(model, USER_LEVEL);
	systemTab = new CertificateTabPanel(model, SYSTEM_LEVEL);

        tabbedPane.setName(" ");
        tabbedPane.addTab(getMessage("cert.dialog.user.level"), userTab);          
        tabbedPane.addTab(getMessage("cert.dialog.system.level"), systemTab);  
	tabbedPane.setSelectedIndex(0);
	tabbedPane.addChangeListener(this);

        // add tabs to topPanel
        topPanel.add(tabbedPane);

        /*
         * Construct certsBtnsPanel with four buttons: 
         * Import, Export, Remove, Details.
         */
        JPanel certsBtnsPanel = new JPanel();
        certsBtnsPanel.setBorder(BorderFactory.createEmptyBorder(0,10,5,10));

        importButton = new JButton(getMessage("cert.import_button"));
        importButton.setMnemonic(ResourceManager.getVKCode("cert.import_button.mnemonic"));
        importButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                importButtonActionPerformed(evt);
            }
        });
        importButton.setToolTipText(getMessage("cert.import_btn.tooltip"));
        certsBtnsPanel.add(importButton);
        
        exportButton = new JButton(getMessage("cert.export_button"));
        exportButton.setMnemonic(ResourceManager.getVKCode("cert.export_button.mnemonic"));
        exportButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                exportButtonActionPerformed(evt);
            }
        });
        exportButton.setToolTipText(getMessage("cert.export_btn.tooltip"));
        certsBtnsPanel.add(exportButton);
        
        removeButton = new JButton(getMessage("cert.remove_button"));
        removeButton.setMnemonic(ResourceManager.getVKCode("cert.remove_button.mnemonic"));
        removeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                removeButtonActionPerformed(evt);
            }
        });
        removeButton.setToolTipText(getMessage("cert.remove_btn.tooltip"));
        certsBtnsPanel.add(removeButton);
        
        detailsButton = new JButton(getMessage("cert.details_button"));
        detailsButton.setMnemonic(ResourceManager.getVKCode("cert.details_button.mnemonic"));
        detailsButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                detailsButtonActionPerformed(evt);
            }
        });
        detailsButton.setToolTipText(getMessage("cert.details_btn.tooltip"));
        certsBtnsPanel.add(detailsButton);

        // add cetsBtnsPanel to topPanel
        topPanel.add(certsBtnsPanel);
        // end certsBtnsPanel
        
        // Add topPanel to certificates
        getContentPane().add(topPanel, BorderLayout.CENTER);

        /*
         * Construct bottomPanel
         */
        JPanel bottomPanel = new JPanel();
        bottomPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        bottomPanel.setBorder(BorderFactory.createEmptyBorder(5,10,5,10));
        
        closeButton = new JButton(getMessage("cert.close_button"));
        closeButton.setMnemonic(ResourceManager.getVKCode("cert.close_button.mnemonic"));
	AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                closeButtonActionPerformed(evt);
            }
        };
        closeButton.addActionListener(cancelAction);
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE),"cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        bottomPanel.add(closeButton);
        // end bottomPanel

        // add bottomPanel to contentpane
        getContentPane().add(bottomPanel, BorderLayout.SOUTH);

	// Update button state
	updateButtonState();
        
        // Set "Close" to be default button.  This means when user
        // presses "Enter" on the keyboard, "Close" button will be pressed.
        getRootPane().setDefaultButton(closeButton);
        

        // Pack all components
        pack();
        
        // The certificate dialog should not be resizable.
        setResizable(false);
    }

    private void certsComboBoxActionPerformed(ActionEvent evt) {
	// Get current certificates
	reset();
    }

    private void closeButtonActionPerformed(ActionEvent evt) {
        setVisible(false);
        dispose();
    }

    private void importButtonActionPerformed(ActionEvent evt) {

	// Popup FileChooser
        JFileChooser jfc = new JFileChooser();

        // Set filter for File Chooser Dialog Box
        CertFileFilter impFilter = new CertFileFilter();
        impFilter.addExtension("csr");
        impFilter.addExtension("p12");
        impFilter.setDescription("Certificate Files");
        jfc.setFileFilter(impFilter);

        jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
        jfc.setDialogType(JFileChooser.OPEN_DIALOG);
        jfc.setMultiSelectionEnabled(false);
        int result = jfc.showOpenDialog(this);
        if (result == JFileChooser.APPROVE_OPTION)
        {
           File f = jfc.getSelectedFile();
           if (f == null) return;

           try {
           	InputStream inStream = System.in;
                inStream = new FileInputStream(f);

               	// Import certificate from file to deployment.certs
            	boolean impStatus = false;
            	impStatus = importCertificate(inStream);

               	// Check if it is belong to PKCS#12 format
		if (!impStatus)
		{
               	   // Create another inputStream for PKCS#12 foramt
                   InputStream inP12Stream = System.in;
                   inP12Stream = new FileInputStream(f);

		   // Get certType to decide how to import
		   String certType = (String)certsComboBox.getSelectedItem();
	
	  	   if (certType.equals("Client Authentication"))
                      importPKCS12CertKey(inP12Stream);
		   else
                      importPKCS12Certificate(inP12Stream);
      		}
    	   }
	   catch(Throwable e2) {
		String errorMsg = getMessage("cert.dialog.import.file.text");
		String errorTitle = getMessage("cert.dialog.import.error.caption");
           	DialogFactory.showExceptionDialog(this.getParent(), e2,  errorMsg, errorTitle);
           }
        }

        reset();
    }

    private void exportButtonActionPerformed(ActionEvent evt) {

	// Get alias name for selected certificate
	String certType = (String)certsComboBox.getSelectedItem();
	int selectedRow = -1;
	int level = USER_LEVEL;

	if (isUserLevelSelected())
	{
	    level = USER_LEVEL;
	    selectedRow = userTab.getSelectedCertificateTableRow();
	}
	else
	{
	    level = SYSTEM_LEVEL;
	    selectedRow = systemTab.getSelectedCertificateTableRow();
	}

	if (selectedRow != -1)
	{
	  // Get certificate which you want to export
	  Collection certs = null;
          X509Certificate cert = null;
	  Certificate[] certChain = {null};

	  if (certType.equals(strTrustedCerts))
	     certs = model.getTrustedCertificates(level);
	  else if (certType.equals(strSecureSite))
	     certs = model.getHttpsCertificates(level);
	  else if (certType.equals(strSignerCa))
	     certs = model.getRootCACertificates(level);
	  else if (certType.equals(strSecureSiteCa))
	     certs = model.getHttpsRootCACertificates(level);
	  else if (certType.equals(strClientAuth))
	     certs = model.getClientAuthCertificates(level);

	  Object[] certsArray = certs.toArray();

 	  if (certType.equals(strClientAuth))
	  {
             certChain = (Certificate[]) certsArray[selectedRow];
	     cert = (X509Certificate) certChain[0];
	  }
	  else
	     cert = (X509Certificate) certsArray[selectedRow];

	  if (cert != null)
          {
             // Popup FileChooser
             JFileChooser jfc = new JFileChooser();
             jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
             jfc.setDialogType(JFileChooser.SAVE_DIALOG);
             jfc.setMultiSelectionEnabled(false);
             int result = jfc.showSaveDialog(this);
             if (result == JFileChooser.APPROVE_OPTION)
             {
          	File f = jfc.getSelectedFile();
                if (f == null) return;
                PrintStream ps = null;
                try {
		    // Export certificate file
 	  	    if (certType.equals(strClientAuth))
                       exportPKCS12Cert(certChain, f, level);
		    else
		    {
                       ps = new PrintStream(new BufferedOutputStream(new FileOutputStream(f)));
                       exportCertificate(cert, ps);
		    }
                }
                catch(Throwable e2)
                {
          	    String errorMsg = getMessage("cert.dialog.export.text");
          	    String errorTitle = getMessage("cert.dialog.export.error.caption");
                    DialogFactory.showExceptionDialog(this.getParent(), e2, errorMsg, errorTitle); 
                }
                finally {
                    if (ps != null)
                        ps.close();
                }
              }
          } // Cert not null
          else
	  {
          	String errorMsg2 = getMessage("cert.dialog.export.text");
          	String errorTitle2 = getMessage("cert.dialog.export.error.caption");
                DialogFactory.showErrorDialog(this, errorMsg2, errorTitle2);
	  }
	} //Alias not null
    }

    private void removeButtonActionPerformed(ActionEvent evt) {

	// Get alias name for selected certificate
	String certType = (String)certsComboBox.getSelectedItem();
	int selectedRow = -1;
	int level = USER_LEVEL;

	if (isUserLevelSelected())
	{
	    level = USER_LEVEL;
	    selectedRow = userTab.getSelectedCertificateTableRow();
	}
	else
	{
	    level = SYSTEM_LEVEL;
	    selectedRow = systemTab.getSelectedCertificateTableRow();
	}


	if (selectedRow != -1)
	{
	   // Pop up confirm dialog box
           Object msgs = getMessage("cert.dialog.remove.text");
           String title = getMessage("cert.dialog.remove.caption"); 
           int selectValue = DialogFactory.showConfirmDialog(this, msgs, title);

           // User click OK button
           if (selectValue == 0)
           {
		// Remove certificate based on user selection
	  	if (certType.equals(strTrustedCerts))
		{
		    Collection certs = model.getTrustedCertificates(level);
		    Object[] certsArray = certs.toArray();
           	    model.removeTrustedCertificate((Certificate) certsArray[selectedRow]);
		}
	  	else if (certType.equals(strSecureSite))
		{
		    Collection certs = model.getHttpsCertificates(level);
		    Object[] certsArray = certs.toArray();
           	    model.removeHttpsCertificate((Certificate) certsArray[selectedRow]);
		}
	  	else if (certType.equals(strSignerCa))
		{
		    Collection certs = model.getRootCACertificates(level);
		    Object[] certsArray = certs.toArray();
           	    model.removeRootCACertificate((Certificate) certsArray[selectedRow]);
		}
	  	else if (certType.equals(strSecureSiteCa))
		{
		    Collection certs = model.getHttpsRootCACertificates(level);
		    Object[] certsArray = certs.toArray();
           	    model.removeHttpsRootCACertificate((Certificate) certsArray[selectedRow]);
		}
	  	else if (certType.equals(strClientAuth))
		{
		    Collection certs = model.getClientAuthCertificates(level);
		    Object[] certsArray = certs.toArray();
           	    model.removeClientAuthCertificate(this, (Certificate[]) certsArray[selectedRow]);
		}

		reset();
	   }
	}
    }

    private void detailsButtonActionPerformed(ActionEvent evt) 
    {
	// Get alias name for selected certificate
	String certType = (String)certsComboBox.getSelectedItem();
	int selectedRow = -1;

	if (isUserLevelSelected())
	    selectedRow = userTab.getSelectedCertificateTableRow();
	else
	    selectedRow = systemTab.getSelectedCertificateTableRow();


	// Display detail certificate info based on user selection
	if (selectedRow != -1)
	{
	   Collection certs = null;
           Certificate cert = null;
	   Certificate[] certChain = {null};
	   int level = 0;

	   // Get radio button value
	   if (!isUserLevelSelected())
	      level = 1;

	   if (certType.equals(strTrustedCerts))
		certs = model.getTrustedCertificates(level);
	   else if (certType.equals(strSecureSite))
		certs = model.getHttpsCertificates(level);
	   else if (certType.equals(strSignerCa))
		certs = model.getRootCACertificates(level);
	   else if (certType.equals(strSecureSiteCa))
		certs = model.getHttpsRootCACertificates(level);
	   else if (certType.equals(strClientAuth))
		certs = model.getClientAuthCertificates(level);

	   Object[] certsArray = certs.toArray();

	   // Client Authenticate keystore return certificate chain
	   if (certType.equals(strClientAuth))
	      certChain = (Certificate[]) certsArray[selectedRow];
	   else
              certChain = new Certificate[] { (Certificate) certsArray[selectedRow] };

           if (certChain[0] != null)
	   {
          	CertificateDialog dialog = new CertificateDialog(this, certChain, 0, certChain.length);
           	dialog.DoModal();
	   }
	}
    }

    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
        setVisible(false);
        dispose();
    }

    /* Export a certificate from client authenticate keystore to a PKCS12 file */
    private void exportPKCS12Cert(Certificate[] certChain, File f, int level)
    {
	char[] keyPassJKS = null;
	char[] keyPassPKCS = null;
	String mykeyStore = null;

	try {
	  // Get client authentication certificate file deployment.clientauthcerts
	  if (level == USER_LEVEL)
             mykeyStore = Config.getUserClientAuthCertFile();
	  else
             mykeyStore = Config.getSystemClientAuthCertFile();

	  KeyStore ks = KeyStore.getInstance("JKS");
	  FileInputStream fis = new FileInputStream(mykeyStore);

	  // Pop up password dialog box to get JKS keystore password
          keyPassJKS = getPasswordDialog("cert.dialog.exportpassword.text", "cert.dialog.password.export.caption");
	  ks.load(fis, keyPassJKS);

	  // User click OK button
          if (keyPassJKS != null)
          {
	     // Get export cert and key info
	     String alias = ks.getCertificateAlias(certChain[0]);
	     Key outKey = ks.getKey(alias, keyPassJKS);

	     // Write to output PKCS12 file
	     FileOutputStream fos = new FileOutputStream(f);
	     KeyStore pkcs = KeyStore.getInstance("PKCS12");
	     pkcs.load(null, null);

	     // Pop up password dialog box to get keystore password
             keyPassPKCS = getPasswordDialog("cert.dialog.savepassword.text", "cert.dialog.password.export.caption");

	     // User click OK button
             if (keyPassPKCS != null)
             {
	     	pkcs.setKeyEntry(alias, outKey, keyPassPKCS, certChain);
	     	pkcs.store(fos, keyPassPKCS);
	     }
	  }
	}
	catch (Exception e) {
          String errorMsg = getMessage("cert.dialog.export.password.text");
          String errorTitle = getMessage("cert.dialog.export.error.caption");
          DialogFactory.showExceptionDialog(this.getParent(), e, errorMsg, errorTitle);
        }
    }

    /* Export a certificate to a file */
    private void exportCertificate(X509Certificate cert, PrintStream ps)
    {
        BASE64Encoder encoder = new BASE64Encoder();
        ps.println(X509Factory.BEGIN_CERT);
        try
        {
            encoder.encodeBuffer(cert.getEncoded(), ps);
        }
        catch (Throwable e)
        {
            //Trace.printException(this, e);
        }
        ps.println(X509Factory.END_CERT);
    }

    /* Import a PKCS#12 format certificate and adds it to the list of trusted certificates */
    void importPKCS12Certificate(InputStream is)
    {
        char[] password = null;

        try
        {
          KeyStore myKeySto = KeyStore.getInstance("PKCS12");

	  // Pop up password dialog box to get keystore password
          password = getPasswordDialog("cert.dialog.password.text", "cert.dialog.password.import.caption");

	  // User click OK button
          if (password != null)
          {
             // Load KeyStore based on the password
             myKeySto.load(is,password);

             // Get Alias list from KeyStore.
             Enumeration aliasList = myKeySto.aliases();

	     // Get keyStore certificate type
	     String certType = (String)certsComboBox.getSelectedItem();

             while (aliasList.hasMoreElements())
             {
               	// Get Certificate based on the alias name
               	String certAlias = (String)aliasList.nextElement();

             	X509Certificate cert = (X509Certificate)myKeySto.getCertificate(certAlias);

               	// Add to import list based on certificate type selection
	  	if (certType.equals(strTrustedCerts))
           	   model.addTrustedCertificate(cert);
	  	else if (certType.equals(strSecureSite))
           	   model.addHttpsCertificate(cert);
	  	else if (certType.equals(strSignerCa))
           	   model.addCACertificate(cert);
	  	else if (certType.equals(strSecureSiteCa))
           	   model.addHttpsCACertificate(cert);

		reset();
             }
          } // OK button
        }
        catch(Throwable e)
        {
            // Show up Error dialog box if the user enter wrong password
            // Avoid to convert password array into String - security reason
            String uninitializedValue =  "uninitializedValue";
            if (!compareCharArray(password, uninitializedValue.toCharArray()))
            {
           	String errorMsg = getMessage("cert.dialog.import.password.text");
                String errorTitle = getMessage("cert.dialog.import.error.caption");
                DialogFactory.showExceptionDialog(this.getParent(), e, errorMsg, errorTitle);
            }
        }
	finally // Reset password
	{
            if (password != null)
               java.util.Arrays.fill(password, ' ');
        }
    }

    void importPKCS12CertKey(InputStream is)
    {
        char[] password = null;

        try
        {
	  // Pop up password dialog box to get keystore password
          password = getPasswordDialog("cert.dialog.password.text", "cert.dialog.password.import.caption");

	  // User click OK button
          if (password != null)
          {
             // Load KeyStore based on the password
             KeyStore myKeySto = KeyStore.getInstance("PKCS12");
             myKeySto.load(is,password);

             // Get Alias list from KeyStore.
             Enumeration aliasList = myKeySto.aliases();

             while (aliasList.hasMoreElements())
             {
                String certAlias = (String)aliasList.nextElement();
                Certificate[] certChain = myKeySto.getCertificateChain(certAlias);
                Key inKey = myKeySto.getKey(certAlias, password);
                model.addClientAuthCertChain(this, certChain, inKey);
            }
	  } // OK click	
        }
        catch(Throwable e)
        {
           String errorMsg = getMessage("cert.dialog.import.password.text");
	   String errorTitle = getMessage("cert.dialog.import.error.caption");
           DialogFactory.showExceptionDialog(this.getParent(), e, errorMsg, errorTitle);
        }
	finally // Reset password
	{
            if (password != null)
               java.util.Arrays.fill(password, ' ');
        }
    }

    private boolean compareCharArray(char[] c1, char[] c2) {
   	if (c1.length != c2.length)
       	   return false;

        for (int index = 0; index < c1.length; index ++)
	{
            if (c1[index] != c2[index])
               return false;
        }

        return true;
    }

    /* Import a certificate and adds it to the list of trusted certificates */
    boolean importCertificate(InputStream is)
    {
        CertificateFactory cf = null;
        X509Certificate cert = null;

        try
        {
	    // Get keyStore certificate type
	    String certType = (String)certsComboBox.getSelectedItem();

            cf = CertificateFactory.getInstance("X.509");
            cert = (X509Certificate)cf.generateCertificate(is);

            // Add to keyStore based on certificate type selection
	    if (certType.equals(strTrustedCerts))
               model.addTrustedCertificate(cert);
	    else if (certType.equals(strSecureSite))
               model.addHttpsCertificate(cert);
	    else if (certType.equals(strSignerCa))
               model.addCACertificate(cert);
	    else if (certType.equals(strSecureSiteCa))
               model.addHttpsCACertificate(cert);
	    else if (certType.equals(strClientAuth))
	       return false;

	    reset();
        }
        catch (CertificateParsingException cpe)
        {
            // It is PKCS#12 format.
            return false;
        }
        catch (CertificateException e)
        {
            // Wrong format of the selected file
            String errorMsg = getMessage("cert.dialog.import.format.text");
            String errorTitle = getMessage("cert.dialog.import.error.caption");
            DialogFactory.showExceptionDialog(this.getParent(), e, errorMsg, errorTitle);
        }

        return true;
    }

    private void reset()
    {
	// Get user selected cert type
        String certType = (String)certsComboBox.getSelectedItem();

	// Based on System or User level
        Collection certs = null;
	if (isUserLevelSelected())
	{
          // Based on different Cert type, show related certificate in list box
          if (certType.equals(strTrustedCerts))
             certs = model.getTrustedCertificates(USER_LEVEL);
          else if (certType.equals(strSecureSite))
             certs = model.getHttpsCertificates(USER_LEVEL);
          else if (certType.equals(strSignerCa))
             certs = model.getRootCACertificates(USER_LEVEL);
          else if (certType.equals(strSecureSiteCa))
             certs = model.getHttpsRootCACertificates(USER_LEVEL);
          else if (certType.equals(strClientAuth))
             certs = model.getClientAuthCertificates(USER_LEVEL);
	}
	else
	{
          // Based on different Cert type, show related certificate in list box
          if (certType.equals(strTrustedCerts))
             certs = model.getTrustedCertificates(SYSTEM_LEVEL);
          else if (certType.equals(strSecureSite))
             certs = model.getHttpsCertificates(SYSTEM_LEVEL);
          else if (certType.equals(strSignerCa))
             certs = model.getRootCACertificates(SYSTEM_LEVEL);
          else if (certType.equals(strSecureSiteCa))
             certs = model.getHttpsRootCACertificates(SYSTEM_LEVEL);
          else if (certType.equals(strClientAuth))
             certs = model.getClientAuthCertificates(SYSTEM_LEVEL);
	}


	TableModel certsTableModel;

        if (certs == null || certs.size() == 0)
	{
	    certsTableModel = new ReadOnlyTableModel();
	}
        else
        {
            // Fill up list box with the sorted certificates
	    certsTableModel = new ReadOnlyTableModel(certs.size());

	    int row = 0;

	    for (Iterator iter = certs.iterator(); iter.hasNext(); row++)
	    {
		X509Certificate c = null;

		// For client authentication, it will return cert chain
          	if (certType.equals(strClientAuth))
		{
		  Certificate[] certChain = (Certificate[]) iter.next();
		  c = (X509Certificate) certChain[0];
		}
	 	else
		  c = (X509Certificate) iter.next();

		certsTableModel.setValueAt(CertUtils.extractSubjectAliasName(c), row, 0);
		certsTableModel.setValueAt(CertUtils.extractIssuerAliasName(c), row, 1);
	    }	
	}

	if (isUserLevelSelected())
	    userTab.setCertificateTableModel(certsTableModel);
	else
	    systemTab.setCertificateTableModel(certsTableModel);

	updateButtonState();
    }

    boolean isUserLevelSelected()
    {
	return (tabbedPane.getSelectedIndex() == 0);
    }

    /**
     * Enable/Disabled a component
     *
     * @param field the component to enable/disable
     * @param b true to enable to text field, false to disable it
     */
    private void setEnabled(JComponent field, boolean b) {
        field.setEnabled(b);
        field.repaint();
    }

    /**
     * Called when tabbedPane state is changed.
     */
    public void stateChanged(ChangeEvent e)  
    {
	reset();
    }

    /**
     * Updated button state.
     */
    private void updateButtonState()
    {
	if (isUserLevelSelected())
	{
	    // For user certificate store
	    boolean certificateSelected = userTab.isCertificateSelected();

            setEnabled(removeButton, certificateSelected);
            setEnabled(exportButton, certificateSelected);
            setEnabled(importButton, true);
            setEnabled(detailsButton, certificateSelected);
	}
	else
	{
	    // For system certificate store
	    boolean certificateSelected = systemTab.isCertificateSelected();

            setEnabled(removeButton, false);
            setEnabled(exportButton, certificateSelected);
            setEnabled(importButton, false);
            setEnabled(detailsButton, certificateSelected);
	}
    }

    private char[] getPasswordDialog(final String inLabel, final String inTitle)
    {
        try {
            return (char[])(DeploySysRun.execute(new DeploySysAction() {
                public Object execute() throws Exception {
		    LookAndFeel lookAndFeel = null;

		    try
		    {
			// Change look and feel
			lookAndFeel = DeployUIManager.setLookAndFeel();
			
			return getPasswordDialogImp(inLabel, inTitle);
		    }
		    finally
		    {
			// Restore look and feel
			DeployUIManager.restoreLookAndFeel(lookAndFeel);
		    }                               
                }}));
        }
        catch(Exception e) { // should never happen
            Trace.ignoredException(e);
            return null;
        }
    }

    private char[] getPasswordDialogImp(String inLabel, String inTitle)
    {
        // Pop up password dialog box
        Object dialogMsg = getMessage(inLabel);
        JPasswordField passwordField = new JPasswordField();

        Object[] msgs = new Object[2];
        msgs[0] = dialogMsg.toString();
        msgs[1] = passwordField;

        JButton okButton = new JButton(getMessage("cert.dialog.password.okButton"));
        JButton cancelButton = new JButton(getMessage("cert.dialog.password.cancelButton"));

        String title = getMessage(inTitle);
        Object[] options = {okButton, cancelButton};
        int selectValue = DialogFactory.showOptionDialog(this, DialogFactory.QUESTION_MESSAGE, msgs, title, options, options[0]);

        // for security purpose, DO NOT put password into String.
        // Reset password as soon as possible.
        final char[] mypassword = passwordField.getPassword();

        // User click OK button
        if (selectValue == 0)
           return mypassword;
        else
           return null;
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
        return com.sun.deploy.resources.ResourceManager.getMessage(key);
    }

    private int USER_LEVEL = 0;
    private int SYSTEM_LEVEL = 1;
    private JComboBox certsComboBox;

    private JButton importButton, exportButton, removeButton, detailsButton;
    private JButton closeButton;
    private CertificatesInfo model;
    private JTabbedPane tabbedPane;
    private CertificateTabPanel userTab;
    private CertificateTabPanel systemTab;

    private String strTrustedCerts = getMessage("cert.type.trusted_certs");
    private String strSecureSite = getMessage("cert.type.secure_site");
    private String strSignerCa = getMessage("cert.type.signer_ca");
    private String strSecureSiteCa = getMessage("cert.type.secure_site_ca");
    private String strClientAuth = getMessage("cert.type.client_auth");
    private String[] certTypeName = {strTrustedCerts, strSecureSite,
				strSignerCa, strSecureSiteCa, strClientAuth};
}


/**
 * Read-only table model.
 */
class ReadOnlyTableModel extends DefaultTableModel
{
    ReadOnlyTableModel()
    {
	super(new Object[] { getMessage("cert.dialog.issued.to"), getMessage("cert.dialog.issued.by") }, 0);
    }

    ReadOnlyTableModel(int row)
    {
	super(new Object[] {  getMessage("cert.dialog.issued.to"), getMessage("cert.dialog.issued.by") }, row);
    }

    public boolean isCellEditable(int row, int column) 
    {
	return false;
    }

    private static String getMessage(String key)  {
        return com.sun.deploy.resources.ResourceManager.getMessage(key);
    }
}


/**
 * Tab panel for user/system certificates.
 */
class CertificateTabPanel extends JPanel
{
    private JTable certsTable;

    CertificateTabPanel(CertificatesInfo model, int level)
    {
        /*
         * Construct certsPanel with scroll pane and text area.
         */
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createEmptyBorder(5,10,5,10));

        JScrollPane certsScrollPane = new JScrollPane();
        certsScrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        certsScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        certsScrollPane.setAutoscrolls(true);

	// Create certificate list box
        Collection certs = model.getTrustedCertificates(level);
	if (certs == null || certs.size() == 0)
	{
	    TableModel certsTableModel = new ReadOnlyTableModel();
            certsTable = new JTable(certsTableModel);
	}
        else
        {
            // Fill up list box with the sorted certificates
	    TableModel certsTableModel = new ReadOnlyTableModel(certs.size());

	    int row = 0;

	    for (Iterator iter = certs.iterator(); iter.hasNext(); row++)
	    {
		X509Certificate c = (X509Certificate) iter.next();
		certsTableModel.setValueAt(CertUtils.extractSubjectAliasName(c), row, 0);
		certsTableModel.setValueAt(CertUtils.extractIssuerAliasName(c), row, 1);
	    }	

            certsTable = new JTable(certsTableModel);
        }

	// Set font
	// certsTable.setFont(new Font("Dialog", 0, 11));
	//certsTable.setFont(new javax.swing.JLabel().getFont());

	// Disable drag
	certsTable.setDragEnabled(false);

	// Single selection only
	certsTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

	// Disable column selection
	certsTable.setColumnSelectionAllowed(false);

	if (certs != null && certs.size() > 0)
	    certsTable.setRowSelectionInterval(0, 0);

        certsScrollPane.setViewportView(certsTable);
        
	add(certsScrollPane, BorderLayout.CENTER);
    }

    /**
     * Change model of certificate table.
     */
    void setCertificateTableModel(TableModel m)
    {
	certsTable.setModel(m);

	// Disable drag
	certsTable.setDragEnabled(false);

	// Single selection only
	certsTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

	// Disable column selection
	certsTable.setColumnSelectionAllowed(false);

	if (m.getRowCount() > 0)
	    certsTable.setRowSelectionInterval(0, 0);

	// Force UI to be updated.
	certsTable.updateUI();
    }

    /**
     * Return selected row in certificate table; return -1 otherwise.
     *
     */
    int getSelectedCertificateTableRow()
    {
	return certsTable.getSelectedRow();
    }

    /**
     * Return true if a row in certificate table is selected.
     */
    boolean isCertificateSelected()
    {
	return (getSelectedCertificateTableRow() != -1);
    }
}

