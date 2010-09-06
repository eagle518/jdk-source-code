/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import sun.plugin.AppletViewer;
import java.awt.*;
import java.awt.event.*;
import java.net.*;

class GrayBoxPanel extends Container implements MouseListener, ComponentListener {
	private static final String JAVA_FRAME = "Java_COM_Frame";
	private static final String JAVA_URL = "http://java.com";
	private static final String ERROR_IMAGE_FILE = "sun/plugin/util/graybox_error.gif";

	private static final Color LOADING_BORDER = new Color(153, 153, 153);
	private static final Color ERROR_BORDER = new Color(204, 204, 204);

	private static final Color BACKGROUND_COLOR = Color.white;

	private static Image ERROR_IMAGE = null;
	

	private AnimationPanel m_panel = null;
	private Container	   m_parent;
	private int			   m_maxValue;
	private Image		   m_image = null;
	private	boolean		   m_error = false;

	public GrayBoxPanel(Container parent) {
		m_parent = parent;
		setBackground(Color.white);
		setForeground(Color.white);	
		setLayout(new BorderLayout());
	}


	public void setCustomImage(Image image) {
		setImage(image);
	}

	private synchronized Image getErrorImage() {
		if(ERROR_IMAGE == null){
			Toolkit tk = Toolkit.getDefaultToolkit();
			ERROR_IMAGE = tk.createImage(ClassLoader.getSystemResource(ERROR_IMAGE_FILE));
			MediaTracker mt = new MediaTracker(this);
			mt.addImage(ERROR_IMAGE, 0);
			try {
				mt.waitForID(0);
			} catch (InterruptedException e) {
			}
		}

		return ERROR_IMAGE;
	}

	public void setError() {
		if(m_error)
			return;

		m_error = true;
		setImage(getErrorImage());
	}

	public void setMaxProgressValue(int max) {
		m_maxValue = max;
	}

	public void progress(int curValue) {
		if(m_panel != null)
			m_panel.setProgressValue(((float)curValue)/m_maxValue);
		else 
			paint(m_parent.getGraphics());
	}

	public void start() {
		Dimension dm = m_parent.getSize();
		m_panel = new AnimationPanel(dm);
		m_panel.setCursor(new Cursor(Cursor.HAND_CURSOR));
		m_panel.addMouseListener(this);
		add(m_panel, BorderLayout.CENTER);
		m_panel.startAnimation();

		m_parent.addComponentListener(this);
	}

	public void stop() {
		if(m_panel != null)
			m_panel.stopAnimation();

		m_parent.removeComponentListener(this);
	}

	public void paint(Graphics g) {
		Dimension d = m_parent.getSize();
		// animation alive	
		if(m_panel != null){
			m_panel.repaint();
		}else {		
			g.setColor(BACKGROUND_COLOR);
			g.fillRect(0, 0, d.width, d.height);
			if(d.width > 24 && d.height > 24) {
				if(m_error) {
					drawImage(g, getErrorImage(), 4, 4);
				}else {
					drawImage(g, m_image, 1, 1);
				}
			}
			else {
				if(!m_error) {
					drawImage(g, m_image, 0, 0);
				}
			}
		}
		if(d.width > 24 && d.height > 24)
			drawBorder(g, d);
	}

	private void drawImage(Graphics g, Image image, int x, int y) {
		g.drawImage(image, x, y, BACKGROUND_COLOR, null);
	}

	private void drawBorder(Graphics g, Dimension d) {
		Color color = m_error?ERROR_BORDER:LOADING_BORDER;
		Color oldColor = g.getColor();
		g.setColor(color);
		g.drawRect(0, 0, d.width - 1, d.height - 1);
		g.setColor(oldColor);
	}

	private synchronized void setImage(Image image) {
		if(m_panel != null) {
			m_panel.stopAnimation();
			remove(m_panel);
			m_panel = null;
		}
		m_image = image;
		paint(m_parent.getGraphics());
	}

	

	public void mouseClicked(MouseEvent e) {
		AppletViewer v = ((AppletViewer)m_parent);
		try {
			v.getAppletContext().showDocument(new URL(JAVA_URL), JAVA_FRAME);
		}catch(Exception ex) {
		}

	}

	public void mouseEntered(MouseEvent e) {
		
	}

	public void mouseExited(MouseEvent e) {

	}

	public void mousePressed(MouseEvent e) {

	}

	public void mouseReleased(MouseEvent e) {

	}

	public void componentHidden(ComponentEvent e) {

	}

	public void componentMoved(ComponentEvent e) {

	}

	public void componentResized(ComponentEvent e) {
		Dimension d = m_parent.getSize();
		setSize(d);
		if(m_panel != null) {
			m_panel.setSize(d);
		}
	}

	public void componentShown(ComponentEvent e) {

 
	}

}

