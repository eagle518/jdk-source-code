/*
 * @(#)AboutDialog.java	1.17 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.Icon;
import javax.swing.border.Border;
import javax.swing.WindowConstants;
import javax.swing.SwingConstants;
import javax.swing.JTextArea;
import javax.swing.JLabel;
import javax.swing.BorderFactory;
import javax.swing.KeyStroke;
import javax.swing.AbstractAction;
import java.awt.Window;
import java.awt.Font;
import java.awt.FlowLayout;
import java.awt.Image;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.KeyEvent;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.BorderLayout;
import java.net.URL;
import com.sun.deploy.panel.JSmartTextArea;
import com.sun.deploy.resources.ResourceManager;
import javax.swing.JEditorPane;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Insets;
import java.text.MessageFormat;
import com.sun.deploy.si.DeploySIListener;
import com.sun.deploy.si.SingleInstanceImpl;
import com.sun.deploy.si.SingleInstanceManager;



public class AboutDialog extends JDialog implements DeploySIListener{

    private SingleInstanceImpl sil = null;
    private static String ABOUT_JAVA_ID; 

	static {
		ABOUT_JAVA_ID = "com_sun_deploy_AboutJava-" + com.sun.deploy.config.Config.getProperty(com.sun.deploy.config.Config.VERSION_UPDATED_KEY);
	}

	public AboutDialog(JFrame parent, boolean modal, boolean si) {
		super(parent, modal);		
		if(si)
			initSIImpl();
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
	this.setResizable(false);
	setTitle( getMessage("about.dialog.title") );
	setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
	addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent evt) {
			closeDialog(evt);
		}
	});

	JPanel pane = new JPanel(new BorderLayout());
 	pane.setForeground(Color.white);
	pane.setBackground(Color.white);
	pane.setBorder(BorderFactory.createLineBorder(Color.black));

	// Create top image.
	URL url = ClassLoader.getSystemResource("com/sun/deploy/resources/image/aboutjava.png");
	ImageIcon imageTopIcon = new ImageIcon(url);
	JLabel topImage = new JLabel(imageTopIcon);
	JPanel topImagePane = new JPanel(new BorderLayout());
 	topImagePane.setForeground(Color.WHITE);
	topImagePane.setBackground(Color.WHITE);
	topImagePane.add(topImage, BorderLayout.CENTER);
	topImagePane.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
	pane.add(BorderLayout.NORTH, topImagePane);

	int lineWidth = imageTopIcon.getIconWidth() - 20;
	JPanel layoutPane = new JPanel(new BorderLayout());
	layoutPane.setBorder(BorderFactory.createEmptyBorder(6, 9, 9, 11));
	layoutPane.setForeground(Color.WHITE);
	layoutPane.setBackground(Color.WHITE);

	String javaFamilyVersion = System.getProperty("java.version");
	int pos = javaFamilyVersion.lastIndexOf("_");
	if(pos != -1)
		javaFamilyVersion = javaFamilyVersion.substring(0, pos);

	String verText = (new StringBuffer(MessageFormat.format(getMessage("about.java.version"),
			new String[] {javaFamilyVersion,System.getProperty("java.runtime.version")})))
			.append('\n')
			.append(getMessage("about.copyright"))
			.append('\n')
			.append(getMessage("about.legal.note"))
			.toString();
	JTextArea txtVer = new JTextArea();
	txtVer.setBorder(BorderFactory.createEmptyBorder(0, 4, 0, 4));
	txtVer.setEditable(false);
	txtVer.setLineWrap(true);
	txtVer.setWrapStyleWord(true);
	Font font = txtVer.getFont();
	// reduce font size by 1.0, or to 11.0 (whichever is larger)
	float fontSize = font.getSize2D();
	fontSize = Math.max((float)(fontSize - 1.0), (float) 11.0);
	Font dispFont = font.deriveFont(fontSize);
	txtVer.setFont(dispFont);
	txtVer.setText(verText);
	FontMetrics fm = txtVer.getFontMetrics(dispFont);
	int txtLen = fm.stringWidth(getMessage("about.legal.note"));
	int rows = ((txtLen + lineWidth - 1) / lineWidth) + 2;
	txtVer.setRows(rows);
	layoutPane.add(txtVer, BorderLayout.NORTH);

        String strInfo = getMessage("about.prompt.info");
	String linkURL = getMessage("about.home.link");
	final String text = (new StringBuffer("<font face=")).append(dispFont.getFamily())
			.append(" size=-1>")
			.append(strInfo)
			.append(" <a style='color:blue' href=").append(linkURL)
			.append('>').append(linkURL).append("</a></font>").toString();


	final JEditorPane editPane = new JEditorPane("text/html",text);

	// seems JEditPane uses different default font
	editPane.setFont(dispFont);
	fm = editPane.getFontMetrics(dispFont);
	txtLen = fm.stringWidth(strInfo + ' ' + linkURL);
	rows = ((txtLen + lineWidth - 1)/ lineWidth) ;
	int txtHeight = fm.getHeight();
        int height = Math.max(rows, 2) * Math.max(txtHeight, 15) +
		     editPane.getInsets().top + editPane.getInsets().bottom;
	Dimension d = new Dimension(lineWidth, height);
	editPane.setPreferredSize(d);
	editPane.setEditable(false);   // required for hyperlink events
	editPane.addHyperlinkListener(new HyperlinkListener() {
	    public void hyperlinkUpdate(HyperlinkEvent e) {
		// Link is clicked
		if ( e.getEventType() == HyperlinkEvent.EventType.ACTIVATED ) {
			com.sun.deploy.config.Config.getInstance().showDocument(e.getURL().toString());
		}
		// Link is moused over
		else if ( e.getEventType() == HyperlinkEvent.EventType.ENTERED ) {
			editPane.setText(text.replaceFirst("'color:blue'","'color:red'"));
		}
		// Link is mouse exited
		else if ( e.getEventType() == HyperlinkEvent.EventType.EXITED ) {
			editPane.setText(text);
		}
	    }
	});

	editPane.setForeground(new Color(txtVer.getForeground().getRGB()));
	editPane.setBackground(new Color(txtVer.getBackground().getRGB()));
	layoutPane.add(editPane, BorderLayout.SOUTH);

	JPanel commandPane = new JPanel(new FlowLayout(FlowLayout.RIGHT));
	JButton btnClose = new JButton(getMessage("about.option.close"));
	btnClose.setMnemonic(com.sun.deploy.resources.ResourceManager.getAcceleratorKey("about.option.close"));
	
        AbstractAction closeAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
		okBtnActionPerformed(evt);
            }
        };
        btnClose.addActionListener(closeAction);

        KeyStroke esc = KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE);
        getRootPane().getInputMap(
                          JComponent.WHEN_IN_FOCUSED_WINDOW).put(esc,"cancel");
        getRootPane().getActionMap().put("cancel", closeAction); 
        txtVer.getInputMap().put(esc,"cancel");
        txtVer.getActionMap().put("cancel", null); 
        editPane.getInputMap().put(esc, "cancel");
        editPane.getActionMap().put("cancel", null); 

	commandPane.add(btnClose);
	pane.add(layoutPane, BorderLayout.CENTER);
	getContentPane().setLayout(new BorderLayout());
	getContentPane().add(commandPane, BorderLayout.SOUTH);
	getContentPane().add(pane, BorderLayout.CENTER);

	// Set "Close" to be default button.  This means when user
	// presses "Enter" on the keyboard, "Close" button will be pressed.
	getRootPane().setDefaultButton(btnClose);

	pack();

	/* setLocationRelativeTo() only sets best effort location for AboutDialog as this dialog
	 * is instantiated with null owner.  Therefore, using DialogFactory.positionDialog(Dialog)
	 * here instead, as that provides better placement of this dialog using screen's dimension.
	 * this.setLocationRelativeTo(this.getOwner());
	 */
	DialogFactory.positionDialog(this);

    }

    private void okBtnActionPerformed(ActionEvent evt){
		if(sil != null)
			sil.removeSingleInstanceListener(this);
        setVisible(false);
        dispose();
    }


    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
		if(sil != null)
			sil.removeSingleInstanceListener(this);
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

    private String java_image = "com/sun/deploy/resources/image/about_java.png";


    public void newActivation(String[] params) {
		Window owner = this;
		while((owner = owner.getOwner()) != null) {
			if(owner instanceof JFrame) 
				break;
		}
		if(owner != null) 
			((JFrame)owner).toFront();
	}

    public Object getSingleInstanceListener() {
		return this;
	}
}
