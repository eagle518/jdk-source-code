/*
 * @(#)GrayBoxPanel.java	1.21 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.net.*;
import com.sun.deploy.util.Trace;

import sun.plugin.util.UIUtil;

class GrayBoxPanel extends Panel implements ComponentListener, ImageObserver {

    private static final Color LOADING_BORDER = new Color(153, 153, 153);
    
    private Color backgroundColor;
    private Color foregroundColor;
    
    private ErrorPanel err_panel = null;
    
    private AnimationPanelBase m_panel = null;
    private static final boolean USE_NEW_ANIMATION = true;
    private Container	       m_parent;
    private int		       m_maxValue;
    private boolean            m_boxBorder = true;
    private Image	       m_image = null;
    private boolean            m_imageIsCentered = false;
    // Support for animated GIFs in the loading screen
    private boolean            m_animateImage = true;
    private boolean	       m_error = false;
    private ErrorDelegate errorDelegate = null;
    
    long startTimeMillis;
    int endSequenceMillis = 2000;
    
    public GrayBoxPanel(Container parent) {
	this(parent, Color.WHITE);
    }
    
    public GrayBoxPanel(Container parent, Color bgColor) {
	this(parent, bgColor, Color.BLACK);
    }
    
    public GrayBoxPanel(Container parent, Color bgColor, Color fgColor) {
	this(parent, bgColor, fgColor, null);
    }
    
    public GrayBoxPanel(Container parent, Color bgColor, Color fgColor, ErrorDelegate errorDelegate) {
	m_parent = parent;
	setBgColor(bgColor);
	setFgColor(fgColor);
	this.errorDelegate = errorDelegate;
	setLayout(new BorderLayout());
	UIUtil.disableBackgroundErase(this);
    }			

    public void setCustomImage(Image image, boolean centered) {
	setImage(image, centered);
    }
    
    public void setBgColor(Color bgColor){
	backgroundColor = bgColor;
	setBackground(bgColor);
	if (m_panel != null) {
	    m_panel.setBoxBGColor(bgColor);
	}
    }

    public void setFgColor(Color fgColor){
	foregroundColor = fgColor;
	setForeground(fgColor);
	if (m_panel != null) {
	    m_panel.setBoxFGColor(fgColor);
	}
    }
    
    public void setBoxBorder(boolean drawBorder) {
        m_boxBorder = drawBorder;
    }

    public void setError() {
        //  If there is animation - stop it and remove it.
        if(m_panel != null) {	
            m_panel.stopAnimation();
			
            // There is only Wrapper to remove, but we don't 
            // have any reference to it
            this.removeAll(); 
            m_panel = null;
            this.validate();
        }
        
        if(m_error){
            return;
        }			
        
        if (err_panel == null){
            err_panel = new ErrorPanel(backgroundColor, foregroundColor, m_parent, errorDelegate);
            add(err_panel, BorderLayout.CENTER);
            m_error = true;
            this.validate();

        }

        repaint();        
        
    }

	public void setMaxProgressValue(int max) {
		m_maxValue = max;
	}

	public void progress(int curValue) {
            long timeLeftMillis = ((m_maxValue-curValue) * 
                (System.currentTimeMillis() - startTimeMillis))/ (curValue + 1);  
            
            if(m_panel != null) {
                float value = ((float)curValue)/m_maxValue;

                // Our end sequence - fade and advertisement 
                // takes up 1500 milliseconds.  
                // If we have less then or equal to end sequence length
                //time left to download - start the end sequence.  
                if (timeLeftMillis > endSequenceMillis){  
                    m_panel.setProgressValue(value);
                }else{   
                    // fadeAway() will set the progress to its max.
                    m_panel.fadeAway();   
                }   
            } else {  
                repaint();
            }
	}

	public void start() {
		Dimension dm = m_parent.getSize();
		if (m_panel == null && m_image == null) {
                    if (USE_NEW_ANIMATION) {
                        m_panel = new AnimationPanel2();
                        m_panel.setCursor(new Cursor(Cursor.HAND_CURSOR));
                        add(m_panel, BorderLayout.CENTER);
                    } else {
                        m_panel = new AnimationPanel();
                        m_panel.setCursor(new Cursor(Cursor.HAND_CURSOR));
                        add(new AnimationPanel.Wrapper((AnimationPanel) m_panel), 
                            BorderLayout.CENTER);
                    }
                    m_panel.setBoxBGColor(backgroundColor);
                    m_panel.setBoxFGColor(foregroundColor);
		}

                startTimeMillis = System.currentTimeMillis();

                if (m_panel != null) {
                    m_panel.startAnimation();
                } else if (m_image != null) {
                    m_animateImage = true;
                    repaint();
                }

		m_parent.addComponentListener(this);
	}

	public void stop() {
		if (m_panel != null) {
		    m_panel.stopAnimation();
		}
                m_animateImage = false;
		m_parent.removeComponentListener(this);
	}


        // Used for double buffering the custom image, in particular
        // when it's an animated GIF
        private Image backBufferImage;
        private int backBufferImageWidth;
        private int backBufferImageHeight;

	public void update(Graphics g) {
            // Override the AWT's default background clearing behavior
            paint(g);
        }

	public void paint(Graphics g) {
            try {
		Dimension d = m_parent.getSize();
		// animation alive	
		if(m_panel != null){
                    m_panel.repaint();
		}else {		
                    if(m_error) {
                        err_panel.repaint();
                    } else {
                        if (backBufferImage == null ||
                            backBufferImageWidth != d.width ||
                            backBufferImageHeight != d.height) {
                            if (backBufferImage != null) {
                                backBufferImage.flush();
                            }

                            // Allocate a new back buffer
                            int w = Math.max(1, d.width);
                            int h = Math.max(1, d.height);
                            backBufferImage = createImage(w, h);
                            backBufferImageWidth = w;
                            backBufferImageHeight = h;
                        }

                        Graphics bbg = backBufferImage.getGraphics();
                        bbg.setColor(backgroundColor);
                        bbg.fillRect(0, 0, backBufferImageWidth, backBufferImageHeight);
                        if (m_image != null) {
                            int imgWidth = m_image.getWidth(this);
                            int imgHeight = m_image.getHeight(this);
                            if (m_imageIsCentered && imgWidth >= 0 && imgHeight >= 0) {
                                drawImage(bbg, m_image,
                                          (backBufferImageWidth - imgWidth) / 2,
                                          (backBufferImageHeight - imgHeight) / 2);
                            } else {
                                // Preserve old logic exactly
                                if(d.width > 24 && d.height > 24) {
                                    drawImage(bbg, m_image, 1, 1);
                                } else {
                                    drawImage(bbg, m_image, 0, 0);
                                }
                            }
                        }

                        // Now draw the back buffer image in one shot
                        g.drawImage(backBufferImage, 0, 0, null);
                    }
		}
                // The box border doesn't look good -- flickers in some
                // situations -- provide the ability to turn it off,
                // though it needs to be on by default for backward
                // compatibility
                if (m_boxBorder) {
                    if (d.width > 24 && d.height > 24)
                        drawBorder(g, d);
                }
            } catch (Exception e) {
                Trace.ignoredException(e);
            }
	}

        public boolean imageUpdate(Image img,
                                   int infoflags,
                                   int x,
                                   int y,
                                   int width,
                                   int height) {
            if ((infoflags & ImageObserver.FRAMEBITS) != 0) {
                if (m_animateImage) {
                    repaint();
                }
                return m_animateImage;
            }

            if ((infoflags & ImageObserver.ALLBITS) != 0) {
                repaint();
            }
            return true;
        }

	private void drawImage(Graphics g, Image image, int x, int y) {
		g.drawImage(image, x, y, backgroundColor, this);
	}

	private void drawBorder(Graphics g, Dimension d) {
		Color color = LOADING_BORDER;
		Color oldColor = g.getColor();
		g.setColor(color);
		g.drawRect(0, 0, d.width - 1, d.height - 1);
		g.setColor(oldColor);
	}

	private synchronized void setImage(Image image, boolean centered) {
		if(m_panel != null) {
			m_panel.stopAnimation();
                        // There is only Wrapper to remove, but we don't
                        // have any reference to it 
			this.removeAll();
			m_panel = null;
		}
		m_image = image;
                m_imageIsCentered = centered;
		repaint();
	}

	public void componentResized(ComponentEvent e) {
		Dimension d = m_parent.getSize();
		setSize(d);
		if(m_panel != null) {
			m_panel.setSize(d);
		}
	}

	public void componentShown(ComponentEvent e) {}
        public void componentMoved(ComponentEvent e) {}
        public void componentHidden(ComponentEvent e) {}
}

