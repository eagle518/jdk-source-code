/*
 * @(#)ClientCertDialog.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.text.MessageFormat;
import javax.swing.JList;
import javax.swing.ListSelectionModel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;

final class ClientCertDialog
{

    /**
     * <P> Show ClientCertDialog.
     * </P>
     *
     * @return alias name of the selected certificate
     */
    static String showDialog(final HashMap clientAuthCertsMap, 
            final HashMap clientAuthTypeMap)  
    {
        
        // Construct JList of certificates to be displayed:
        final Vector certs = new Vector();

	for (Iterator iter = clientAuthCertsMap.keySet().iterator(); 
                                                        iter.hasNext();){
            String aliasName = (String) iter.next();

            // Get related certfiicate and type from HashMap based on alias name
            X509Certificate[] xcerts = 
                    (X509Certificate[]) clientAuthCertsMap.get(aliasName);
            Object certType = clientAuthTypeMap.get(aliasName);

	    // Display certificate subject, issuer and type in list box
	    String subjectName = CertUtils.extractSubjectAliasName(xcerts[0]);
	    String issuerName = CertUtils.extractIssuerAliasName(xcerts[0]);
	    String certString = subjectName + ":" + issuerName;
            
            String key = "clientauth.certlist.dialog.browserKS";
	    if (certType.equals(CertType.PLUGIN)){
	       key = "clientauth.certlist.dialog.javaKS";
            }
            	
            MessageFormat mf = new MessageFormat(getMessage(key));            
	    Object[] argsCert = {certString};
	    certs.add(mf.format(argsCert));
        }
        
        // Create JList object
        JList certList = new JList();
        certList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        certList.setListData(certs);
        if (certs.size() > 0){
           certList.setSelectedIndex(0);
        }

        int selectedIndex = UIFactory.showListDialog(null, 
                        getMessage("clientauth.certlist.dialog.caption"),
                        getMessage("clientauth.certlist.dialog.text"), 
                        null, true, certList, clientAuthCertsMap);
	
        // Get selected alias name for certificate
        String alias = null;
	
        // User clicked OK button
        if (selectedIndex != UIFactory.ERROR) {
            for (Iterator iter = clientAuthCertsMap.keySet().iterator(); 
                       		   selectedIndex >= 0 && iter.hasNext(); 
                                                        selectedIndex--) {                        
                alias = (String) iter.next();
            }
        }
        return alias;
    }

    private static String getMessage(String key)  {
	return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
}
