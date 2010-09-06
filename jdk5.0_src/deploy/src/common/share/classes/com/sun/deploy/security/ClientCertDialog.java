/*
 * @(#)ClientCertDialog.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.BorderLayout;
import java.io.IOException;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JList;
import javax.swing.border.*;
import javax.swing.ListSelectionModel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


final class ClientCertDialog implements ActionListener 
{
    private HashMap	clientAuthCertsMap = new HashMap();
    private Vector  	certs    = new Vector();
    private JList	certList;

    /**
     * <P> Constructor for ClientCertDialog.
     * </P>
     *
     * @param HashMap of certficate array
     */
    ClientCertDialog(HashMap clientAuthCertsMap)  
    {
	this.clientAuthCertsMap = clientAuthCertsMap;

	for (Iterator iter = clientAuthCertsMap.values().iterator(); iter.hasNext();)
	{
	    X509Certificate[] xcerts = (X509Certificate[]) iter.next();
	    certs.add(CertUtils.extractSubjectAliasName(xcerts[0]));
	}
    }

    /**
     * <P> Show ClientCertDialog.
     * </P>
     *
     * @return alias name of the selected certificate
     */
    String DoModal()  
    {
	// Pop up certificate list dialog box
        Object dialogMsg = getMessage("clientauth.certlist.dialog.text");

        // Create a panel to hold JList
        JPanel certPanel = new JPanel();
        certPanel.setLayout(new BorderLayout());
        certPanel.setBorder(new EmptyBorder(0, 5, 0, 5));

        // Create JList object
        certList = new JList();
        certList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        certList.setListData(certs);
	if (certs.size() > 0)
           certList.setSelectedIndex(0);

        // Create detail info button
        JButton detailsButton = new JButton(getMessage("clientauth.certlist.dialog.buttonDetails"));
        detailsButton.setMnemonic(getAcceleratorKey("clientauth.certlist.dialog.buttonDetails"));
        detailsButton.addActionListener(this);

        // Add JList to panel
        certPanel.add(new JScrollPane(certList), BorderLayout.CENTER);

	// Add Button to panel
	JPanel buttonPanel = new JPanel();
	buttonPanel.add(detailsButton);
	buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));

        Object[] msgs = new Object[3];
        msgs[0] = dialogMsg.toString();
        msgs[1] = certPanel;
        msgs[2] = buttonPanel;

        JButton okButton = new JButton(getMessage("clientauth.certlist.dialog.buttonOK"));
        JButton cancelButton = new JButton(getMessage("clientauth.certlist.dialog.buttonCancel"));

	String title = getMessage("clientauth.certlist.dialog.caption");
        Object[] options = {okButton, cancelButton};
        int selectValue = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE,
					msgs, title, options, options[0]);

        // Get select alias name for certificate
        int selectedIndex = certList.getSelectedIndex();
        String alias = null;

        // User click OK button
        if (selectValue == 0)
        {
	    for (Iterator iter = clientAuthCertsMap.keySet().iterator(); selectedIndex >= 0 && iter.hasNext(); selectedIndex--)
	    {
		alias = (String) iter.next();
	    }
        }

	return alias;
    }

    /**
     * <P> Launch Certificate dialog if the "Details" button is clicked.
     * </P>
     *
     * @param e ActionEvent object.
     */
    public void actionPerformed(ActionEvent e)  
    {
	int selectedIndex = certList.getSelectedIndex();
	X509Certificate[] selectCert = null;
	
	for (Iterator iter = clientAuthCertsMap.values().iterator(); selectedIndex >= 0 && iter.hasNext(); selectedIndex--)
	{
	    selectCert = (X509Certificate[]) iter.next();
	}

	// Only show detail certificate dialog box when certificate is available
	if (selectCert != null)
	{
	    CertificateDialog dialog = new CertificateDialog(selectCert, 0, selectCert.length);
	    dialog.DoModal();			
	}
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
	return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
}
