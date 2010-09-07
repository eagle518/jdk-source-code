/*
 * @(#)DragHelper.java	1.19 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.applet.Applet;
import java.awt.AWTEvent;
import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.PointerInfo;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.image.*;
import java.awt.event.*;
import java.awt.MediaTracker;
import java.io.*;
import java.lang.reflect.Method;
import java.net.URL;
import java.security.*;
import java.util.*;
import javax.imageio.*;
import javax.swing.JFrame;
import com.sun.deploy.util.DeployAWTUtil;
import sun.plugin2.util.*;

/** Helps with the drag-and-drop operation of taking applets out of
    the web browser. This class is a singleton. */

public class DragHelper {
    private static final boolean DEBUG = SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null;
    private static final boolean isOSX = System.getProperty("os.name").startsWith("Mac OS X");

    /** Gets the sole instance of the DragHelper. */
    public static DragHelper getInstance() {
        return soleInstance;
    }

    public synchronized void register(Plugin2Manager manager,
                                      DragListener listener) {
        if (!initialize()) {
            return;
        }
        perAppletInfoList.add(new PerAppletInfo(manager, listener));
    }

    public void makeDisconnected(Plugin2Manager manager,
				 Frame containingFrame) {
	for (Iterator iter = perAppletInfoList.iterator(); iter.hasNext(); ) {
	    PerAppletInfo info = (PerAppletInfo) iter.next();
	    if (info.getManager() == manager) {
		info.makeDisconnected(containingFrame);
		return;
	    }
	}
	// Silently ignore failures to make life easier on callers
    }

    public synchronized void restore(Plugin2Manager manager) {
        for (Iterator iter = perAppletInfoList.iterator(); iter.hasNext(); ) {
            PerAppletInfo info = (PerAppletInfo) iter.next();
            if (info.getManager() == manager) {
                info.restore();
                return;
            }
        }
        // Silently ignore failures to make life easier on callers
    }

    public synchronized void unregister(Plugin2Manager manager) {
        for (Iterator iter = perAppletInfoList.iterator(); iter.hasNext(); ) {
            PerAppletInfo info = (PerAppletInfo) iter.next();
            if (info.getManager() == manager) {
                // Robustness
                if (info.iAmDragging()) {
                    setSomeoneDragging(false);
                }
                iter.remove();
                return;
            }
        }
        // Silently ignore failures to make life easier on callers
    }
        
    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private static DragHelper soleInstance = new DragHelper();

    private boolean initialized;
    private Method  isSystemGeneratedMethod;

    // List of information associated with the currently active
    // managers that support drag-and-drop to the desktop
    private List/*<PerAppletInfo>*/ perAppletInfoList = new ArrayList();

    // Indicates whether we're in a drag state or not -- it would be
    // good to get rid of this, but we want it for quick rejection of
    // incoming AWT events that aren't intended for us
    private volatile boolean dragging;

    private DragHelper() {
    }

    // Assumes this is called from a synchronized method
    private boolean initialize() {

        if (!initialized) {
            initialized = true;

            // We install an AWTEventListener so that we can see all
            // mouse-related events. This allows us to implement the
            // drag-and-drop operation for any component hierarchy rooted
            // in one of the applets that we own, without having to add
            // listeners all over the place.
            AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        // We require access to SunToolkit.isSystemGenerated(AWTEvent e)
                        try {
                            Class c = sun.awt.SunToolkit.class;
                            isSystemGeneratedMethod = c.getMethod("isSystemGenerated",
                                                                  new Class[] { AWTEvent.class });
                            isSystemGeneratedMethod.setAccessible(true);
                        } catch (Exception e) {
                            if (!isOSX) {
                                e.printStackTrace();
                            }
                        }

                        Toolkit.getDefaultToolkit().addAWTEventListener(new AWTEventListener() {
                                public void eventDispatched(AWTEvent event) {
                                    dispatchEvent(event);
                                }
                            }, AWTEvent.MOUSE_EVENT_MASK | AWTEvent.MOUSE_MOTION_EVENT_MASK);
                        return null;
                    }
                });
        }
        return (isSystemGeneratedMethod != null) || isOSX;
    }

    private void dispatchEvent(AWTEvent event) {
        if (event instanceof InputEvent) {
            InputEvent ev = (InputEvent) event;
            // We only want to look at system events, not ones created by
            // user code
            if (isSystemGenerated(event) &&
                (dragging || couldBeDragStartEvent(ev))) {
                synchronized(this) {
                    // See whether any of our managers want this event
                    for (Iterator iter = perAppletInfoList.iterator(); iter.hasNext(); ) {
                        PerAppletInfo info = (PerAppletInfo) iter.next();
                        if (info.dispatchEvent(ev)) {
                            // Consumed
                            return;
                        }
                    }
                }
            }
        }
    }

    private boolean isSystemGenerated(AWTEvent event) {
        if (isSystemGeneratedMethod == null) {
            if (!isOSX) {
                return false;  // Robustness
            } else {
                return (!((InputEvent) event).isConsumed());
            }
        }
        try {
            return ((Boolean) isSystemGeneratedMethod.invoke(null, new Object[] { event })).booleanValue();
        } catch (Exception e) {
            return false;
        }
    }

    private boolean isSomeoneDragging() {
        return dragging;
    }

    private void setSomeoneDragging(boolean isDragging) {
        dragging = isDragging;
    }

    /** Where all the heavy lifting is done */
    private class PerAppletInfo {
        private Plugin2Manager manager;

        // If the applet has been dragged out of the browser, we have
        // a top-level frame for it; note that we use a JFrame rather
        // than a Frame for JApplets for better behavior with the
        // shaped and translucent window support
        private Frame frame;

        // If the drag operation is in progress, we'll have an offset
        // relative to the upper left corner of the component where
        // the drag started from
        private Point dragOffset;

        // The location (upper-left corner) of the frame on the screen
        private Point upperLeft;

        // We only need to support one listener; this is how the
        // PluginMain can know when the applet is really disconnected
        // from the browser
        private DragListener listener;

        // We only send notifications out of this class (both to the
        // DragListener as well as to the applet) the first time it is
        // dragged out of the browser; after that, we take care of
        // moving it around the desktop
        private boolean notificationsSent;

        // If we have dropped this applet to the desktop, and the user
        // hasn't registered a method like
        //     public void setAppletCloseListener(ActionListener e);
        // then we create a close button for it which is another
        // undecorated Frame
        private Frame closeButtonFrame;

        // The offset, from the upper right of the frame containing
        // the applet, of the close button
        private final Point closeButtonOffset = new Point(CLOSE_BUTTON_OFFSET,
                                                          -CLOSE_BUTTON_OFFSET);

        // This is to allow close operations on the Frame containing
        // the applet to initiate teardown without introducing cycles
        // in the shutdown process
        private ActionListener closeListener;
        private boolean closing;

        private static final int CLOSE_BUTTON_SIZE = 10;
        private static final int CLOSE_BUTTON_OFFSET = 5;

        PerAppletInfo(Plugin2Manager manager,
                      DragListener listener) {
            this.manager = manager;
            this.listener = listener;
        }

        public Plugin2Manager getManager() {
            return manager;
        }

        public boolean dispatchEvent(AWTEvent event) {
            MouseEvent e = (MouseEvent) event;

            // Quickly reject incoming events for performance reasons
            if (isSomeoneDragging() && !iAmDragging()) {
                return false;
            }

            // See whether this is an event that we care about. It
            // will be if the associated component is owned by the applet.
            Component c = e.getComponent();
            Applet a = manager.getApplet();
            if (a == null) {
                // Should only happen during termination or similar
                return false;
            }

	    // Check with Plugin2Manager if there's a modal dialog being 
	    // popped up from the applet and don't dispatch the event if there's one.
	    if (manager.getModalityLevel() != 0) {
		return false;
	    }

            // Take all mouse events if we're dragging this particular applet
            if (iAmDragging()) {
                dispatchEventImpl(a, e);
                return true;
            }

            boolean mightBeDragStart = false;
            while (c != null) {
                if (c == a) {
                    mightBeDragStart = true;
                    break;
                }
                c = c.getParent();
            }
            if (!mightBeDragStart) {
                // Event is not owned by this applet
                return false;
            }

            // Consult the applet to see if it wants this to be a drag
            // start event
            if (isDragStartEvent(a, e)) {
                dispatchEventImpl(a, e);
                return true;
            }

            return false;
        }

        public void restore() {
            // Called to restore the applet to its original location in the web page
            Container originalContainer = manager.getAppletParentContainer();
            Applet applet = manager.getApplet();
            if (applet != null && originalContainer != null) {
                // Clear out the Java logo we put in place (or
                // anything the user put in there)
                if (originalContainer instanceof JFrame) {
                    ((JFrame) originalContainer).getContentPane().removeAll();
                } else {
                    originalContainer.removeAll();
                }
                // Put the applet back
                originalContainer.add(applet);
                // Notify the applet that it is back in place
                sendAppletRestored(applet);
                // Allow the sending of notifications again
                notificationsSent = false;
                // Allow it to close itself again
                closing = false;
                // Force a repaint of the applet in its new location
                applet.repaint();
            }
        }

        private boolean iAmDragging() {
            return (dragOffset != null);
        }

	private void setFrameTitle(Frame frame) {
            // Set its title -- might like to have this frame
            // not show up at all as an independent entry in
            // the list of top-level windows
            String name = manager.getName();
            // Trim the package name if any
            int lastDot = name.lastIndexOf(".");
            if (lastDot > 0) {
                name = name.substring(lastDot + 1);
            }
	    frame.setTitle(name);
	}

	private void setupCloseListener(Applet applet) {
	    closeListener = new CloseListener();
	    if (!sendSetAppletCloseListener(applet, closeListener)) {
		// The user didn't have their own
		// setAppletCloseListener(ActionListener l)
		// method; use the default behavior of
		// creating a small floating close button
		closeButtonFrame = createCloseButton(closeListener);
		// Move the close button into place and make
		// it visible
                refreshCloseButtonFrame();
		closeButtonFrame.setVisible(true);
	    }
	}

        private void setupWindow(Frame frame) {
            frame.addWindowListener(new WindowAdapter() {
                    private long lastActivate;

                    public void windowActivated(WindowEvent e) {
                        // Hack to avoid pingponging focus back and forth
                        long curActivate = System.currentTimeMillis();
                        if (curActivate - lastActivate > 200) {
                            // Bring the close button to the front too
                            if (closeButtonFrame != null) {
                                closeButtonFrame.toFront();
                            }
                            lastActivate = curActivate;
                        }
                    }

                    public void windowClosing(WindowEvent e) {
                        if (closeListener != null) {
                            closeListener.actionPerformed(null);
                        }
                    }

                    public void windowIconified(WindowEvent e) {
                        if (closeButtonFrame != null) {
                            closeButtonFrame.setVisible(false);
                        }
                    }

                    public void windowDeiconified(WindowEvent e) {
                        if (closeButtonFrame != null) {
                            closeButtonFrame.setVisible(true);
                            refreshCloseButtonFrame();
                        }
                    }
                });

            // If this window is maximized, try to prevent it from
            // covering the entire desktop
            GraphicsEnvironment env = GraphicsEnvironment.getLocalGraphicsEnvironment();
            if (env != null) {
                GraphicsDevice dev = env.getDefaultScreenDevice();
                if (dev != null) {
                    DisplayMode mode = dev.getDisplayMode();
                    if (mode != null) {
                        int maxSizeDelta = 4 * CLOSE_BUTTON_OFFSET + CLOSE_BUTTON_SIZE;
                        frame.setMaximizedBounds(new Rectangle(0, 0,
                                                               mode.getWidth() - maxSizeDelta,
                                                               mode.getHeight() - maxSizeDelta));
                    }
                }
            }
        }

	private void makeDisconnected(Frame frame) {
	    this.frame = frame;
            setupWindow(frame);
	    setFrameTitle(frame);
	    upperLeft = frame.getLocation();
	    Applet applet = manager.getApplet();
	    sendDragStarted(applet);
	    sendDragFinished(applet);
            notificationsSent = true;
	    setupCloseListener(applet);
	}

        private void refreshCloseButtonFrame() {
            // Refreshes the position of the close button's frame
            if (closeButtonFrame != null) {
                Point framePos = frame.getLocation();
                closeButtonFrame.setLocation(framePos.x + frame.getWidth() + closeButtonOffset.x,
                                             framePos.y + closeButtonOffset.y);
            }
        }

        // The logic for actually doing the drag operation
        private void dispatchEventImpl(final Applet applet, MouseEvent e) {
            if (!iAmDragging()) {
                // We'll only get here if the incoming event is the
                // drag start event.
                setSomeoneDragging(true);

                // Compute the drag offset, which we need to make the
                // applet appear to stick properly to the mouse as we
                // drag it
                upperLeft = applet.getLocationOnScreen();

                // FIXME: make this work on earlier JDKs than 5.0
                // Could infer this from the MouseEvent by walking up
                // the hierarchy with the appropriate math
                Point curPos = getCurrentMouseLocation();
                dragOffset = new Point(curPos.x - upperLeft.x, curPos.y - upperLeft.y);

                // Figure out the size of the applet
                // (FIXME: should see whether we still need
                // getDragSize(), or can just use the size of the
                // applet at this point)
                final Dimension size = getDragSize(applet);
                if (frame == null) {
                    // Create the top-level frame
                    // Note: we have to use
                    // AccessController.doPrivileged() to avoid having
                    // the applet warning banner show up in some cases
                    frame = (Frame) AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            // Avoid touching JApplet directly to avoid loading
                            // Swing classes unnecessarily
                            if (isSubclass(applet.getClass(), "javax.swing.JApplet")) {
                                return new JFrame();
                            } else {
                                return new Frame();
                            }
                        }
                    });
                    // Always use an undecorated frame
                    frame.setUndecorated(true);
                    setupWindow(frame);
                    setFrameTitle(frame);
                    
                    final Container currentParent = manager.getAppletParentContainer();
                    // It should only be null in the case of race conditions
                    if (currentParent != null) {
                        // Shuffle things around in the component hierarchy
                        currentParent.remove(applet);
                        // Create the canvas that will display the Java logo image later. It is not critical
                        DeployAWTUtil.invokeLater(currentParent, new Runnable() {
                            public void run() {
                                // Create the canvas that will display the Java logo image
                                Canvas c = getJavaLogoCanvas(size);
                                currentParent.add(c);
                            }
                        });
                    }

                    frame.add(applet, BorderLayout.CENTER);
                    applet.setLocation(0, 0);
                    frame.setSize(size);
                    frame.setResizable(false);
                }

                if (!notificationsSent) {
                    // Tell the applet it's been moved around. The current
                    // implicit semantic is that this is done after the
                    // applet is added to the new frame but before it has
                    // been shown. FIXME: need to decide whether the final
                    // Frame for the applet should have a window border
                    // based on AWTUtilities.isWindowOpaque() after the
                    // call out to the applet -- or always add a manually-
                    // drawn close button.)
                    sendDragStarted(applet);
                }
                // Make the location of the frame line up with the
                // applet's current location
                frame.setLocation(upperLeft);
                // Make the frame visible after notifying the applet
                frame.setVisible(true);

                if (isOSX && !notificationsSent) {
                    // On Mac OS X, the window server will swallow all
                    // subsequent mouse events when we pop up the
                    // applet's new window over the region in the web
                    // browser which received the mouse down event. To
                    // work around this we use the AWT Robot to
                    // synthesize a mouse up and mouse down event pair
                    // to cause the drag to re-start over the applet's
                    // region. Note that this also causes the
                    // appletDragFinished notification to be sent
                    // earlier than on other platforms.
                    AccessController.doPrivileged(new PrivilegedAction() {
                            public Object run() {
                                try {
                                    java.awt.Robot r = new java.awt.Robot();
                                    r.mouseRelease(InputEvent.BUTTON1_MASK);
                                    r.mousePress(InputEvent.BUTTON1_MASK);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                                return null;
                            }
                        });
                }
            } else {
                // We have to see whether this is in fact something we
                // care about. While we're dragging the applet out of
                // the browser, we want to pay attention to mouse
                // dragged and release events.
                int id = e.getID();
                if (id == MouseEvent.MOUSE_DRAGGED ||
                    id == MouseEvent.MOUSE_EXITED) {
                    // Move the frame to the new location
                    if (frame != null && dragOffset != null) {
                        // Figure out where we should move the window
                        // to in response to the mouse movement
                        try {
                            // Note that we could use e.getXOnScreen()
                            // / e.getYOnScreen(), but MouseInfo is
                            // even more portable
                            Point curPos = getCurrentMouseLocation();
                            upperLeft.x = curPos.x - dragOffset.x;
                            upperLeft.y = curPos.y - dragOffset.y;
                        } catch (Throwable t) {
                            // This incremental approach (for running
                            // on earlier JDKs) doesn't work as well
                            if (id != MouseEvent.MOUSE_EXITED) {
                                int deltaX = e.getX() - dragOffset.x;
                                int deltaY = e.getY() - dragOffset.y;
                                upperLeft.x += deltaX;
                                upperLeft.y += deltaY;
                            }
                        }

                        frame.setLocation(upperLeft);
                        frame.toFront();
                        if (closeButtonFrame != null) {
                            refreshCloseButtonFrame();
                            closeButtonFrame.toFront();
                        }
                    }
                } else if ((id == MouseEvent.MOUSE_RELEASED &&
                            e.getButton() == MouseEvent.BUTTON1) ||
                           // A mouse moved event indicates the mouse button was
                           // released and we didn't detect it earlier because
                           // the mouse was moving too fast
                           (id == MouseEvent.MOUSE_MOVED)) {
                    // Release of the applet onto the desktop.

                    // FIXME: check to see whether it got outside the
                    // original parent's bounds or not during the drag
                    // operation. If not, assume the user didn't
                    // really mean to move it there, and put it back.

                    dragOffset = null;
                    if (!notificationsSent) {
                        sendDragFinished(applet);
                        listener.appletDroppedOntoDesktop(manager);
                        notificationsSent = true;
			setupCloseListener(applet);
                    }
                    setSomeoneDragging(false);
                }
            }
            // Consume all mouse events during drag operations
            e.consume();
        }

        // Implementation of a "close button" floating above and to
        // the right of the widget
        private Frame createCloseButton(final ActionListener closeListener) {
            Canvas closeButton = new Canvas() {
                    public void paint(Graphics graphics) {
                        Graphics2D g = (Graphics2D) graphics;
                        // Thickness of lines = 2.5% of minimum dimension
                        int rectWidth = (int)
                            Math.max(1, 0.025 * Math.min(getWidth(), getHeight()));
                        int centerX = getWidth() / 2;
                        int centerY = getHeight() / 2;
                        g.setColor(Color.WHITE);
                        // Draw border
                        g.fillRect(0, 0, rectWidth, getHeight());
                        g.fillRect(0, 0, getWidth(), rectWidth);
                        g.fillRect(0, getHeight() - rectWidth, getWidth(), rectWidth);
                        g.fillRect(getWidth() - rectWidth, 0, rectWidth, getHeight());
                        g.setStroke(new BasicStroke(rectWidth, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL));
                        float insetPercentage = 0.15f;
                        g.drawLine((int) (insetPercentage * getWidth()),
                                   (int) (insetPercentage * getHeight()),
                                   (int) ((1 - insetPercentage) * getWidth()),
                                   (int) ((1 - insetPercentage) * getHeight()));
                        g.drawLine((int) ((1 - insetPercentage) * getWidth()),
                                   (int) (insetPercentage * getHeight()),
                                   (int) (insetPercentage * getWidth()),
                                   (int) ((1 - insetPercentage) * getHeight()));
                    }
                };
            closeButton.setBackground(Color.BLACK);
            closeButton.addMouseListener(new MouseAdapter() {
                    public void mouseClicked(MouseEvent e) {
                        closeListener.actionPerformed(null);

                        if (closeButtonFrame != null) {
                            // Shut things down
                            closeButtonFrame.setVisible(false);
                            closeButtonFrame.dispose();
                            closeButtonFrame = null;
                        }
                    }
                });
            Frame f = (Frame)
                AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            return new Frame();
                        }
                    });
            f.setUndecorated(true);
            f.setResizable(false);
            // Make this window not show up in the task bar
            f.setFocusableWindowState(false);
            f.add(closeButton);
            f.setSize(CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE);
            return f;
        }

        // This ActionListener should be called when the close button
        // for the applet has been clicked
        class CloseListener implements ActionListener {
            public void actionPerformed(ActionEvent e) {
                if (closing) {
                    return;
                }

                closing = true;

                // Notify the outside world that the applet is being closed
		if (listener != null) {
		    listener.appletExternalWindowClosed(manager);
                }
                if (manager.isDisconnected()) {
                    // We have no need to keep track of this applet any more
                    unregister(manager);
                }

                // Clean up the external frame for the applet
                frame.setVisible(false);
                frame.dispose();
                frame = null;

                // If our manually-drawn close button is still there,
                // shut that down as well
                if (closeButtonFrame != null) {
                    closeButtonFrame.setVisible(false);
                    closeButtonFrame.dispose();
                    closeButtonFrame = null;
                }
            }
        }

        // The following use of reflection is partly a workaround
        // because we can't add APIs or event listeners in a Java SE
        // update release. Ideally we would have an event / listener
        // based registration mechanism for them. However this might
        // introduce compatibility problems for applets that want to
        // run on earlier JRE releases.

        // State for isDragStartEvent
        private boolean initializedDragStartMethod;
        private Method  dragStartMethod;
        private boolean isDragStartEvent(Applet a, MouseEvent e) {
            if (!initializedDragStartMethod) {
                initializedDragStartMethod = true;
                // See whether the applet has the following method:
                // public boolean isAppletDragStart(MouseEvent e);
                try {
                    dragStartMethod =
                        a.getClass().getMethod("isAppletDragStart",
                                               new Class[] { MouseEvent.class });
                } catch (Throwable t) {
                    // Any errors during this lookup are ignored and
                    // the applet loses its chance to override this
                    // functionality
                }
            }

            if (dragStartMethod != null) {
                try {
                    Boolean res = (Boolean)
                        dragStartMethod.invoke(a, new Object[] { e });
                    return res.booleanValue();
                } catch (Throwable t) {
                    // Any errors during this operation are ignored and
                    // the applet loses its chance to override this
                    // functionality
                    dragStartMethod = null;
                }
            }

            // Default behavior is to use Alt + Left Click + Drag as the drag operation
            int mods = e.getModifiersEx();

            // Don't use Command-Click on Mac OS X as a drag gesture
            // because it has special semantics
            return (e.getButton() == MouseEvent.BUTTON1 &&
                    (mods == (InputEvent.BUTTON1_DOWN_MASK | InputEvent.ALT_DOWN_MASK) ||
                     (!isOSX && mods == (InputEvent.BUTTON1_DOWN_MASK | InputEvent.META_DOWN_MASK))));
        }

        private void sendDragStarted(Applet applet) {
            if (DEBUG) {
                System.out.println("DragHelper sending appletDragStarted for applet ID " + manager.getAppletID());
            }
            try {
                Method m = applet.getClass().getMethod("appletDragStarted", null);
                m.invoke(applet, null);
            } catch (Throwable t) {
                // Silently ignore any failures
            }
        }

        private void sendDragFinished(Applet applet) {
            if (DEBUG) {
                System.out.println("DragHelper sending appletDragFinished for applet ID " + manager.getAppletID());
            }
            try {
                Method m = applet.getClass().getMethod("appletDragFinished", null);
                m.invoke(applet, null);
            } catch (Throwable t) {
                // Silently ignore any failures
            }
        }

        private void sendAppletRestored(Applet applet) {
            if (DEBUG) {
                System.out.println("DragHelper sending appletRestored for applet ID " + manager.getAppletID());
            }
            try {
                Method m = applet.getClass().getMethod("appletRestored", null);
                m.invoke(applet, null);
            } catch (Throwable t) {
                // Silently ignore any failures
            }
        }

        private boolean sendSetAppletCloseListener(Applet applet, ActionListener l) {
            if (DEBUG) {
                System.out.println("DragHelper sending setAppletCloseListener for applet ID " + manager.getAppletID());
            }
            try {
                Method m = applet.getClass().getMethod("setAppletCloseListener",
                                                       new Class[] { ActionListener.class });
                m.invoke(applet, new Object[] { l });
                return true;
            } catch (Throwable t) {
                // If the method didn't exist, or there was any error
                // callng it, return false
                return false;
            }
        }
    }

    // Quick filter to prevent iteration where not necessary
    private static boolean couldBeDragStartEvent(AWTEvent ae) {
        int id = ae.getID();
        if (id != MouseEvent.MOUSE_PRESSED &&
            id != MouseEvent.MOUSE_DRAGGED) {
            // Quickly reject events that we aren't interested in
            return false;
        }

        // We may need to consult the various applets to see if they
        // want to handle it
        return true;
    }

    private static Point getCurrentMouseLocation() {
        // FIXME: should consider adding fallbacks for this to work on
        // earlier JREs
        return (Point) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return MouseInfo.getPointerInfo().getLocation();
                }
            });
    }

    private static Dimension getDragSize(Component component) {
        // Get the smallest dimension of the visible components up the hierarchy
        Component c = component;
        Dimension result = null;
        while (c != null) {
            Dimension cur = c.getSize();
            if (result == null ||
                cur.width < result.width ||
                cur.height < result.height) {
                result = cur;
            }
            c = c.getParent();
        }
        return result;
    }

    // Image that we put in place of the applet (FIXME: want better
    // customizability of this behavior; at least the background
    // color)
    private Image javaLogoImage;
    private Image getJavaLogoImage(final Component c) {
        if (javaLogoImage == null) {
            javaLogoImage = (Image)
                AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        URL logoURL = ClassLoader.getSystemResource("sun/plugin/util/JavaCupLogo-161.png");
                        Image logoImage = Toolkit.getDefaultToolkit().getImage(logoURL);
                        MediaTracker mt = new MediaTracker(c);
                        mt.addImage(logoImage, 0);
                        try {
                            mt.waitForID(0);
                        } catch (InterruptedException e) {
                        }
                        return logoImage;
                    }
                });
        }
        return javaLogoImage;
    }

    private Canvas getJavaLogoCanvas(Dimension size) {
        Canvas c = new Canvas() {
                public void paint(Graphics g) {
                    Image image = getJavaLogoImage(this);
                    if (image != null) {
                        Rectangle r =
                            getCenteredImageBoundsWithinContainer(new Dimension(image.getWidth(this),
                                                                                image.getHeight(this)),
                                                                  getParent().getSize());
                        Graphics2D g2d = (Graphics2D) g;
                        g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
                        g2d.drawImage(image, r.x, r.y, r.width, r.height, this);
                    }
                }
            };
        c.setSize(size);
        return c;
    }

    private static Rectangle getCenteredImageBoundsWithinContainer(Dimension imageSize,
                                                                   Dimension containerSize) {
        float aspectRatio          = (float) imageSize.width / (float) imageSize.height;
        float containerAspectRatio = (float) containerSize.width / (float) containerSize.height;
        if (containerAspectRatio < aspectRatio) {
            // May need to fit width
            if (imageSize.width > containerSize.width) {
                int newImageHeight = (int) (containerSize.width / aspectRatio);
                return new Rectangle(0, (containerSize.height - newImageHeight) / 2,
                                     containerSize.width, newImageHeight);
            }
        } else {
            // May need to fit height
            if (imageSize.height > containerSize.height) {
                int newImageWidth = (int) (containerSize.height * aspectRatio);
                return new Rectangle((containerSize.width - newImageWidth) / 2,
                                     0,
                                     newImageWidth, containerSize.height);
            }
        }

        return new Rectangle((containerSize.width - imageSize.width) / 2,
                             (containerSize.height - imageSize.height) / 2,
                             imageSize.width,
                             imageSize.height);
    }

    private static boolean isSubclass(Class c, String name) {
        if (c == null) {
            return false;
        }

        if (c.getName().equals(name)) {
            return true;
        }

        return isSubclass(c.getSuperclass(), name);
    }
}
