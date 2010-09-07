/*
 * @(#)ErrorPanel.java	1.10 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import javax.swing.JPanel;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.MenuItem;
import java.awt.PopupMenu;
import com.sun.deploy.ui.UIFactory;
import sun.plugin.JavaRunTime;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.MemoryCache;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import javax.swing.ImageIcon;
import java.util.HashSet;
import java.util.Iterator;
import java.net.URL;
import java.util.StringTokenizer;

/**
 *
 * @author mfisher
 */
public class ErrorPanel extends JPanel 
    implements MouseListener, ActionListener{
    
    private Image errorImage = null;
    private static final String ERROR_IMAGE_FILE = 
	"sun/plugin/util/graybox_error.png";
    private static final Color ERROR_BORDER = new Color(204, 204, 204);
    
    private Color bg_color;
    private Color fg_color;
    private PopupMenu popup;
    private MenuItem open_console, about_java, reload;
    private Container parent =  null;
    private ErrorDelegate errorDelegate;
    private boolean deniedCertificateFailure = false;
    
    public ErrorPanel (Color bg_color, Color fg_color, Container m_parent, ErrorDelegate ed){
	super();   
	parent = m_parent;
	errorDelegate = ed;
	this.bg_color = bg_color;            
	this.fg_color = fg_color;            
	this.setToolTipText(
			    ResourceManager.getMessage("applet.error.message"));
	addMouseListener(this);  
	deniedCertificateFailure = deniedCertificateFailure();

	if (deniedCertificateFailure) {
            // Clear cache files and JARs from memory
            MemoryCache.clearLoadedResources();
	    
            // We MUST reset the deny cert store in trust decider
            // in order for the security warning dialog with certificate
            // information to be shown again to user.
            TrustDecider.resetDenyStore();
	}
    }
    
    private synchronized Image getErrorImage() {
	if(errorImage == null){
	    Toolkit tk = Toolkit.getDefaultToolkit();
	    errorImage = tk.createImage(
					ClassLoader.getSystemResource(ERROR_IMAGE_FILE));
	    MediaTracker mt = new MediaTracker(this);
	    mt.addImage(errorImage, 0);
	    try {
		mt.waitForID(0);
	    } catch (InterruptedException e) { }
            
	}
	return errorImage;
    }  
    
        protected void paintComponent(Graphics g) {  
            g.setColor(bg_color);
            g.fillRect(0, 0, getWidth(), getHeight());                        

            Graphics2D g2d = (Graphics2D)g.create();  
            
            // Draw border around applet area.	
            drawBorder(g2d, getSize());   

            // Draw image and String only if applet size
            // is larger then 24x24 pixels
            if(getWidth() > 24 && getHeight() > 24) {                                 
                int xOffset = 4;
                int yOffset = 5;
                        
                // The image is drawn at (4, 5) coordinate
                // according to UE spec.
                g2d.drawImage(getErrorImage(), xOffset, yOffset, bg_color, null);
                                    
                // Align bottom of image with baseline of error text.
                // There should be 7 pixels between image and beginning
                // of the text.  The text should be placed 4 pixels 
                // above the bottom of image - according to UE spec.
                drawMessage(g2d, 
                    ResourceManager.getMessage("applet.error.message"), 
                    getErrorImage().getWidth(null) + xOffset + 7,
                    getErrorImage().getHeight(null) + yOffset - 4); 
            }
            
            g2d.dispose(); //clean up
        }
        
        
        private void drawBorder(Graphics g, Dimension d) {
		Color color = ERROR_BORDER;
		Color oldColor = g.getColor();
		g.setColor(color);
		g.drawRect(0, 0, d.width - 1, d.height - 1);
		g.setColor(oldColor);
	}
        
        private void drawMessage(Graphics2D g2d, String error, int x, int y) {
            Dimension d = this.getSize();
            
            // Figure out if string fits into available space.  In case
            // if it does not, keep trimming word by word adding "..." to 
            // the end.
            FontMetrics fm = g2d.getFontMetrics();
            Rectangle2D strBounds = fm.getStringBounds(error, g2d);
            
            boolean fit = true;
            // See if icon and string fit.  Add starting position "x" to 
            // string's width.
            if ((strBounds.getWidth() + x) > d.getWidth()){
                // the string with icon does not fit, check if the string 
                // created by trimming last word and adding "..." will fit
                fit = false;
                while(!fit) {
                    int spaceIdx = error.lastIndexOf(" ");
                    if (spaceIdx == -1){
                        break;
                    }
                    error = error.substring(0, spaceIdx);
                    error = error + "...";
                    strBounds = fm.getStringBounds(error, g2d);
                    if ((strBounds.getWidth() + x) < d.getWidth()){
                        fit = true;
                    }
                }
                
                if (!fit){
                    // try if just "..." will fit
                    error = "...";
                    strBounds = fm.getStringBounds(error, g2d);
                    if ((strBounds.getWidth() + x) < d.getWidth()){
                        fit = true;
                    }
                }
            } 
            
            // draw string if it fits
            if (fit) {
                Color oldColor = g2d.getColor();
                g2d.setColor(fg_color);
                g2d.drawString(error, x, y);
                g2d.setColor(oldColor);
            }            
        }
        
	private PopupMenu getPopupMenu()
	{
	    if (popup == null){
		// Create popup menu
		popup = new PopupMenu();
		open_console = new MenuItem(ResourceManager.getMessage(
                        "dialogfactory.menu.open_console"));                 		
                popup.add(open_console);
		popup.add("-");

		about_java = new MenuItem(ResourceManager.getMessage(
                        "dialogfactory.menu.about_java"));
                popup.add(about_java);
                
                if (deniedCertificateFailure){
                    // If applet failed to load due to denied certificate,
                    // add "Reload application" menu item to allow user
                    // to reload applet and view the certificate again.                
                    reload = new MenuItem("Reload applet");

                    popup.add("-");
                    popup.add(reload);
                    reload.addActionListener(this);
                }

		open_console.addActionListener(this);
		about_java.addActionListener(this);                

                this.add(popup);          	    
	    }

	    return popup;
	}        
        
	public void mouseEntered(MouseEvent e) { }        
	public void mouseExited(MouseEvent e) { }
	public void mousePressed(MouseEvent e) {
            if (e.isPopupTrigger()) {
		// Pop up menu with option to display Java Console 
		// This will be called on unix.
		getPopupMenu().show(e.getComponent(), e.getX(), e.getY());            
	    } else if (e.getButton() == MouseEvent.BUTTON1) {
                // This must be left mouse button click - 
                // show the error dialog.
                onLeftMouseClick();
            }
        }
        
	public void mouseReleased(MouseEvent e) {
	     if (e.isPopupTrigger()) {
		// Pop up menu with option to display Java Console
		// This will be called on windows.
		getPopupMenu().show(e.getComponent(), e.getX(), e.getY());            
	    } else if (e.getButton() == MouseEvent.BUTTON1) {
                // This must be left mouse button click - 
                // show the error dialog.
                onLeftMouseClick();
            }       
        }
        
	public void mouseClicked(MouseEvent e) {}
        
        private void onLeftMouseClick(){
            int selection = -1;
            
            // Left mouse click should show error dialog.
            if (deniedCertificateFailure){
                // Show 3-buttons error dialog
                // Using this detailed version of showErrorDialog to specify
                // button names - different from standard error dialog.
                // Providing keys for the buttons names.  Keys stored in
                // Deployment.java file
                selection = UIFactory.showErrorDialog(
                    null, 
                    new com.sun.deploy.ui.AppInfo(),
                    ResourceManager.getMessage("applet.error.generic.masthead"),
                    ResourceManager.getMessage("applet.error.generic.body"), 
                    "applet.error.details.btn",
                    "applet.error.ignore.btn", 
                    "applet.error.reload.btn");
            } else {
                // Show 2-buttons error dialog
                selection = UIFactory.showErrorDialog(
                    null, 
                    new com.sun.deploy.ui.AppInfo(),
                    ResourceManager.getMessage("applet.error.generic.masthead"),
                    ResourceManager.getMessage("applet.error.generic.body"), 
                    "applet.error.details.btn",
                    "applet.error.ignore.btn", 
                    null);
            }
            if (selection == UIFactory.OK){
                // First button was pressed (Details) - open Java Console
                JavaRunTime.showJavaConsoleLater(true);
                JavaRunTime.installConsoleTraceListener();
            } else if (selection == UIFactory.CANCEL){
                // Second button was pressed (Ignore) - do nothing                
            } else if (selection == UIFactory.ASK_ME_LATER){
                // Third button was pressed (Reload).  We are reusing
                // an update dialog with 3 buttons, so answer is ASK_ME_LATER
                // - clear the denied certificate store
                // - reload applet allowing user to view the certificate again                
                reloadApplet();
            }
        }
        
	public void actionPerformed(ActionEvent e) {
	    if (e.getSource() == open_console){ 
		// Popup java console and print exceptions to it from trace file.        
		JavaRunTime.showJavaConsoleLater(true);
                JavaRunTime.installConsoleTraceListener();
	    }
	    else if (e.getSource() == about_java){
		// Show about java dialog
		UIFactory.showAboutJavaDialog();
	    }
            else if (e.getSource() == reload){
                // Reload applet
                reloadApplet();
	    }
	} 
        
        private void reloadApplet(){
            if (errorDelegate != null) {
                // Delegate the rest of this operation to the ErrorDelegate
                errorDelegate.handleReloadApplet();
            }
        }
        
        
        /*
         * Look through the list of denided codebase/jar file names
         * and compare them to the codebase of this applet.
         * When codebases are the same, compare jar file name from the list
         * to jar file names from this applet's archive.
         * When matching combination is found, we know that one of the 
         * certificates for this applet's jar files has been denied.
         */
        private boolean deniedCertificateFailure(){
            boolean foundDenied = false;
            String codebase = null;
            HashSet jarFilesSet = new HashSet();

            // Retrieve denied codebase/jar file list
            HashSet deniedURLSet = TrustDecider.getDeniedURL();
            
            if (errorDelegate != null) {
                // Get the code base from the error delegate
                codebase = errorDelegate.getCodeBase();

                // Get the list of archive files from the error delegate
                errorDelegate.addJarFilesToSet(jarFilesSet);
            }
            
            Iterator deniedIter = deniedURLSet.iterator();
            while (deniedIter.hasNext()){
                String deniedURL = deniedIter.next().toString();
                if (codebase != null && deniedURL.startsWith(codebase)){
                    // See if any of the jar files from this applet are in
                    // the denied cert store
                    Iterator jarsIter = jarFilesSet.iterator();
                    while (jarsIter.hasNext()){
                        String jarName = jarsIter.next().toString();
                        if (deniedURL.endsWith(jarName)){
                            foundDenied = true;
                            break;
                        }
                    }                                                     
                }
            }
            
            return foundDenied;
        }
        
    }

