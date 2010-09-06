/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Color;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.BufferedImage;
import java.awt.Dimension;
import java.awt.Canvas;
import java.awt.Cursor;
import javax.swing.Timer;

public class AnimationPanel extends Canvas implements ActionListener {
	private static final String		LOGO_IMAGE_LARGE = "sun/plugin/util/JavaCupLogo-161.png";
	private static final String		WATERMARK_IMAGE = "sun/plugin/util/java-watermark.gif";


    private static final double TWO_PI = 2 * Math.PI;
    private static final double HALF_PI = Math.PI / 2;

    private static final Color clrBorder = new Color(153, 153, 153);
    private static final Color clrBg1 = new Color(255, 255, 255, 0);
    private static final Color clrBg2 = Color.white;
    private static final Color clrRay1 = Color.white;
    private static final Color clrRay2 = new Color(180, 180, 180);
    private static final Color clrProgBg = Color.white;
    private static final Color clrProgBorder = Color.lightGray;
    private static final Color clrProgBar = Color.red;

    /** Maximum inner diameter of the burst */
    private static final int MAX_INNER_DIAMETER = 161 + 8;

    /** Maximum outer diameter of the burst */
    private static final int MAX_OUTER_DIAMETER =
        (int)(MAX_INNER_DIAMETER * 1.8);

    /** Number of rays in the burst */
    private int numRays;

    /** Distance of ray base from center of burst in pixels */
    private int rayStart;

    /** Distance of ray tip from center of burst in pixels */
    private int rayEnd;

    /** Width of ray base in pixels */
    private int rayWidth;

    /** Width of progress bar (including border) in pixels */
    private int progressWidth;

    /** Height of progress bar (including border) in pixels */
    private int progressHeight;

    /** Gap in pixels between progress border and filling */
    private int progressGap;

    /** Distance of progress bar below center of burst */
    private int progressYOff;

    private Polygon[] burstPoints;
    private BufferedImage background;
    private Image logo, backbuffer;
    private Timer timer;
    private float burstProgress, loadingProgress;
    private int width, height;
    private boolean showBurst, showLogoAndBar, showWatermark;
    private long startTime;


	public AnimationPanel(Dimension d){
        timer = new Timer(30, this);
        startTime = System.currentTimeMillis();
		loadingProgress = 0.0f; 
		width = d.width;
		height = d.height;
	}

	public void setProgressValue(float value) {
		loadingProgress = value;
	}

    private void initDimensions(int w, int h) {
        int minDim = (int)Math.min(w, h);

        numRays = 18;
        showBurst = (minDim >= 25);
        showLogoAndBar = (minDim >= 170);
        showWatermark = (minDim >= 400);

        int burstDiameter, innerDiameter;
        if (showWatermark) {
            // calculate burst diameter based on large logo dimensions
            innerDiameter = MAX_INNER_DIAMETER;
            burstDiameter = MAX_OUTER_DIAMETER;
        } else {
            if (showLogoAndBar) {
                // calculate burst diameter based on component dimensions
                burstDiameter = minDim - 8;
                if (burstDiameter > MAX_OUTER_DIAMETER) {
                    // the burst should stop growing at this boundary
                    burstDiameter = MAX_OUTER_DIAMETER;
                    innerDiameter = MAX_INNER_DIAMETER;
                } else {
                    // inner diameter is based on logo size
                    innerDiameter = (int)(burstDiameter / 1.8);
                }
            } else {
                // calculate burst diameter based on component dimensions
                burstDiameter = minDim - 4;
                // inner diameter is adjusted to increase the ray length
                innerDiameter = (int)(minDim * 0.35);
                // use fewer rays at this small size
                if (minDim < 100) {
                    numRays = 9;
                } else {
                    numRays = 12;
                }
            }
        }

        rayStart = innerDiameter / 2;
        rayEnd = burstDiameter / 2;
        rayWidth = (int)Math.max(burstDiameter / 40 + 1, 2);
    }

    private void initBackgroundImage(GraphicsConfiguration gc,
                                     int w, int h)
    {
        background = gc.createCompatibleImage(w, h);
        Graphics2D g = background.createGraphics();

        Toolkit tk = Toolkit.getDefaultToolkit();
        Image watermark =
            tk.createImage(ClassLoader.getSystemResource(WATERMARK_IMAGE));
        MediaTracker mt = new MediaTracker(this);
        mt.addImage(watermark, 0);
        try {
            mt.waitForID(0);
        } catch (InterruptedException e) {
        }

        // render the tiled watermark
        int imgw = watermark.getWidth(null);
        int imgh = watermark.getHeight(null);
        for (int y = 0; y < h; y += imgh) {
            for (int x = 0; x < w; x += imgw) {
                g.drawImage(watermark, x, y, null);
            }
        }

        // render the gradient multiple times to acheive the desired effect
        g.setPaint(new GradientPaint(0, 0, clrBg1,
                                     w/2, h/2, clrBg2, true));
        g.fillRect(0, 0, w, h);
        g.fillRect(0, 0, w, h);
        g.fillRect(0, 0, w, h);
        g.fillRect(0, 0, w, h);

        g.dispose();
    }

    private void initLogoImage(GraphicsConfiguration gc) {
        String filename = LOGO_IMAGE_LARGE;
        boolean scale = !showWatermark;

        Toolkit tk = Toolkit.getDefaultToolkit();
        Image origLogo = tk.createImage(ClassLoader.getSystemResource(filename));
        MediaTracker mt = new MediaTracker(this);
        mt.addImage(origLogo, 0);
        try {
            mt.waitForID(0);
        } catch (InterruptedException e) {
        }

        if (scale) {
            int origW = origLogo.getWidth(null);
            int origH = origLogo.getHeight(null);
            int h = (rayStart * 2) - 8;
            int w = (int)(((double)h / origH) * origW);
            logo = gc.createCompatibleImage(w, h);
            Graphics2D g2d = (Graphics2D)logo.getGraphics();
            g2d.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                                 RenderingHints.VALUE_INTERPOLATION_BICUBIC);
            g2d.drawImage(origLogo, 0, 0, w, h, Color.white, null);
            g2d.dispose();
        } else {
            logo = origLogo;
        }

        // progress bar width is relative to the width of the logo image
        progressWidth = logo.getWidth(null) * 2;
        progressHeight = 8;
        progressGap = 2;
        progressYOff = rayStart + 10;
    }

    private void initBurst(int ctrx, int ctry) {
        burstPoints = new Polygon[numRays];
        for (int i = 0; i < numRays; i++) {
            Polygon burst = new Polygon();
            double angle = ((double)i / numRays) * TWO_PI - HALF_PI;
            double tangent = angle + HALF_PI;
            double cos = Math.cos(angle);
            double sin = Math.sin(angle);
            int x1 = ctrx + (int)(cos * rayStart);
            int y1 = ctry + (int)(sin * rayStart);
            int x2 = ctrx + (int)(cos * rayEnd);
            int y2 = ctry + (int)(sin * rayEnd);
            int tcos = (int)(Math.cos(tangent) * rayWidth);
            int tsin = (int)(Math.sin(tangent) * rayWidth);
            burst.addPoint(x1-tcos, y1-tsin);
            burst.addPoint(x2, y2);
            burst.addPoint(x1+tcos, y1+tsin);
            burstPoints[i] = burst;
        }
    }

    public void paint(Graphics g) {
        int w = getWidth();
        int h = getHeight();
        int w2 = w / 2;
        int h2 = h / 2;

        if (backbuffer == null || w != width || h != height) {
            GraphicsConfiguration gc = getGraphicsConfiguration();
            backbuffer = createImage(w, h);
            initDimensions(w, h);
            if (showWatermark) {
                initBackgroundImage(gc, w, h);
            }
            if (showLogoAndBar) {
                initLogoImage(gc);
            }
            initBurst(w2, h2);
            width = w;
            height = h;
        }

        Graphics2D g2d = (Graphics2D)backbuffer.getGraphics();

        renderBackground(g2d, w, h);
        if (showBurst) {
            renderBurst(g2d, w2, h2);
        }
        if (showLogoAndBar) {
            renderLogo(g2d, w2, h2);
            renderProgress(g2d, w2 - (progressWidth / 2), h2 + progressYOff,
                           progressWidth, progressHeight, progressGap);
        }

        // render the thin border
        g2d.setColor(clrBorder);
        g2d.drawRect(0, 0, w-1, h-1);

        g2d.dispose();

        // copy the backbuffer to the screen
        g.drawImage(backbuffer, 0, 0, null);
    }

    public void update(Graphics g) {
        paint(g);
    }

    private void renderBackground(Graphics2D g2d, int w, int h) {
        if (showWatermark) {
            g2d.drawImage(background, 0, 0, null);
        } else {
            g2d.setColor(Color.white);
            g2d.fillRect(0, 0, w, h);
        }
    }

    private void renderBurst(Graphics2D g2d, int ctrx, int ctry) {
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                             RenderingHints.VALUE_ANTIALIAS_ON);

        int numpts = burstPoints.length;
        int minGradEnd = rayStart + ((rayEnd - rayStart) / 2);
        for (int i = 0; i < numpts; i++) {
            // render each ray of the burst
            double pct = (double)i / numpts;
            double angle = pct * TWO_PI - HALF_PI;
            double cos = Math.cos(angle);
            double sin = Math.sin(angle);
            double lenpct;
            if (pct > burstProgress) {
                lenpct = 1.0 - (pct - burstProgress);
            } else {
                lenpct = burstProgress - pct;
            }
            double length = minGradEnd + (lenpct * rayEnd);
            float x1 = ctrx + (float)(cos * rayStart);
            float y1 = ctry + (float)(sin * rayStart);
            float x2 = ctrx + (float)(cos * length);
            float y2 = ctry + (float)(sin * length);
            GradientPaint gp = new GradientPaint(x1, y1, clrRay1,
                                                 x2, y2, clrRay2);
            g2d.setPaint(gp);
            g2d.fillPolygon(burstPoints[i]);
        }

        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                             RenderingHints.VALUE_ANTIALIAS_OFF);
    }

    private void renderLogo(Graphics2D g2d, int ctrx, int ctry) {
        g2d.drawImage(logo,
                      ctrx-(logo.getWidth(null)/2),
                      ctry-(logo.getHeight(null)/2), null);
    }

    private void renderProgress(Graphics2D g2d,
                                int x, int y, int w, int h, int gap)
    {
        g2d.setColor(clrProgBg);
        g2d.fillRect(x, y, w, h);
        g2d.setColor(clrProgBorder);
        g2d.drawRect(x, y, w, h);
        g2d.setColor(clrProgBar);
        g2d.fillRect(x+gap, y+gap,
                     (int)(w * loadingProgress)-(gap+1), h-(gap+1));
    }

    public void actionPerformed(ActionEvent e) {
        repaint();

        long elapsed = System.currentTimeMillis() - startTime;
        burstProgress = (elapsed % 3000) / 3000.0f;
    }


    public void startAnimation() {
        timer.start();
    }


    public void stopAnimation(){
	timer.stop();
    }

}

