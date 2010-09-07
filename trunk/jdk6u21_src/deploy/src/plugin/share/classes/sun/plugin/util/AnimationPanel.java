/*
 * @(#)AnimationPanel.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Component;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.LinearGradientPaint;
import java.awt.MediaTracker;
import java.awt.Paint;
import java.awt.Panel;
import java.awt.RadialGradientPaint;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.Toolkit;
import java.awt.Transparency;
//import java.awt.event.ActionEvent;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.GeneralPath;
import java.awt.image.BufferedImage;

import java.security.*;
import com.sun.deploy.util.Trace;

/**
 * This class implements the updated Java Plug-in "loading progress"
 * animation.  The animation can be broken down into a number of distinct
 * segments, or "sequences":
 *   1.  Logo fades into center on solid dark orange background
 *   2.  Thin dark curve appears on left side
 *   3a. Left side breaks off
 *   3b. White flash
 *   3c. Solid background dissolves to gradient background
 *   4a. Logo pulsates
 *   4b. Progress bar animates
 *   5.  Fade to white / Cross fade to "Java.com" text
 *   6.  Freeze on "Come visit..." text for a short period of time
 *   7.  Text fades to white
 *
 * Each sequence has an ideal duration time (specified in milliseconds).  The
 * fourth sequence (containing the progress bar) is a bit of a special case,
 * since it needs to run as long as the plugin is still loading resources.
 *
 * Note that this source code makes use of some features available only in
 * JDK 6 and above (namely LinearGradientPaint and AlphaComposite.derive()),
 * but I have pointed out in those places alternate code that will allow
 * this class to be backported to JDK 5 if necessary.
 */
public class AnimationPanel extends AnimationPanelBase implements Runnable {

    private static boolean DEBUG;
    
    private static final String IMAGE_EXT = ".png";
    private static final String LOGO_IMAGE = "javalogo";
    private static final String GLOW_IMAGE = "javaglow";
    private static final String JAVACOM_IMAGE = "javacom";
    private static final String DROPTEXT_IMAGE = "droptext";

    /** Number of milliseconds in one full glow cycle */
    private static final int GLOW_CYCLE_TIME = 2000;
    /** Number of milliseconds over which to animate a progress bar update */
    private static final int ZIPPY_PULSE_TIME = 300;
    
    private static final long[] stateStops = new long[] {
        0, 300, 500, 400, 99999999, 750, 950, 300, 1000,
    };

    private static final Color[] bgColors = new Color[] {
        new Color(255, 229, 203),
        new Color(247, 171, 103),
        new Color(255, 124,  15),
        new Color(255, 117,   0),
        new Color(255, 144,   0),
        new Color(255, 165,   0),
    };
    private static final float[] bgStops = new float[] {
        0.0f, 0.15f, 0.39f, 0.60f, 0.83f, 1.0f,
    };
    
    private static final boolean isOnJDK6;
    static {
        String version = (String) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return System.getProperty("java.version");
                }
            });
        isOnJDK6 = (version.compareTo("1.6") >= 0);
    }

    // the following is only needed for backporting to JDK 5
    private static final GradientPaint[] bgGrads =
        new GradientPaint[bgStops.length-1];

    private static final Color clrBgSolid = new Color(231, 111, 0);
    private static final Color clrProgBg = new Color(231, 111, 0);
    private static final Color clrProgBar = Color.WHITE;
    private static final Color clrProgGlow1 = new Color(1.0f, 1.0f, 1.0f, 0.5f);
    private static final Color clrProgGlow2 = new Color(1.0f, 1.0f, 0.8f, 0.0f);
    private static final Color clrCurve1 = new Color(201, 104, 0);
    private static final Color clrCurve2 = Color.LIGHT_GRAY;
    private static final Color clrGlowInnerHi = new Color(253, 239, 175, 48);
    private static final Color clrGlowInnerLo = new Color(255, 209, 0);
    private static final Color clrGlowOuterHi = new Color(253, 239, 175, 24);
    private static final Color clrGlowOuterLo = new Color(255, 179, 0);

    /** Sun 'S' curve constants */
    private static final float curveWidth = 0.18f;
    private static final float curveY1    = 0.40f;
    private static final float curveY2    = 0.50f;

    private Color[] shadowColors;
    private Paint gradProgGlowL, gradProgGlowR, bgGradient;
    private Shape fullBg, leftBgSolid, leftBgGrad, rightBgSolid, rightBgGrad;
    private Shape thinCurve;
    private BufferedImage leftImage, rightImage;

    private float minGap, maxGap;
    private float shadowWidth, innerGlowWidth, cornerRadius;
    private int logoX, logoY;
    private int glowX, glowY;
    private int javaComX, javaComY;
    private int dropTextX, dropTextY;
    private int thinCurveX;
    private int progressWidth, progressHeight;
    private int progressX, progressY;
    private int progGlowRadius;

    private Image imgLogo, imgGlow, imgJavaCom, imgDropText;
    private boolean preloadedImages=false;
    private Image backbuffer=null;
    private float loadingProgress;
    private float zippyProgress, zippyStartProgress;
    private long zippyStartTime;
    private int width, height;
    private long startTime;
    private int currentState;
    private float stateProgress, pulseProgress;
    private boolean showText, showProgress;
    private boolean fadeAway;
    private boolean paused; // REMIND: debug only
    private boolean preloadedBackground=false;
    private boolean preloadedAll=false;

    public static boolean animationThreadRunning = false;
    
    public AnimationPanel() {
        currentState = 1;
        
        if (DEBUG) {
            addMouseListener(new MouseAdapter() {
                public void mousePressed(MouseEvent e) {
                    paused = !paused;
                }
            });
        }

        UIUtil.disableBackgroundErase(this);
    }
    
    private Shape createFullShape(float x1, float y1, float x2, float y2) {
        float arc = cornerRadius;
        GeneralPath gp = new GeneralPath();
        gp.moveTo(x1+arc, y1);
        gp.lineTo(x2-arc, y1);
        gp.quadTo(x2, y1, x2, y1+arc);
        gp.lineTo(x2, y2-arc);
        gp.quadTo(x2, y2, x2-arc, y2);
        gp.lineTo(x1+arc, y2);
        gp.quadTo(x1, y2, x1, y2-arc);
        gp.lineTo(x1, y1+arc);
        gp.quadTo(x1, y1, x1+arc, y1);
        gp.closePath();
        return gp;
    }

    private Shape createLeftShape(float x1, float y1, float x2, float y2,
                                  float bgw, float bgh)
    {
        float adj = 1.0f; // helps round out the sharp corners
        float arc = cornerRadius;
        float dcx = bgw*curveWidth;
        float cx1 = x2-dcx;
        float cy1 = bgh*curveY1;
        float cx2 = x2+dcx;
        float cy2 = bgh*curveY2;
        GeneralPath lt = new GeneralPath();
        lt.moveTo(x1+arc, y1);
        lt.lineTo(x2-adj, y1);
        lt.quadTo(x2, y1, x2, y1+adj);
        lt.curveTo(cx1, cy1, cx2, cy2, x2, y2-adj);
        lt.quadTo(x2, y2, x2-adj, y2);
        lt.lineTo(x1+arc, y2);
        lt.quadTo(x1, y2, x1, y2-arc);
        lt.lineTo(x1, y1+arc);
        lt.quadTo(x1, y1, x1+arc, y1);
        lt.closePath();
        return lt;
    }

    private Shape createRightShape(float x1, float y1, float x2, float y2,
                                   float bgw, float bgh)
    {
        float adj = 1.0f; // helps round out the sharp corners
        float arc = cornerRadius;
        float dcx = bgw*curveWidth;
        float cx1 = x1-dcx;
        float cy1 = bgh*curveY1;
        float cx2 = x1+dcx;
        float cy2 = bgh*curveY2;
        GeneralPath rt = new GeneralPath();
        rt.moveTo(x1-adj, y1+adj);
        rt.quadTo(x1, y1, x1+adj, y1);
        rt.lineTo(x2-arc, y1);
        rt.quadTo(x2, y1, x2, y1+arc);
        rt.lineTo(x2, y2-arc);
        rt.quadTo(x2, y2, x2-arc, y2);
        rt.lineTo(x1+adj, y2);
        rt.quadTo(x1, y2, x1, y2-adj);
        rt.curveTo(cx2, cy2, cx1, cy1, x1-adj, y1+adj);
        rt.closePath();
        return rt;
    }

    private BufferedImage createTranslucentImage(int w, int h) {
        GraphicsConfiguration gc = getGraphicsConfiguration();
        if(null==gc) return null;
        return gc.createCompatibleImage(w, h, Transparency.TRANSLUCENT);
    }
    
    /**
     * Renders the given shape into an image so that on subsequent repaints
     * we only need to render the image, which will be much faster than
     * rendering the complex shape and all its associated effects on each
     * frame.
     */
    private BufferedImage createGradientShapeImage(Shape s,
                                                   int imgw, int imgh)
    {
        BufferedImage img = createTranslucentImage(imgw, imgh);
        if(null==img) return null;
        Graphics2D gimg = img.createGraphics();
        gimg.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                              RenderingHints.VALUE_ANTIALIAS_ON);
        renderBorderShadow(gimg, s);
        
        // we need another intermediate image here so that we can make
        // the "soft clipping" effect in renderBorderGlow() work properly
        BufferedImage tmp = createTranslucentImage(imgw, imgh);
        if(null==tmp) {
            if(null!=gimg) {
                gimg.dispose();
            }
            return null;
        }
        Graphics2D gtmp = tmp.createGraphics();
        gtmp.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                              RenderingHints.VALUE_ANTIALIAS_ON);
        gtmp.setComposite(AlphaComposite.Clear);
        gtmp.fillRect(0, 0, imgw, imgh);
        gtmp.setComposite(AlphaComposite.SrcOver);
        renderGradientShape(gtmp, s, imgw, imgh);
        renderBorderGlow(gtmp, s, imgh);
        gtmp.dispose();

        gimg.drawImage(tmp, 0, 0, null);
        gimg.dispose();
        
        return img;
    }
    
    private void initMultiStopGradient(int h) {
        if (isOnJDK6) {
            bgGradient = new LinearGradientPaint(0, 0, 0, h, bgStops, bgColors);
        } else {
            // the following code is only needed for backporting to JDK 5,
            // since LinearGradientPaint is only available in JDK 6 and beyond
            for (int i = 0; i < bgGrads.length; i++) {
                float y1 = bgStops[i] * h;
                float y2 = bgStops[i+1] * h;
                bgGrads[i] =
                    new GradientPaint(0, bgStops[i] * h, bgColors[i],
                                      0, bgStops[i+1] * h, bgColors[i+1]);
            }
        }
    }
    
    private void initBackground(int w, int h) {
        if(preloadedBackground) return;

        initMultiStopGradient(h);

        // shadow border width
        float bw = shadowWidth;
        thinCurveX = (int)(w*0.27f);

        // solid background is less wide than tall to account for "S" curve gap
        float adjx = bw*2+minGap;

        // initialize background (rounded rect) shape
        float x1 = bw+adjx;
        float y1 = bw;
        float x2 = w-bw;
        float y2 = h-bw;
        fullBg = createFullShape(x1, y1, x2, y2);

        // initialize left part of split background shape
        x1 = bw;
        y1 = bw;
        x2 = thinCurveX - (minGap/2) - bw;
        y2 = h-bw;
        leftBgSolid = createLeftShape(x1, y1, x2, y2, w, h);

        // cache left curve/gradient shape in image (left edge of the shape
        // will align with the left edge of the cached image)...

        // width of the shape (not including shadow borders)
        float sw = thinCurveX-(bw*2)-(minGap/2);

        // make image as wide as shape plus a little extra since the curve
        // bulges out to the right beyond x2
        int imgw = (int)(sw + (sw*0.8f));
        int imgh = h;
        leftBgGrad = createLeftShape(x1, y1, x2, y2, w, h);
        leftImage = createGradientShapeImage(leftBgGrad, imgw, imgh);
        if(null==leftImage) return;

        // initialize right part of split background shape
        x1 = thinCurveX + (minGap/2) + bw;
        x2 = w-bw;
        rightBgSolid = createRightShape(x1, y1, x2, y2, w, h);

        // cache right curve/gradient shape in image (right edge of the
        // shape will align with the right edge of the cached image)...

        // width of the right shape (not including shadow borders)
        sw = w-thinCurveX-(bw*2)-(minGap/2);
        if (!showText) {
            // hack to make the gap just a hair wider in the 25x case
            sw -= 1.0f;
        }

        // make image as wide as shape plus a little extra since the curve
        // bulges out to the left beyond x1
        imgw = (int)(sw + (sw*0.3f));
        imgh = h;
        x2 = imgw-bw;
        x1 = x2-sw;
        rightBgGrad = createRightShape(x1, y1, x2, y2, w, h);
        rightImage = createGradientShapeImage(rightBgGrad, imgw, imgh);
        if(null==rightImage) return;

        // initialize thin curve shape
        float dcx = w*curveWidth;
        float cy1 = h*curveY1;
        float cy2 = h*curveY2;
        GeneralPath tc = new GeneralPath();
        tc.moveTo(0.0f, y1);
        tc.curveTo(-dcx, cy1, dcx, cy2, 0.0f, y2-1);
        thinCurve = tc;

        preloadedBackground = true;
    }

    private Image loadImage(String basename, String size) {
        Toolkit tk = Toolkit.getDefaultToolkit();
        String filename = basename + size + IMAGE_EXT;
        return tk.createImage(getClass().getResource(filename));
    }

    private static Color[] createShadowColors(int[] vals) {
        Color[] clrs = new Color[vals.length];
        for (int i = 0; i < vals.length; i++) {
            int val = vals[i];
            clrs[i] = new Color(val, val, val);
        }
        return clrs;
    }

    private void initImages(int w, int h) {
        if(preloadedImages) return;

        String size;
        if (w < 100) {
            size = "25";
        } else if (w < 170) {
            size = "100";
        } else if (w < 300) {
            size = "170";
        } else {
            size = "300";
        }

        Toolkit tk = Toolkit.getDefaultToolkit();
        MediaTracker mt = new MediaTracker(this);
        imgLogo = loadImage(LOGO_IMAGE, size);
        mt.addImage(imgLogo, 0);
        imgGlow = loadImage(GLOW_IMAGE, size);
        mt.addImage(imgGlow, 1);
        if (showText) {
            // these images are only defined for sizes of at least 100
            imgJavaCom = loadImage(JAVACOM_IMAGE, size);
            mt.addImage(imgJavaCom, 2);
            imgDropText = loadImage(DROPTEXT_IMAGE, size);
            mt.addImage(imgDropText, 3);
        }
        try {
            mt.waitForAll();
        } catch (InterruptedException e) {
        }

        // logo centerpoint is relative to the size of the orange box
        int ctrX = (int)(w*0.66);
        int ctrY = showProgress ? (int)(h*0.46) : (int)(h*0.50);
        logoX = ctrX - (imgLogo.getWidth(null)  / 2);
        logoY = ctrY - (imgLogo.getHeight(null) / 2);
        glowX = ctrX - (imgGlow.getWidth(null)  / 2);
        glowY = ctrY - (imgGlow.getHeight(null) / 2);

        // initialize variables that are relative to logo/box centers
        ctrY = h / 2;
        if (w < 100) {
            progressWidth  = 10;
            progressHeight = 1;
            progressY      = logoY + imgLogo.getHeight(null) + 1;
            dropTextY      = 0;    // no drop text in this case
            shadowWidth    = 0.0f; // no shadow in this case
            shadowColors   = null; // no shadow in this case
            innerGlowWidth = 0.0f; // no glow in this case
            cornerRadius   = 3.0f;
            minGap         = 1.0f;
            maxGap         = 1.0f;
        } else if (w < 170) {
            progressWidth  = 37;
            progressHeight = 1;
            progressY      = ctrY + 36;
            dropTextY      = 5;
            shadowWidth    = 2.0f;
            shadowColors   = createShadowColors(new int[] {0xc4, 0xff});
            innerGlowWidth = 3.0f;
            cornerRadius   = 4.0f;
            minGap         = 2.0f;
            maxGap         = shadowWidth * 3;
        } else if (w < 300) {
            progressWidth  = 58;
            progressHeight = 2;
            progressY      = ctrY + 55;
            dropTextY      = 7;
            shadowWidth    = 3.0f;
            shadowColors   = createShadowColors(new int[] {0xc4, 0xed, 0xff});
            innerGlowWidth = 4.0f;
            cornerRadius   = 5.0f;
            minGap         = 2.0f;
            maxGap         = shadowWidth * 3;
        } else {
            progressWidth  = 83;
            progressHeight = 2;
            progressY      = ctrY + 78;
            dropTextY      = 10;
            shadowWidth    = 5.0f;
            shadowColors   = createShadowColors(new int[] {0xc4, 0xe0, 0xed, 0xf6, 0xff});
            innerGlowWidth = 5.0f;
            cornerRadius   = 6.0f;
            minGap         = 1.0f;
            maxGap         = shadowWidth * 3;
        }
        progressX = ctrX - (progressWidth / 2);

        if (isOnJDK6) {
            // progress bar glow size is relative to the progress dimensions
            progGlowRadius = progressHeight*3;
        
            float[] fractions = {0.0f, 1.0f};
            Color[] colors = {clrProgGlow1, clrProgGlow2};
            float radius = progGlowRadius;
            float cx = progressX + 1;
            float cy = progressY + (progressHeight / 2.0f);

            gradProgGlowR =
                new RadialGradientPaint(cx+0.5f, cy-0.5f,
                                        radius, fractions, colors);
            gradProgGlowL =
                new LinearGradientPaint(cx, cy-(radius*2)-0.5f,
                                        cx, cy-radius-0.5f,
                                        fractions, colors,
                                        LinearGradientPaint.CycleMethod.REFLECT);
        }

        if (showText) {
            int jh = imgJavaCom.getHeight(null);

            // in the 170x case, the "Java.com" text will dangle off the
            // right edge unless we adjust it by a couple pixels (this
            // hack isn't necessary at the other sizes)
            int xadj = (w < 170) ? 2 : 0;
            
            // "Java.com" text is relative to the logo image
            javaComX = logoX-xadj;
            javaComY = logoY + imgLogo.getHeight(null) - jh;

            // "Visit us..." text is relative to "Java.com" text
            dropTextX = javaComX - imgDropText.getWidth(null) + xadj;
            dropTextY = javaComY + jh - dropTextY - imgDropText.getHeight(null);
        }
        preloadedImages=true;
    }
    
    /**
     * Loads the resources (images, shapes, etc) for the current size of
     * the component.  This only needs to be called once at startup, or in
     * the event that the component is resized at runtime (this is usually
     * only an issue for the demo version that allows user resizing).
     */
    public void preloadResources(int w, int h) {
        // reset preloaded* states, if dimension reshaped, etc.
        if (null==backbuffer || w != width || h != height || !preloadedAll) {
            preloadedImages=false;
            preloadedBackground=false;
            preloadedAll=false;
        }

        if(preloadedAll) return; // nothing todo ..

        int size = getBoxSize(w, h);
        
        backbuffer = createImage(w, h);
        showText = (size >= 100);
        showProgress = (size >= 100);

        initImages(size, size);
        if(!preloadedImages) return;

        initBackground(size, size);
        if(!preloadedBackground) return;

        width = w;
        height = h;

        preloadedAll=true;
    }

    /**
     * Returns the size of the orange box given the dimensions of the
     * available destination area.
     */
    private static int getBoxSize(int w, int h) {
        int mindim = (w > h) ? h : w;
        if (mindim < 25) {
            return 0;
        } else if (mindim < 100) {
            return 25;
        } else if (mindim < 170) {
            return 100;
        } else if (mindim < 300) {
            return 170;
        } else {
            return 300;
        }
    }
    
    /**
     * Returns the bounds of the orange box given the dimensions of the
     * available destination area.
     */
    private static Rectangle getBoxBounds(int w, int h) {
        int size = getBoxSize(w, h);
        int x, y;
        if (w < 600 && h < 600) {
            // at normal sizes we will center the orange box within the
            // available area
            x = w/2 - size/2;
            y = h/2 - size/2;
        } else {
            // beyond that we will simply anchor the orange box to the
            // upper-left corner
            x = 0;
            y = 0;
        }
        return new Rectangle(x, y, size, size);
    }

    public void doPaint(Graphics g) {
        int w = getWidth();
        int h = getHeight();
        
        int size = getBoxSize(w, h);
        if (size <= 0) {
            // REMIND: we should turn off the timer in this case
            return;
        }

        preloadResources(w, h);
        if (!preloadedAll) {
            return; // bail out
        }

        Graphics2D g2d = (Graphics2D)backbuffer.getGraphics();
        g2d.setColor(Color.WHITE);
        g2d.fillRect(0, 0, w, h);

        w = h = size;

        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                             RenderingHints.VALUE_ANTIALIAS_ON);

        switch (currentState) {
            case 1:
                renderState1(g2d, w, h);
                break;
            case 2:
                renderState2(g2d, w, h);
                break;
            case 3:
                renderState3(g2d, w, h);
                break;
            case 4:
                renderState4(g2d, w, h);
                break;
            case 5:
                renderState5(g2d, w, h);
                break;
            case 6:
                renderState6(g2d, w, h);
                break;
            case 7:
                renderState7(g2d, w, h);
                break;
            default:
                break;
        }

        g2d.dispose();

        // copy the backbuffer to the screen
        g.drawImage(backbuffer, 0, 0, null);
    }

    /**
     * 1. Logo fades into center on solid dark orange background
     */
    private void renderState1(Graphics2D g2d, int w, int h) {
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, stateProgress));
        renderSolidBackground(g2d);
        renderLogo(g2d);
        g2d.setComposite(oldcomp);
    }

    /**
     * 2. Thin dark curve appears on left side
     */
    private void renderState2(Graphics2D g2d, int w, int h) {
        renderSolidBackground(g2d);
        renderLogo(g2d);

        if (stateProgress <= 0.5f) {
            float prog = stateProgress / 0.5f;
            renderAnimCurve(g2d, thinCurveX, h, prog);
        } else {
            float prog = (1.0f-stateProgress) / 0.5f;
            Composite oldcomp = g2d.getComposite();
            g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, prog));
            g2d.setColor(clrCurve1);
            renderCurve(g2d, thinCurveX);
            g2d.setComposite(oldcomp);
        }
    }

    /**
     * 3a. Left side breaks off
     * 3b. White flash
     * 3c. Solid background dissolves to gradient background
     */
    private void renderState3(Graphics2D g2d, int w, int h) {
        renderSplitBackground(g2d, w, h, stateProgress, true);
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f-stateProgress));
        renderSplitBackground(g2d, w, h, stateProgress, false);
        g2d.setComposite(oldcomp);
        renderLogo(g2d);
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, getFlashAlpha()));
        g2d.setColor(Color.white);
        // flash region is a little bigger than the orange box to account
        // for the left part of the shape that moves outside the bounding box
        // to the left
        g2d.fillRect(-50, 0, w+50, h);
        g2d.setComposite(oldcomp);
    }

    /**
     * 4a. Logo pulsates
     * 4b. Progress bar animates
     */
    private void renderState4(Graphics2D g2d, int w, int h) {
        renderSplitBackground(g2d, w, h, 1.0f, true);
        renderLogo(g2d);
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, getPulseAlpha()));
        renderGlow(g2d);
        g2d.setComposite(oldcomp);
        if (showProgress) {
            renderProgress(g2d);
        }
    }

    /**
     * 5. Fade to white / Cross fade to "Java.com" text
     */
    private void renderState5(Graphics2D g2d, int w, int h) {
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f - stateProgress));
        renderSplitBackground(g2d, w, h, 1.0f, true);
        renderLogo(g2d);
        float prog = getPulseAlpha() * (1.0f - stateProgress);
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, prog));
        renderGlow(g2d);
        if (showText) {
            g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, stateProgress));
            renderVisitUsText(g2d);
            renderJavaText(g2d);
        }
        g2d.setComposite(oldcomp);
    }

    /**
     * 6. Freeze on "Come visit..." text for a short period of time
     */
    private void renderState6(Graphics2D g2d, int w, int h) {
        renderVisitUsText(g2d);
        renderJavaText(g2d);
    }

    /**
     * 7. Text fades to white
     */
    private void renderState7(Graphics2D g2d, int w, int h) {
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f - stateProgress));
        renderVisitUsText(g2d);
        renderJavaText(g2d);
        g2d.setComposite(oldcomp);
    }

    private void renderSolidShape(Graphics2D g2d, Shape s) {
        g2d.setColor(clrBgSolid);
        g2d.fill(s);
    }

    private void renderSolidBackground(Graphics2D g2d) {
        renderSolidShape(g2d, fullBg);
    }

    private void renderGradientShape(Graphics2D g2d, Shape s, int w, int h) {
        if (isOnJDK6) {
            g2d.setPaint(bgGradient);
            g2d.fill(s);
        } else {
            // the following code is only needed for backporting to JDK 5; on
            // JDK 6 and beyond we can render the shape in a single pass using
            // LinearGradientPaint, as is done above

            Shape oldclip = g2d.getClip();
            for (int i = 0; i < bgGrads.length; i++) {
                int y1 = (int)(bgStops[i] * h);
                int y2 = (int)(bgStops[i+1] * h);
                g2d.setPaint(bgGrads[i]);
                // add 1 to height here to workaround Mac OS X clipping issues
                g2d.clipRect(0, y1, w, y2-y1+1);
                if (h > 25) {
                    // at smaller sizes we seem to hit clipping bugs on Mac OS X
                    //g2d.clip(s);
                }
                g2d.fill(s);
                g2d.setClip(oldclip);
            }
        }
    }

    private void renderGradientBackground(Graphics2D g2d, int w, int h) {
        renderGradientShape(g2d, fullBg, w, h);
    }

    private void renderSplitBackground(Graphics2D g2d, int w, int h,
                                       float prog, boolean gradient)
    {
        float tx;
        if (prog <= 0.6f) {
            // for the first 60% of the sequence, slide the left shape from
            // its original location (part of the "full shape") over to the
            // left (so that it is touching the edge of the white box)
            tx = (shadowWidth*2) + (-maxGap * (prog / 0.6f));
        } else {
            // for the remaining 40% of the sequence, slide the left shape
            // from its furthest extent to its "resting location", such that
            // a gap forms between the left and right parts of the full shape
            tx = -shadowWidth + (shadowWidth * ((prog - 0.6f) / 0.4f));
        }

        AffineTransform oldxform = g2d.getTransform();
        g2d.translate(tx, 0);
        if (gradient) {
            g2d.drawImage(leftImage, 0, 0, null);
        } else {
            renderSolidShape(g2d, leftBgSolid);
        }
        g2d.setTransform(oldxform);

        if (gradient) {
            g2d.drawImage(rightImage, w-rightImage.getWidth(), 0, null);
        } else {
            renderSolidShape(g2d, rightBgSolid);
        }
    }

    private Color getMixedColor(Color c1, float pct1, Color c2, float pct2) {
        float[] clr1 = c1.getComponents(null);
        float[] clr2 = c2.getComponents(null);
        for (int i = 0; i < clr1.length; i++) {
            clr1[i] = (clr1[i] * pct1) + (clr2[i] * pct2);
        }
        return new Color(clr1[0], clr1[1], clr1[2], clr1[3]);
    }

    /**
     * Renders the drop shadow that appears just outside the border of
     * the background shapes.
     */
    private void renderBorderShadow(Graphics2D g2d, Shape s) {
        if (shadowColors == null) {
            return;
        }

        for (int i = shadowColors.length; i >= 1; i--) {
            Color c = shadowColors[i-1];
            g2d.setStroke(new BasicStroke((float)i*2));
            g2d.setColor(c);
            g2d.draw(s);
        }
    }

    /**
     * Renders the yellowish glow that appears just inside the border of
     * the background shapes.
     */
    private void renderBorderGlow(Graphics2D g2d, Shape s, int h) {
        g2d = (Graphics2D)g2d.create();

        int bw = (int)(innerGlowWidth*2);
        if (bw == 0) {
            // render a special 1-pixel wide border in the 25px case
            Color c1 = new Color(0xfffdaa5d);
            Color c2 = new Color(0xfffc720c);
            if (s == leftBgGrad) {
                g2d.setColor(c1);
                g2d.fillRect(2, 0, 3, 1);
                g2d.fillRect(1, 1, 1, 1);
                g2d.setPaint(new GradientPaint(0, 0, c1, 0, h, c2));
                g2d.fillRect(0, 2, 1, h-4);
                g2d.setColor(c2);
                g2d.fillRect(1, h-2, 1, 1);
                g2d.fillRect(2, h-1, 3, 1);
            } else {
                g2d.setColor(c1);
                g2d.fillRect(4, 0, 16, 1);
                g2d.fillRect(20, 1, 1, 1);
                g2d.setPaint(new GradientPaint(0, 0, c1, 0, h, c2));
                g2d.fillRect(21, 2, 1, h-4);
                g2d.setColor(c2);
                g2d.fillRect(20, h-2, 1, 1);
                g2d.fillRect(4, h-1, 16, 1);
            }
            return;
        }

        for (int i = bw; i >= 2; i-=2) {
            float pct = (float)(bw - i) / (float)(bw - 1);
            Color mixHi = getMixedColor(clrGlowInnerHi, pct,
                                        clrGlowOuterHi, 1.0f - pct);
            Color mixLo = getMixedColor(clrGlowInnerLo, pct,
                                        clrGlowOuterLo, 1.0f - pct);
            g2d.setPaint(new GradientPaint(0.0f, h*0.5f,  mixHi,
                                           0.0f, h-bw, mixLo));
            // here we use SrcAtop to acheive a kind of soft clipping effect:
            //   1. clear the (temporary) translucent image
            //   2. fill the gradient shape into that image
            //   3. stroke the border glow using SrcAtop so that only the
            //      pixels that fall within the gradient shape (i.e. those that
            //      correspond to non-zero alpha values in the destination)
            //      are rendered
            g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_ATOP, pct));
            g2d.setStroke(new BasicStroke(i));
            g2d.draw(s);
        }

        g2d.dispose();
    }

    private void renderLogo(Graphics2D g2d) {
        g2d.drawImage(imgLogo, logoX, logoY, null);
    }

    private void renderGlow(Graphics2D g2d) {
        g2d.drawImage(imgGlow, glowX, glowY, null);
    }

    private void renderJavaText(Graphics2D g2d) {
        g2d.drawImage(imgJavaCom, javaComX, javaComY, null);
    }

    private void renderVisitUsText(Graphics2D g2d) {
        g2d.drawImage(imgDropText, dropTextX, dropTextY, null);
    }

    /**
     * Renders the progress trough/bar/glow based on the current
     * loading progress value.
     */
    private void renderProgress(Graphics2D g2d) {
        // render trough
        g2d.setColor(clrProgBg);
        g2d.fillRect(progressX, progressY, progressWidth, progressHeight);

        int progW = (int)(progressWidth * zippyProgress);
        if (isOnJDK6) {
            // render glow around progress bar
            AffineTransform oldxform = g2d.getTransform();
            int radius = progGlowRadius;
            int x = progressX;
            int y = progressY + 1 - radius;
            int w = progW;
            int h = radius * 2;
            g2d.setPaint(gradProgGlowR);
            g2d.fillRect(x+1-radius, y, radius, h);
            g2d.setPaint(gradProgGlowL);
            g2d.fillRect(x+1, y, w-2, h);
            g2d.translate(w-3, 0);
            g2d.setPaint(gradProgGlowR);
            g2d.fillRect(x+2, y, radius, h);
            g2d.setTransform(oldxform);
        }
        
        // render progress bar
        g2d.setColor(clrProgBar);
        g2d.fillRect(progressX, progressY, progW, progressHeight);
    }

    /**
     * Renders the thin curve that animates briefly in scene 3 by sliding
     * a gradient downwards along the curve to simulate a "shining" effect.
     */
    private void renderAnimCurve(Graphics2D g2d, int x, int h, float prog) {
        Shape oldclip = g2d.getClip();
        g2d.clipRect(x-50, 0, 100, (int)(h*prog));
        g2d.setPaint(new GradientPaint(0.0f, h*prog*0.8f, clrCurve1,
                                       0.0f, h*prog,      clrCurve2));
        renderCurve(g2d, x);
        g2d.setClip(oldclip);
    }

    /**
     * Renders the thin curve the appears briefly in scene 3.
     */
    private void renderCurve(Graphics2D g2d, int x) {
        g2d.translate(x, 0);
        g2d.draw(thinCurve);
        g2d.translate(-x, 0);
    }

    /**
     * Calculates the current alpha value for the white flash that appears
     * in scene 4 (based on the current state progress value).
     */
    private float getFlashAlpha() {
        return (stateProgress <= 0.5f) ?
                   (stateProgress / 0.5f) * 0.3f :
                   ((1.0f - stateProgress) / 0.5f) * 0.3f;
    }

    /**
     * Calculates the current alpha value for the pulsating glow that
     * surrounds the logo in scene 5 (based on the current pulse progress
     * value).
     */
    private float getPulseAlpha() {
        return (pulseProgress <= 0.5f) ?
                   pulseProgress / 0.5f :
                   (1.0f - pulseProgress) / 0.5f;
    }

    public void update(Graphics g) {
        paint(g);
    }
     
    public void paint(Graphics g) {
        if (DEBUG || !animationThreadRunning) {
	    try {
		doPaint(g);
	    } catch (Exception e) {}
        }
    }
     
    public void startAnimation() {
        synchronized(this) {
            if (animationThreadRunning) {
                return;
            }
            animationThreadRunning = true;
        }
        (new Thread(this)).start();
    }
 
    public void stopAnimation(){
        synchronized(this) {
            animationThreadRunning = false;
        }
    }

    public float getProgressValue() {
        return loadingProgress;
    }
    
    public void setProgressValue(float value) {
        zippyStartTime = 0L;
        zippyStartProgress = zippyProgress;
        
        loadingProgress = value;
    }
    
    public void fadeAway(){
        if (currentState == 4 && !fadeAway) {
            // do this only once to move off the pulse state
            setProgressValue(1.0f);
            fadeAway = true;
        }
    }
    

    /**
     * Converts the given linear fraction value (in the range [0,1]) to
     * a non-linear value using the provided acceleration and deceleration
     * parameters (also specified in the range [0,1]).
     */
    private static float convertToNonLinear(float fraction,
                                            float accel, float decel)
    {
        // See the SMIL 2.0 specification for details on this
        // calculation
        float runRate = 1.0f / (1.0f - accel/2 - decel/2);
        if (fraction < accel) {
            float averageRunRate = runRate * (fraction / accel) / 2;
            fraction *= averageRunRate;
        } else if (fraction > (1.0f - decel)) {
            // time spent in deceleration portion
            float tdec = fraction - (1.0f - decel);
            // proportion of tdec to total deceleration time
            float pdec = tdec / decel;
            fraction =
                runRate * (1.0f - (accel/2) -
                decel + tdec * (2.0f - pdec) / 2);
        } else {
            fraction = runRate * (fraction - (accel/2));
        }
        return fraction;
    }
    
    public void run() {
        while (true) {
            try {
                Thread.sleep(20);
            } catch (Exception e) {}
            

            synchronized(this) {
                if (!animationThreadRunning) {
                    break;
                }
            }


            if (startTime == 0L){
                startTime = System.currentTimeMillis();
            }
            
            long currentTime = System.currentTimeMillis();

            if (DEBUG) {
                if (paused) {
                    startTime = currentTime -
                            (long)(stateProgress * stateStops[currentState]);
                    continue;
                }
            }
            
            if (isShowing()) {
		try {
		    doPaint(getGraphics());
		} catch (RuntimeException e) {
		    Trace.ignoredException(e);
		}
            }

            long elapsed = currentTime - startTime;
            
            // Make sure currentState is valid:
            if ( currentState < stateStops.length ){
                if (elapsed >= stateStops[currentState]) {
                    startTime = currentTime;
                    stateProgress = 0.0f;
                    elapsed = 0L;
                    currentState++;
                } else {
                    stateProgress = ((float)elapsed) / stateStops[currentState];
                }
            }
            if (currentState == 4) {
                pulseProgress =
                    (elapsed % GLOW_CYCLE_TIME) / (float)GLOW_CYCLE_TIME;

                // calculate the "zippy" progress, which is the value used
                // to control the animated progress update from one position
                // to the next
                if (zippyStartTime == 0L) {
                    zippyStartTime = currentTime;
                }
                long zippyElapsed = currentTime - zippyStartTime;
                if (zippyElapsed > ZIPPY_PULSE_TIME) {
                    zippyProgress = loadingProgress;
                } else {
                    float fraction = ((float)zippyElapsed) / ZIPPY_PULSE_TIME;
                    fraction = convertToNonLinear(fraction, 0.5f, 0.1f);
                    zippyProgress =
                        zippyStartProgress +
                        ((loadingProgress - zippyStartProgress) * fraction);
                }

                if (fadeAway && zippyProgress >= 1.0f) {
                    fadeAway = false;
                    loadingProgress = zippyProgress = 1.0f;
                    startTime = currentTime;
                    currentState++;
                }
            } else if (DEBUG &&
                       ((!showText && currentState > 5) || currentState > 8)){
                // reset animation
                loadingProgress = 0.0f;
                zippyProgress = zippyStartProgress = 0.0f;
                zippyStartTime = 0L;
                startTime = currentTime;
                currentState = 1;
            }

        }
    }

    static class Wrapper extends Panel {
        private AnimationPanel anim;

        Wrapper(final AnimationPanel anim) {
            this.anim = anim;
            setLayout(null);
            setBackground(Color.WHITE);
            setSize(anim.getPreferredSize());
            add(anim);
            layoutAnimationPanel();
            addComponentListener(new ComponentAdapter() {
                public void componentResized(ComponentEvent e) {
                    layoutAnimationPanel();
                }
            });
        }
        
        private void layoutAnimationPanel() {
            anim.setBounds(anim.getBoxBounds(getWidth(), getHeight()));
        }
    }
    
    public static void main(String[] args) {
        DEBUG = true;
        final AnimationPanel demo = new AnimationPanel();

        EventQueue.invokeLater(new Runnable() {
            public void run() {
                try {
                    demo.setPreferredSize(new Dimension(400, 300));
                } catch (NoSuchMethodError e) {
                    // 1.4.2 code path
                    demo.setSize(new Dimension(400, 300));
                }
                Frame frame = new Frame("Java Plugin Animation - 2006 Prototype");
                frame.addWindowListener(new WindowAdapter() {
                    public void windowClosing(WindowEvent e) {
                        System.exit(0);
                    }
                });
                //frame.add(demo);
                frame.add(new Wrapper(demo));
                frame.pack();
                frame.setLocationRelativeTo(null);
                frame.setVisible(true);
                demo.preloadResources(400,300);
                demo.startAnimation();
            }
        });

        //demo.setProgressValue(0.8f);
        //try { Thread.sleep(12000); } catch (Exception e) {}
        
        while (true) {
            try { Thread.sleep(1200); } catch (Exception e) {}
            float prog = demo.getProgressValue();
            prog += 0.15f;
            if (prog >= 0.9f) {
                demo.fadeAway();
                try { Thread.sleep(3000); } catch (Exception e) {}
                prog = 0.0f;
            }
            demo.setProgressValue(prog);
        }
    }
}
