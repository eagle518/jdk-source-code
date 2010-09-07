/*
 * @(#)AnimationPanel2.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Panel;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.geom.AffineTransform;
import com.sun.deploy.util.Trace;

/** Based on user feedback, this is a version of the animation panel
    which is less visually jarring than the orange logo animation. */

public class AnimationPanel2 extends AnimationPanelBase {
    private static boolean DEBUG;

    /** Number of milliseconds in one animation cycle */
    private static final int ANIMATION_CYCLE_TIME = 2000;
    /** Number of milliseconds over which to animate a progress bar update */
    private static final int ZIPPY_PULSE_TIME = 300;

    private static final long[] stateStops = new long[] {
        0, 300, 99999999, 750, 950, 300, 1000,
    };

    private static final String JAVA_LOGO_IMAGE = "JavaCupLogo-161.png";
    private static final String JAVA_COM_IMAGE  = "javacom300.png";

    private Color bgColor, fgColor = Color.BLACK;
    private Image javaLogoImage, javaComImage;
    private boolean preloadedAll = false;
    private boolean errorDuringPreloading = false;
    private Image backbuffer=null;
    private float loadingProgress;
    private float spinnerProgress;
    private static final int NUM_SPINNER_STOPS = 16;
    private static final int SPINNER_R = 84;
    private static final int SPINNER_G = 130;
    private static final int SPINNER_B = 161;
    private long startTime;
    private long initialStartTime;
    private int currentState;
    private float stateProgress;
    private boolean showLogoAndText;
    private float zippyProgress, zippyStartProgress;
    private long zippyStartTime;
    private boolean fadeAway;
    private boolean paused; // REMIND: debug only
    private boolean animationThreadRunning = false;

    public AnimationPanel2() {
        currentState = 1;

        if (DEBUG) {
            addMouseListener(new MouseAdapter() {
                public void mousePressed(MouseEvent e) {
                    paused = !paused;
                }
            });
        }

        UIUtil.disableBackgroundErase(this);
        setBoxBGColor(Color.WHITE);
    }
    
    public void setBoxBGColor(Color bgColor) {
        setBackground(bgColor);
        this.bgColor = bgColor;
    }

    public void setBoxFGColor(Color fgColor) {
        this.fgColor = fgColor;
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

    public void stopAnimation() {
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
        if (currentState == 2 && !fadeAway) {
            // do this only once to move off the animating state
            setProgressValue(1.0f);
            fadeAway = true;
        }
    }

    private Image loadImage(Toolkit tk, String filename) {
        return tk.createImage(getClass().getResource(filename));
    }

    private void preloadResources() {
        if (preloadedAll || errorDuringPreloading) {
            return;
        }

        Toolkit tk = Toolkit.getDefaultToolkit();
        MediaTracker mt = new MediaTracker(this);
        javaLogoImage = loadImage(tk, JAVA_LOGO_IMAGE);
        mt.addImage(javaLogoImage, 0);
        javaComImage = loadImage(tk, JAVA_COM_IMAGE);
        mt.addImage(javaComImage, 1);
        try {
            mt.waitForAll();
        } catch (InterruptedException e) {
            errorDuringPreloading = true;
            return;
        }
        preloadedAll = true;
    }

    private void allocateBackBuffer(int width, int height) {
        if (backbuffer != null &&
            (backbuffer.getWidth(null) != width ||
             backbuffer.getHeight(null) != height)) {
            backbuffer.flush();
            backbuffer = null;
        }

        if (backbuffer == null) {
            backbuffer = createImage(width, height);
        }
    }


    private Dimension getImageBoundsWithinSize(int imageWidth,
                                               int imageHeight,
                                               int containerSize,
                                               float scaleFactor) {
        // Compute scale factor allowing diagonal of image to fit
        // within fraction of container size
        float sf = scaleFactor * containerSize /
            (float) Math.sqrt(imageWidth * imageWidth +
                              imageHeight * imageHeight);
        return new Dimension((int) (imageWidth * sf),
                             (int) (imageHeight * sf));
    }

    private static float bias(float val, float bias) {
        return bias + (val * (1.0f - bias));
    }

    private void renderSpinner(Graphics2D g2d, int w, int h, int size) {
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                             RenderingHints.VALUE_ANTIALIAS_ON);
        int spinnerInnerSize = (int) Math.max(1, 0.04f * size);
        int spinnerOuterSize = (int) Math.max(1, 0.05f * size);
        int spinnerRadius = (int) Math.max(1, 0.35f * size);
        int centerX = w / 2;
        int centerY = h / 2;
        AffineTransform oldxform = g2d.getTransform();
        g2d.translate(centerX, centerY - 0.05f * size);
        if (showLogoAndText) {
            Dimension imageSize = getImageBoundsWithinSize(javaLogoImage.getWidth(null),
                                                           javaLogoImage.getHeight(null),
                                                           size,
                                                           0.6f);
            g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
            g2d.drawImage(javaLogoImage, -imageSize.width / 2, -imageSize.height / 2,
                          imageSize.width, imageSize.height, null);
        }
        double incr = 2 * Math.PI / NUM_SPINNER_STOPS;
        int vertScale = (showLogoAndText ? 1 : 5);
        int[] xPoints = { -spinnerOuterSize,  spinnerOuterSize, spinnerInnerSize, -spinnerInnerSize };
        int[] yPoints = { -spinnerOuterSize, -spinnerOuterSize, vertScale * spinnerInnerSize, vertScale * spinnerInnerSize };
        g2d.rotate(-incr / 2);
        g2d.translate(0, -spinnerRadius);
        float bias = 0.1f;
        float gradientOffset = spinnerProgress;
        float gradientIncr = 1.0f / NUM_SPINNER_STOPS;

        for (int i = 0; i < NUM_SPINNER_STOPS; i++) {
            g2d.translate(0, spinnerRadius);
            g2d.rotate(-incr);
            g2d.translate(0, -spinnerRadius);
            // For each polygon, figure out the linear gradient
            float lowAlpha = (i * gradientIncr) + gradientOffset;
            float highAlpha = lowAlpha + gradientIncr;
            boolean boundary = false;
            while (lowAlpha > 1.0f) {
                lowAlpha -= 1.0f;
            }
            while (highAlpha > 1.0f) {
                highAlpha -= 1.0f;
            }

            if (lowAlpha > highAlpha) {
                boundary = true;
            }

            if (!boundary) {
                GradientPaint paint =
                    new GradientPaint(-spinnerOuterSize, 0,
                                      new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                (int) (255.0f * bias(1.0f - lowAlpha, bias))),
                                      spinnerOuterSize, 0,
                                      new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                (int) (255.0f * bias(1.0f - highAlpha, bias))));
                g2d.setPaint(paint);
                g2d.fillPolygon(xPoints, yPoints, xPoints.length);
            } else {
                // generate two polygons with different gradients and fill them
                float frac = highAlpha / gradientIncr;
                int outerSplit = (int) (frac * 2 * spinnerOuterSize);
                int innerSplit = (int) (frac * 2 * spinnerInnerSize);
                
                // High part
                GradientPaint paint =
                    new GradientPaint(-spinnerOuterSize + outerSplit, 0,
                                      new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                (int) (255.0f * bias(0.0f, bias))),
                                      spinnerOuterSize, 0,
                                      new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                (int) (255.0f * bias(highAlpha, bias))));
                g2d.setPaint(paint);
                g2d.fillPolygon(new int[] { -spinnerOuterSize + outerSplit,
                                            spinnerOuterSize,
                                            spinnerInnerSize,
                                            -spinnerInnerSize + innerSplit },
                                yPoints, yPoints.length);
                // Low part
                paint = new GradientPaint(-spinnerOuterSize, 0,
                                          new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                    (int) (255.0f * bias(lowAlpha, bias))),
                                          -spinnerOuterSize + outerSplit, 0,
                                          new Color(SPINNER_R, SPINNER_G, SPINNER_B,
                                                    (int) (255.0f * bias(1.0f, bias))));
                g2d.setPaint(paint);
                g2d.fillPolygon(new int[] { -spinnerOuterSize,
                                            -spinnerOuterSize + outerSplit,
                                            - spinnerInnerSize + innerSplit,
                                            -spinnerInnerSize },
                                yPoints, yPoints.length);

            }
        }
        g2d.setTransform(oldxform);
    }

    private void renderProgress(Graphics2D g2d, int w, int h, int size) {
        int progressWidth = (int) (size * 0.7f);
        int progressHeight = (int) Math.max(4, size * 0.05f);
        progressHeight = Math.min(8, progressHeight);
        int progressX = (w - progressWidth) / 2;
        int progressY = (int) ((h + 0.9f * size) / 2) - progressHeight;
        g2d.setColor(fgColor);
        g2d.drawRect(progressX, progressY, progressWidth, progressHeight);
        int progW = (int) (progressWidth * zippyProgress);
        g2d.fillRect(progressX + 2, progressY + 2,
                     progW - 3, progressHeight - 3);
    }

    private void renderJavaCom(Graphics2D g2d, int w, int h, int size) {
        if (showLogoAndText) {
            int centerX = w / 2;
            int centerY = h / 2;
            AffineTransform oldxform = g2d.getTransform();
            g2d.translate(centerX, centerY - 0.05f * size);
            Dimension imageSize = getImageBoundsWithinSize(javaComImage.getWidth(null),
                                                           javaComImage.getHeight(null),
                                                           size,
                                                           0.6f);
            g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
            g2d.drawImage(javaComImage, -imageSize.width / 2, -imageSize.height / 2,
                          imageSize.width, imageSize.height, null);
        }
    }

    /**
     * 1. Fade in spinner
     */
    private void renderState1(Graphics2D g2d, int w, int h, int size) {
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, stateProgress));
        renderSpinner(g2d, w, h, size);
        renderProgress(g2d, w, h, size);
    }

    /**
     * 2a. Animate spinner
     * 2b. Animate progress bar
     */
    private void renderState2(Graphics2D g2d, int w, int h, int size) {
        renderSpinner(g2d, w, h, size);
        renderProgress(g2d, w, h, size);
    }

    /**
     * 3. Cross-fade to "java.com" text
     */
    private void renderState3(Graphics2D g2d, int w, int h, int size) {
        Composite oldcomp = g2d.getComposite();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f - stateProgress));
        renderSpinner(g2d, w, h, size);
        renderProgress(g2d, w, h, size);
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, stateProgress));
        renderJavaCom(g2d, w, h, size);
        g2d.setComposite(oldcomp);
    }

    /**
     * 4. Render "java.com" text
     */
    private void renderState4(Graphics2D g2d, int w, int h, int size) {
        if (showLogoAndText) {
            renderJavaCom(g2d, w, h, size);
        } else {
            ++currentState;
        }
    }
    
    /**
     * 5. Fade out "java.com" text
     */
    private void renderState5(Graphics2D g2d, int w, int h, int size) {
        if (showLogoAndText) {
            Composite oldcomp = g2d.getComposite();
            g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f - stateProgress));
            renderJavaCom(g2d, w, h, size);
            g2d.setComposite(oldcomp);
        } else {
            ++currentState;
        }
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

    public void doPaint(Graphics g) {
        int w = getWidth();
        int h = getHeight();
        
        int size = getBoxSize(w, h);
        if (size <= 0) {
            g.setColor(bgColor);
            g.fillRect(0, 0, w, h);
            // REMIND: we should turn off the timer in this case
            return;
        }

        preloadResources();
        if (!preloadedAll) {
            return; // bail out
        }
        allocateBackBuffer(w, h);

        if (size < 50) {
            showLogoAndText = false;
        } else {
            showLogoAndText = true;
        }

        Graphics2D g2d = (Graphics2D)backbuffer.getGraphics();
        g2d.setColor(bgColor);
        g2d.fillRect(0, 0, w, h);

        switch (currentState) {
            case 1:
                renderState1(g2d, w, h, size);
                break;
            case 2:
                renderState2(g2d, w, h, size);
                break;
            case 3:
                renderState3(g2d, w, h, size);
                break;
            case 4:
                renderState4(g2d, w, h, size);
                break;
            case 5:
                renderState5(g2d, w, h, size);
                break;
            default:
                break;
        }

        g2d.dispose();

        // copy the backbuffer to the screen
        g.drawImage(backbuffer, 0, 0, null);
    }

    /**
     * Returns the size of the orange box given the dimensions of the
     * available destination area.
     */
    private static int getBoxSize(int w, int h) {
        int mindim = (w > h) ? h : w;
        if (mindim < 25) {
            return 0;
        }
        if (mindim < 200) {
            return (int) (0.75f * mindim);
        }
        if (mindim < 300)
            return 150;
        if (mindim > 600)
            return 300;
        return mindim / 2;
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

            if (initialStartTime == 0L) {
                initialStartTime = startTime;
            }

            long totalElapsed = currentTime - initialStartTime;
            
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
            if (currentState < 6) {
                spinnerProgress = (totalElapsed % ANIMATION_CYCLE_TIME) / (float) ANIMATION_CYCLE_TIME;

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
                       (currentState > 6)){
                // reset animation
                loadingProgress = 0.0f;
                zippyProgress = zippyStartProgress = 0.0f;
                zippyStartTime = 0L;
                startTime = currentTime;
                initialStartTime = 0;
                currentState = 1;
            } else {
                // Kill ourselves off
                synchronized(this) {
                    animationThreadRunning = false;
                }
            }
        }
    }

    public static void main(String[] args) {
        DEBUG = true;
        final AnimationPanel2 demo = new AnimationPanel2();
        if (args.length > 0 && args[0].equals("-reverse")) {
            demo.setBoxBGColor(Color.BLACK);
            demo.setBoxFGColor(Color.WHITE);
        }

        EventQueue.invokeLater(new Runnable() {
                public void run() {
                    try {
                        demo.setPreferredSize(new Dimension(400, 300));
                    } catch (NoSuchMethodError e) {
                        // 1.4.2 code path
                        demo.setSize(new Dimension(400, 300));
                    }
                    Frame frame = new Frame("Java Plugin Animation - 2008 Prototype");
                    frame.addWindowListener(new WindowAdapter() {
                            public void windowClosing(WindowEvent e) {
                                System.exit(0);
                            }
                        });
                    frame.add(demo);
                    frame.pack();
                    frame.setLocationRelativeTo(null);
                    frame.setVisible(true);
                    demo.startAnimation();
                }
            });

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
