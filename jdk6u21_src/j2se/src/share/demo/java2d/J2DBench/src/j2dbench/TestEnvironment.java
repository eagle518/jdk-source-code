/*
 * @(#)TestEnvironment.java	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench;

import java.awt.Canvas;
import java.awt.Image;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Dimension;
import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Toolkit;
import java.util.Hashtable;

import j2dbench.tests.GraphicsTests;

public class TestEnvironment implements Node.Visitor {
    static Group globaloptroot;
    static Group envroot;

    static Option.Int outputWidth;
    static Option.Int outputHeight;

    static Option.Int runCount;
    static Option.Int repCount;
    static Option.Int testTime;

    public static void init() {
	globaloptroot = new Group("global", "Global Options");
	envroot = new Group(globaloptroot, "env", "Test Environment Options");

	outputWidth =
	    new Option.Int(envroot, "outputwidth",
			   "Width of Output Window or Image",
			   1, Integer.MAX_VALUE, 640);
	outputHeight =
	    new Option.Int(envroot, "outputheight",
			   "Height of Output Window or Image",
			   1, Integer.MAX_VALUE, 480);

	runCount =
	    new Option.Int(envroot, "runcount",
			   "Fixed Number of Test Runs per Benchmark",
			   1, Integer.MAX_VALUE, 5);
	repCount =
	    new Option.Int(envroot, "repcount",
			   "Fixed Number of Reps (0 means calibrate)",
			   0, Integer.MAX_VALUE, 0);
	testTime =
	    new Option.Int(envroot, "testtime",
			   "Target test time to calibrate for",
			   1, Integer.MAX_VALUE, 2500);
    }

    public void visit(Node node) {
	if (node instanceof Test) {
	    ((Test) node).runTest(this);
	}
    }

    public void runAllTests() {
	Group.root.traverse(this);
    }

    Canvas comp;
    Image testImage;
    Image srcImage;
    boolean stopped;
    ResultSet results;
    Hashtable modifiers;
    Timer timer;

    public TestEnvironment() {
	results = new ResultSet();
	modifiers = new Hashtable();
        timer = Timer.getImpl();
    }

    public void startTiming() {
        timer.start();
    }

    public void stopTiming() {
        timer.stop();
    }

    public long getTimeMillis() {
        return timer.getTimeMillis();
    }

    public long getTimeNanos() {
        return timer.getTimeNanos();
    }

    public Canvas getCanvas() {
	if (comp == null) {
	    final int w = getWidth();
	    final int h = getHeight();
	    comp = new Canvas() {
		public Dimension getPreferredSize() {
		    return new Dimension(w, h);
		}
	    };
	}
	return comp;
    }

    public Image getSrcImage() {
	return srcImage;
    }

    public void stop() {
	stopped = true;
    }

    public boolean isStopped() {
	return stopped;
    }
    
    public void setTestImage(Image img) {
	this.testImage = img;
    }

    public void setSrcImage(Image img) {
	this.srcImage = img;
    }

    public void erase() {
	Graphics g = getGraphics();
        if (g != null) {
            g.setColor(Color.white);
            g.fillRect(0, 0, getWidth(), getHeight());
            g.dispose();
        }
    }

    public Graphics getGraphics() {
        if (testImage != null) {
            return testImage.getGraphics();
        }
        if (comp != null) {
            return comp.getGraphics();
        }
        return null;
    }

    public int getWidth() {
	return outputWidth.getIntValue();
    }

    public int getHeight() {
	return outputHeight.getIntValue();
    }

    public int getRunCount() {
	return runCount.getIntValue();
    }

    public int getRepCount() {
	return repCount.getIntValue();
    }

    public long getTestTime() {
	return testTime.getIntValue();
    }

    public void sync() {
	if (comp == null) {
	    Toolkit.getDefaultToolkit().sync();
	} else {
	    comp.getToolkit().sync();
	}
    }

    public boolean idle() {
	if (!stopped) {
	    sync();
	    System.gc();
	    System.runFinalization();
	    System.gc();
	    sync();
	    try {
		Thread.sleep(50);
	    } catch (InterruptedException e) {
		stop();
	    }
	}
	return stopped;
    }

    public void setModifier(Modifier o, Object v) {
	modifiers.put(o, v);
    }

    public Object getModifier(Modifier o) {
	return modifiers.get(o);
    }

    public boolean isEnabled(Modifier o) {
	return ((Boolean) modifiers.get(o)).booleanValue();
    }

    public int getIntValue(Modifier o) {
	return ((Integer) modifiers.get(o)).intValue();
    }

    public void removeModifier(Modifier o) {
	modifiers.remove(o);
    }

    public Hashtable getModifiers() {
	return (Hashtable) modifiers.clone();
    }

    public void record(Result result) {
	results.record(result);
    }

    public void flushToScreen() {
	if (testImage != null && comp != null) {
	    Graphics g = comp.getGraphics();
	    if (GraphicsTests.hasGraphics2D) {
		((Graphics2D) g).setComposite(AlphaComposite.Src);
	    }
	    g.drawImage(testImage, 0, 0, null);
	    g.dispose();
	}
    }

    public void summarize() {
	results.summarize();
    }

    private abstract static class Timer {
        public static Timer getImpl() {
            try {
                System.nanoTime();
                return new Nanos();
            } catch (NoSuchMethodError e) {
                return new Millis();
            }
        }

        public abstract void start();
        public abstract void stop();
        public abstract long getTimeMillis();
        public abstract long getTimeNanos();

        private static class Millis extends Timer {
            private long millis;

            public void start() {
                millis = System.currentTimeMillis();
            }

            public void stop() {
                millis = System.currentTimeMillis() - millis;
            }

            public long getTimeMillis() {
                return millis;
            }

            public long getTimeNanos() {
                return millis * 1000 * 1000;
            }
        }

        private static class Nanos extends Timer {
            private long nanos;

            public void start() {
                nanos = System.nanoTime();
            }

            public void stop() {
                nanos = System.nanoTime() - nanos;
            }

            public long getTimeMillis() {
                return (nanos + (500 * 1000)) / (1000 * 1000);
            }

            public long getTimeNanos() {
                return nanos;
            }
        }
    }
}
