/*
 * @(#)AboutDialog.java	1.32 10/03/29
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.ImageIcon;
import javax.swing.WindowConstants;
import javax.swing.BorderFactory;
import javax.swing.KeyStroke;
import javax.swing.AbstractAction;
import java.awt.Window;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.event.KeyEvent;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.BorderLayout;
import java.net.URL;
import com.sun.deploy.resources.ResourceManager;
import javax.swing.JEditorPane;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.text.MessageFormat;
import com.sun.deploy.si.DeploySIListener;
import com.sun.deploy.si.SingleInstanceImpl;
import com.sun.deploy.si.SingleInstanceManager;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.Font;
import javax.swing.JTextArea;


public class AboutDialog extends JDialog implements DeploySIListener{

    private SingleInstanceImpl sil = null;
    private static String ABOUT_JAVA_ID; 

    static {	
        ABOUT_JAVA_ID = "com_sun_deploy_AboutJava-" + 
                com.sun.deploy.config.Config.getProperty(
                com.sun.deploy.config.Config.VERSION_UPDATED_KEY);
        com.sun.deploy.util.DeployUIManager.setLookAndFeel();
    }


    public AboutDialog(JFrame parent, boolean modal, boolean si) {
	
        super(parent, modal);		
        if(si){
            initSIImpl();
        }			
        initComponents();
    }

    public AboutDialog (JFrame parent, boolean modal) {
        super(parent, modal);
        initComponents();
    }

    public AboutDialog (JDialog parent, boolean modal){
        super(parent, modal);
        initComponents();
    }

    private void initSIImpl() {
        sil = new SingleInstanceImpl(); 
        sil.addSingleInstanceListener(this, ABOUT_JAVA_ID);
    }
    
    public static boolean shouldStartNewInstance() {
        if (SingleInstanceManager.isServerRunning(ABOUT_JAVA_ID) 
			&& SingleInstanceManager.connectToServer("")) {
            return false;
        }
        return true;
    }

    private void initComponents(){
	setTitle( getMessage("about.dialog.title") );
	setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                closeDialog(evt);
            }
	});
        
        // Colors are given to us by UE.
        Color backgrnd = Color.white;
        Color foregrnd = new Color(122, 114, 119);

	JPanel pane = new JPanel(new BorderLayout());
        
        //Set back/foreground to the same color
 	pane.setForeground(backgrnd);
	pane.setBackground(backgrnd);
        pane.setOpaque(true); //For GTK look and feel, see 6252085
	pane.setBorder(BorderFactory.createLineBorder(Color.black));
        GridBagLayout gb = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        pane.setLayout(gb);
        
        // Fill in both directions.
        c.fill = GridBagConstraints.BOTH;                        

	// Create top image.
	URL url = ClassLoader.getSystemResource(
                "com/sun/deploy/resources/image/aboutjava.png");
	ImageIcon imageTopIcon = new ImageIcon(url);
	JLabel topImage = new JLabel(imageTopIcon);
        
        // Make top image occupy the whole row
        c.gridwidth = GridBagConstraints.REMAINDER;
        gb.setConstraints(topImage, c);
        pane.add(topImage);

        // 10 pixels insets on left and right = 20 pixels.
	final int lineWidth = imageTopIcon.getIconWidth() - 20;
        
        String javaVersion = System.getProperty("java.version");
        
        // String version is 1.x.y_ab-ea - the x between two "." is the
        // marketing version.
        int firstDot = javaVersion.indexOf(".");
        String marketingVersion = javaVersion.substring(
                firstDot + 1, javaVersion.indexOf(".", firstDot + 1));
        
        int pos = javaVersion.lastIndexOf("_");
        String updateVersion = null;

        if(pos != -1){
            // Find if there is "-xx" after update version
            int dashIdx = javaVersion.indexOf("-");
            if ( dashIdx != -1 ) {
                updateVersion = 
                    javaVersion.substring(pos + 1, dashIdx);
            } else {
                //There is no "-xx" after update version
                updateVersion = 
                    javaVersion.substring(pos + 1, javaVersion.length());
            }
                        
            if (updateVersion.startsWith("0")){
                // updateVersion starts with "0" -remove it
                updateVersion = updateVersion.substring(1);
            }                    
        }

        String version = null;
        if (updateVersion != null){
            version = MessageFormat.format(
                    getMessage("about.java.version.update"),
                    new String[] {marketingVersion, updateVersion});
        } else {
            version = 
                    MessageFormat.format(getMessage("about.java.version"),
                    new String[] {marketingVersion});
        }
        
        String buildInfo = MessageFormat.format(getMessage("about.java.build"),
                new String[] {System.getProperty("java.runtime.version")});
                
        
        Font derivedFont = ResourceManager.getUIFont()
                .deriveFont((float) DialogTemplate.getSubpanelFontSize());
                   
        
        JLabel versionText = new JLabel(version);
        versionText.setForeground(foregrnd);
        versionText.setBackground(backgrnd);
        versionText.setFont(derivedFont);
        
        JTextArea buildText = new JTextArea(buildInfo);
        buildText.setEditable(false);
        buildText.setForeground(foregrnd);
        buildText.setBackground(backgrnd);
        buildText.setFont(derivedFont);
                        
        String copyright = getMessage("about.copyright");
        String legal = getMessage("about.legal.note");
        String strInfo = getMessage("about.prompt.info");
	String linkURL = getMessage("about.home.link");
        String text = (new StringBuffer(copyright).append("\n")
                                    .append(legal)).toString();
        
        UITextArea topText = 
                new UITextArea( DialogTemplate.getSubpanelFontSize() , 
                lineWidth, false);
        topText.setText(text);
        topText.setForeground(foregrnd);
        
        // This is in case UE decides to use smaller font size for technical
        // version string display        
        JPanel topTextPanel = new JPanel();
        topTextPanel.setBackground(backgrnd);
        GridBagLayout gbl = new GridBagLayout();
        GridBagConstraints co = new GridBagConstraints();
        topTextPanel.setLayout(gbl);
        
        co.fill = GridBagConstraints.BOTH;
        co.gridwidth = GridBagConstraints.RELATIVE;   
        gbl.setConstraints(versionText, co);
        topTextPanel.add(versionText);
        
        co.gridwidth = GridBagConstraints.REMAINDER;
        co.fill = GridBagConstraints.HORIZONTAL;
        co.anchor = GridBagConstraints.SOUTH;
        co.insets = new Insets(0, 10, 0, 0);
        gbl.setConstraints(buildText, co);
        topTextPanel.add(buildText);
        
        co.fill = GridBagConstraints.BOTH;
        co.insets = new Insets(0, 0, 0, 0);
        co.weightx = 0.0;
        gbl.setConstraints(topText, co);                
        topTextPanel.add(topText);
        
        final String infoText = (new StringBuffer("<font face="))
                                .append(ResourceManager.getUIFont().getFamily())
                                .append(" color=#7A7277>")
                                .append(strInfo)
                                .append(" ")
                                .append(" <a style='color:#FF0000' href=")
                                .append(linkURL).append('>')
                                .append(linkURL).append("</a></font>")
                                .toString();
        

        /*
         * Overwrite JEditorPane's getPreferredSize() to get fixed with and
         * variable height.
         */
        final JEditorPane editPane = new JEditorPane() {
            public Dimension getPreferredSize() {
                if (getWidth() == 0) {
                    Insets insets = getInsets();
                    setSize(lineWidth, insets.top + insets.bottom + 1);
                }
                Dimension pref = super.getPreferredSize();
                pref.width = getWidth();
                return pref;
            }
        }; 

        editPane.setFont(derivedFont);
        editPane.putClientProperty(
                JEditorPane.HONOR_DISPLAY_PROPERTIES, Boolean.TRUE);
        editPane.setContentType("text/html");
        
        /*
         * We set text after we set font to workaround font display problem
         * with JEditorPane
         */
        editPane.setText(infoText);        
        editPane.setForeground(foregrnd);
	editPane.setBackground(backgrnd);
        FontMetrics fm = editPane.getFontMetrics(derivedFont);
        int txtLen = fm.stringWidth(strInfo + linkURL);
                
        // Required for hyperlink events.
        editPane.setEditable(false);
        
        // required for hyperlink events
	editPane.addHyperlinkListener(new HyperlinkListener() {
	    public void hyperlinkUpdate(HyperlinkEvent e) {
		// Link is clicked
		if ( e.getEventType() == HyperlinkEvent.EventType.ACTIVATED ) {
			com.sun.deploy.config.Config.getInstance()
                            .showDocument(e.getURL().toString());
		}
		// Link is moused over
		else if ( e.getEventType() == HyperlinkEvent.EventType.ENTERED ) {
			editPane.setText(infoText.replaceFirst(
                                "'color:#35556b'","'color:#c06600'"));
		}
		// Link is mouse exited
		else if ( e.getEventType() == HyperlinkEvent.EventType.EXITED ) {
			editPane.setText(infoText);
		}
	    }
	});
        
        c.insets = new Insets(10, 10, 0, 10);
        gb.setConstraints(topTextPanel, c);
        pane.add(topTextPanel);
        
        c.insets = new Insets(10, 10, 30, 10);
        gb.setConstraints(editPane, c);
        pane.add(editPane);
        
        JLabel sunLogoLabel = new JLabel();
        ImageIcon icon = ResourceManager.getIcon("sun.logo.image");
        sunLogoLabel.setIcon(icon);
        
        c.insets = new Insets(0, 10, 10, 10);
        gb.setConstraints(sunLogoLabel, c);
        pane.add(sunLogoLabel);

	JPanel commandPane = new JPanel(new FlowLayout(FlowLayout.RIGHT, 10, 10));
	JButton btnClose = new JButton(getMessage("about.option.close"));
	
        AbstractAction closeAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
		okBtnActionPerformed(evt);
            }
        };
        btnClose.addActionListener(closeAction);
        
        KeyStroke esc = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0);
        getRootPane().getInputMap(
                JComponent.WHEN_IN_FOCUSED_WINDOW).put(esc,"cancel");
        getRootPane().getActionMap().put("cancel", closeAction); 
        editPane.getInputMap().put(esc, "cancel");
        editPane.getActionMap().put("cancel", null); 

	commandPane.add(btnClose);
       
	getContentPane().setLayout(new BorderLayout());
	getContentPane().add(pane, BorderLayout.CENTER);
        getContentPane().add(commandPane, BorderLayout.SOUTH);

	// Set "Close" to be default button.  This means when user
	// presses "Enter" on the keyboard, "Close" button will be pressed.
	getRootPane().setDefaultButton(btnClose);

	pack();	
        setResizable(false);
    }

    private void okBtnActionPerformed(ActionEvent evt){
        if(sil != null){
            sil.removeSingleInstanceListener(this);
        }            			
        setVisible(false);
        dispose();
    }


    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
        if(sil != null){
            sil.removeSingleInstanceListener(this);
        }			
        setVisible(false);
        dispose();
    }

    /*
     * Get String from resource file.
     */
    private String getMessage(String id)
    {
	return com.sun.deploy.resources.ResourceManager.getMessage(id);
    }

    public void newActivation(String[] params) {
        this.toFront();
    }

    public Object getSingleInstanceListener() {
		return this;
	}
    
}
