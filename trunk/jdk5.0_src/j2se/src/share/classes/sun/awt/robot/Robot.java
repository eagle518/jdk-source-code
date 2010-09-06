/*
 * @(#)Robot.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.robot;

import java.awt.*;
import java.awt.image.*;
import java.util.Vector;
import java.util.Properties;
import java.lang.reflect.InvocationTargetException;

/**
 * Class used to generate native events for test automation.
 *
 * @version 	@(#)Robot.java	1.8 03/12/19
 * @author 	Robi Khan
 */
public final class Robot {
    // can't imagine why you'd want to wait any longer...
    private static final int MAX_DELAY = 60000;

    private RobotPeer	peer;
    private Toolkit	toolkit;
    private boolean	isWaitForIdle = false;
    private int		autoDelay = 0;

    public Robot(RobotPeer robotPeer, Toolkit robotToolkit) {
	peer = robotPeer;
	toolkit = robotToolkit;
    }

    /**
     * Allows another class to access the peer in case it wants to
     * post directly.
     */
    public RobotPeer getPeer() { return peer; }

    /**
     * Moves mouse pointer to given screen coordinates
     * @param x		X position
     * @param y		Y position
     */
    public void mouseMove(int x, int y) {
	peer.mouseMove(x,y);
	afterEvent();
    }

    /**
     * Presses one or more mouse buttons (does not override
     * any previous mousePress calls).
     * @param buttons	Button mask (combination of InputEvent.BUTTON_MASK1/2/3)
     */
    public void mousePress( long buttons ) {
	peer.mousePress(buttons);
	afterEvent();
    }
    
    /**
     * Releases one or more mouse buttons (does not override
     * any previous mouseRelease calls).
     * @param buttons	Button mask (combination of InputEvent.BUTTON_MASK1/2/3)
     */
    public void mouseRelease( long buttons ) {
	peer.mouseRelease(buttons);
	afterEvent();
    }

    /**
     * Presses a given key
     * @param	keyCode	Key to press (e.g. KeyEvent.VK_SHIFT)
     */
    public void keyPress( int keycode ) {
	peer.keyPress(keycode);
	afterEvent();
    }
    
    /**
     * Releases a given key
     * @param	keyCode	Key to release (e.g. KeyEvent.VK_SHIFT)
     */
    public void keyRelease( int keycode ) {
	peer.keyRelease(keycode);
	afterEvent();
    }

    /**
     * Returns the color of a pixel at the given screen coordinates
     * @param	x	X position of pixel
     * @param	y	Y position of pixel
     * @return  Color	Color of the pixel
     */
    public Color getPixelColor(int x, int y) {
	Color color = new Color(peer.getRGBPixel(x,y));
	return color;
    }

    /**
     * Creates a screen capture
     * @param	screenRect	Rect to capture in screen coordinates
     * @return	The captured image
     */
    public ScreenCapture createScreenCapture(Rectangle screenRect) {
	int pixels[] = peer.getRGBPixels(screenRect);
	ImageProducer producer = new ScreenCaptureProducer(pixels, screenRect);
	Image image = toolkit.createImage(producer);
	return new ScreenCapture( toolkit.createImage(producer) );
    }

    /*
     * Called after an event is generated
     */
    private void afterEvent() {
	autoWaitForIdle();
	autoDelay();
    }

    /**
     * Determines whether waitForIdle is automatically
     * called after an event is generated
     * @return
     */
    public boolean isWaitForIdle() {
	return isWaitForIdle;
    }

    /**
     * Sets whether waitForIdle is automatically called
     * after an event has been generated.
     * @param	isOn	Is waitForIdle automatically called?
     */
    public void setWaitForIdle(boolean isOn) {
	isWaitForIdle = isOn;
    }
    
    /*
     * Calls waitForIdle after every event if asked to
     */
    private void autoWaitForIdle() {
	if (isWaitForIdle) {
	    waitForIdle();
	}
    }

    /**
     * Returns the delay (in ms) after an event is generated
     */
    public int getAutoDelay() {
	return autoDelay;
    }

    /**
     * Sets a delay (in ms) after an event is generated
     */
    public void setAutoDelay(int ms) {
	autoDelay = ms;
    }

    /*
     * Automatically sleeps for the specified interval after event generated
     */
    private void autoDelay() {
	delay(autoDelay);
    }

    /**
     * Sleeps for the specified time 
     * @param	ms	Time to sleep in milliseconds
     */
    public void delay(int ms) {
	if (ms < 0 || ms > MAX_DELAY) {
	    throw new IllegalArgumentException("Delay must be to 0 to 60,000ms");
	}
	if (ms > 0) {
	    try {
	        Thread.sleep(ms);  // was autoDelay, which seems to be wrong
	    } catch(InterruptedException ite) {
		ite.printStackTrace();
	    }
	}
    }

    /*
     * Dummy Runnable implementation
     */
    class QueueSyncer implements Runnable {
	public void run() {
	    // don't have to do anything...
	}
    }

    /**
     * Waits for currently pending events to be
     * processed. MUST BE called from a thread
     * other than the EventDispatchThread!
     */
    public void waitForIdle() {
	// post a dummy event to the queue so we know when
	// all the events before it have been processed
	try {
	    EventQueue.invokeAndWait( new QueueSyncer() );
	} catch(InterruptedException ite) {
	    System.err.println("Non-fatal exception:");
	    ite.printStackTrace();
	} catch(InvocationTargetException ine) {
	    System.err.println("Non-fatal exception:");
	    ine.printStackTrace();
	}
    }

}
