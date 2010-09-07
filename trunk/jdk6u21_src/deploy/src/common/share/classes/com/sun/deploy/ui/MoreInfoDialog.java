/*
 * @(#)MoreInfoDialog.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;


import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.ImageIcon;
import javax.swing.JTextArea;
import javax.swing.JScrollPane;
import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.KeyStroke;

import java.awt.Color;
import java.awt.Component;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.security.cert.Certificate;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.security.CertificateDialog;
import com.sun.deploy.config.Config;

/*
 * This dialog displays information messages to user.  
 * It contains security-related messages for certificate validation, 
 * desktop integration and file association messages from web start. 
 * There is also a label "Certificate Details" that brings up
 * the dialog with detailed information about certificate.
 * The "Close" button is the only action button.  It should dismiss the 
 * dialog and dispose of it.
 *
 * @author mfisher
 */

class MoreInfoDialog extends JDialog{
    
    private FancyButton details;
    private String[] alerts;
    private String[] infos;
    private int securityInfoCount;
    private Certificate[] certs;
    private int start;
    private int end;
    
    private final String WARNING_ICON = 
            "com/sun/deploy/resources/image/icon-warning16.png";
    private final String INFO_ICON = 
            "com/sun/deploy/resources/image/icon-info16.png";
    private final int VERTICAL_STRUT = 18;
    private final int HORIZONTAL_STRUT = 12;
    
    // According to spec.
    private final int TEXT_WIDTH = 326;
    
    MoreInfoDialog(JDialog parent, String[] alerts, String[] infos,
            int securityInfoCount, Certificate[] certs, int start, int end){
        super(parent, true);
        this.alerts = alerts;
        this.infos = infos;
	this.securityInfoCount = securityInfoCount;
        this.certs = certs;
        this.start = start;
        this.end = end;
        initComponents(null, null);
	setResizable(false);
    }
    
    MoreInfoDialog(JDialog parent, JPanel detailPanel, 
	Throwable throwable, Certificate[] certs) {
        super(parent, true);
	this.certs = certs;
	this.start = 0;
	this.end = (certs == null) ? 0 : certs.length;

	initComponents(detailPanel, throwable);
    }


    /*
     * Initialize components for this dialog.
     */
    private void initComponents(JPanel detailPanel, Throwable throwable){

        setTitle(getMessage("security.more.info.title"));

        getContentPane().setLayout(new BorderLayout());
        JPanel contentPanel = new JPanel();
        contentPanel.setBorder(
                BorderFactory.createEmptyBorder(16, 16, 12, 16));
        
        contentPanel.setLayout(new BoxLayout(
				   contentPanel, BoxLayout.Y_AXIS));

        contentPanel.setAlignmentX(Component.LEFT_ALIGNMENT);

	if (detailPanel != null) {
	    contentPanel.add(detailPanel);
	} else if (throwable != null) {
	    contentPanel.add(Box.createHorizontalStrut(440));
	    JPanel labelPanel = new JPanel(new BorderLayout());

	    JLabel label = new JLabel(ResourceManager.getString(
				"exception.details.label"));
	    label.setBorder(BorderFactory.createEmptyBorder(0, 0, 8, 0));
	    labelPanel.add(label, BorderLayout.WEST);
	    
	    contentPanel.add(labelPanel);

	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    throwable.printStackTrace(pw);
	    JTextArea text = new JTextArea(sw.toString(), 20, 60);
	    text.setFont(ResourceManager.getUIFont());
	    text.setEditable(false);
	    text.setLineWrap(true);
	    text.setWrapStyleWord(false);
	    JScrollPane sp = new JScrollPane(text,
			JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
		JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	    contentPanel.add(sp);
	    
	    if (certs != null) {
                contentPanel.add(Box.createVerticalStrut(VERTICAL_STRUT));
                contentPanel.add(getLinkPanel());
	    }
	} else {
        
            contentPanel.add(getSecurityPanel());        
	    if (certs != null) {
                contentPanel.add(getLinkPanel()); 
	    }
            contentPanel.add(Box.createVerticalStrut(VERTICAL_STRUT));
            contentPanel.add(getIntegrationPanel());
	}
        contentPanel.add(Box.createVerticalStrut(VERTICAL_STRUT));
        contentPanel.add(getBtnPanel());

        getContentPane().add(contentPanel, BorderLayout.CENTER);

        pack();

	UIFactory.placeWindow(this);
        
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                dismissAction();
            }
        });

    }
    
    /*
     * Security panel is the top portion of the "More Information" dialog
     * It contains security-related messages - either warnings or
     * informational messages, or both.
     */
    private JPanel getSecurityPanel(){
        JPanel securityPanel = new JPanel();
        securityPanel.setLayout(new BoxLayout(securityPanel, BoxLayout.Y_AXIS));
        
        // If there are any alerts, do not show the first element of the
        // array - this is the message we used for bulleted text in parent dlg.
        //
        // If there are no alerts, and infos is not empty(and it should not be),
        // do not show the first element of the infos array - this is the 
        // messages we used for bulleted text in parent dlg.
        int start;
	int end;

	// but for the DTI dialog we want to reshow what we showed in 
	// the first page.
	boolean showall = (certs == null);

	start = (showall || alerts == null) ? 0 : 1;
	end = (alerts == null) ? 0 : alerts.length;
        securityPanel.add(blockPanel(WARNING_ICON, alerts, start, end));

	start = (showall || alerts != null) ? 0 : 1;
	end = securityInfoCount;
        securityPanel.add(blockPanel(INFO_ICON, infos, start, end));
        return securityPanel;
    }
    
    /*
     * this panel contains right-aligned link-looking label that brings
     * up "Certificate Details" dialog when clicked on.
     */
    private JPanel getLinkPanel(){
        JPanel linkPanel = new JPanel();
        linkPanel.setLayout(new BoxLayout(linkPanel, BoxLayout.X_AXIS));
        linkPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
        
        linkPanel.add(Box.createHorizontalGlue());
        details = new FancyButton("security.more.info.details", 
                                  Color.blue);        
        // Add mouse listener to open certificate details dialog on
        // mouse click event.
        details.addMouseListener(new MouseListener(){
            public void mouseClicked(MouseEvent me){
                if (me.getComponent() instanceof FancyButton){
                    showCertDetails();                      
                }
            }
            public void mouseEntered(MouseEvent me){};
            public void mouseExited(MouseEvent e) {}
            public void mousePressed(MouseEvent e) {}
            public void mouseReleased(MouseEvent e) {}
        });
        // Add action listener to open certificate details dialog on
        // keyboard navigation events.        
        details.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent ae){
                showCertDetails();
            }
        });
        linkPanel.add(details);
        
        return linkPanel;
    }
    
    /*
     * this panel contains integration and association messages
     * from the DesktopIntegrationDialog. This is optional and might not appear
     * if app did not request shortcuts/associations.
     */
    private JPanel getIntegrationPanel(){
	int start = securityInfoCount;
	int end = (infos == null) ? 0 : infos.length;
        return blockPanel(INFO_ICON, infos, start, end);
    }
    
    /*
     * This panel contains right-aligned "Close" button.  It should
     * dismiss the dialog and dispose of it.
     */
    private JPanel getBtnPanel(){
        JPanel btnPanel = new JPanel();
        btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.X_AXIS));
        btnPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
        
        btnPanel.add(Box.createHorizontalGlue());
        
        JButton dismissBtn = new JButton(getMessage("common.close_btn"));
        dismissBtn.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent ae){
                dismissAction();
            }
        });        
        getRootPane().setDefaultButton(dismissBtn);
        btnPanel.add(dismissBtn);
        return btnPanel;
    }
    
    /*
     * create a panel with a number of subpanels with an icon 
     * and a text next to it.  Create as many of these subpanels
     * as there are string messages in the 'strs'
     */
    private JPanel blockPanel (String iconLblStr, String[] strs, 
			       int start, int end) {
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
        if ( strs != null ) {                           
            for (int i = start; i < end; i++){
                JPanel p = new JPanel();
                p.setLayout(new BoxLayout(p, BoxLayout.X_AXIS));
                p.setAlignmentX(Component.LEFT_ALIGNMENT);
                
                JLabel iconLbl = new JLabel();
                iconLbl.setIcon(new ImageIcon(
                        ClassLoader.getSystemResource(iconLblStr)));
                iconLbl.setAlignmentY(Component.TOP_ALIGNMENT);
                iconLbl.setAlignmentX(Component.LEFT_ALIGNMENT);
                
                UITextArea text = new UITextArea(
                                            new JLabel().getFont().getSize(),
                                            TEXT_WIDTH,
                                            false);
                text.setText(strs[i]);
                // Set preferred size -to avoid packing problems.
                Dimension pref = text.getPreferredSize();
                text.setSize(pref.width, pref.height);
                text.setAlignmentY(Component.TOP_ALIGNMENT);
                text.setAlignmentX(Component.LEFT_ALIGNMENT);
                
                p.add(iconLbl);
                p.add(Box.createHorizontalStrut(HORIZONTAL_STRUT));
                p.add(text);
                p.add(Box.createHorizontalGlue());
                
                panel.add(p);
                panel.add(Box.createVerticalStrut(VERTICAL_STRUT));
            }
        }
        return panel;

    }
    
    /*
     * When user clicks on link-looking label, bring up
     * the dialog to show certificate details.
     */
    private void showCertDetails(){
        CertificateDialog.showCertificates(this, certs, start, end);
    }
    
    /*
     * Close this dialog and dispose of it.
     */
    private void dismissAction(){
        setVisible(false);
        dispose();
    }
    
    /* 
     * Get resource string from the resource bundle.
     */
    private String getMessage(String id) {
        return ResourceManager.getMessage(id);
    }      
}
