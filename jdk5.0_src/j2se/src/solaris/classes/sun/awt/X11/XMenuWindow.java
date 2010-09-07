/*
 * @(#)XMenuWindow.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import java.awt.Rectangle;
import java.awt.Point;
import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.MenuComponent;
import java.awt.peer.ComponentPeer;
import sun.misc.Unsafe;
import java.awt.GraphicsConfiguration;
import java.awt.Color;
import sun.awt.X11GraphicsConfig;

public abstract class XMenuWindow extends XWindow {

    private static final String MENU_TARGET = "Menu target";
    MenuComponent menuTarget;
    Frame frame;

    XMenuWindow() {
        super((Object)null);
    }

    XMenuWindow(MenuComponent target) {
        super(new XCreateWindowParams(new Object[] {
            MENU_TARGET, target,
            DELAYED, Boolean.TRUE}));
    }

    void instantPreInit(XCreateWindowParams params) {
        super.instantPreInit(params);
        Object target = params.get(MENU_TARGET);
        menuTarget = (MenuComponent)target;
    }

    abstract Frame getFrame();

    protected void initGraphicsConfiguration() {
        graphicsConfig = (X11GraphicsConfig) getFrame().getGraphicsConfiguration();
        graphicsConfigData = new AwtGraphicsConfigData(graphicsConfig.getAData());
    }

    public Object getTarget() {
        return menuTarget;
    }

    public void toFront() {
        try {
            XToolkit.awtLock();
            XlibWrapper.XRaiseWindow(XToolkit.getDisplay(), window);
            XlibWrapper.XMapWindow(XToolkit.getDisplay(), window);
            this.visible = visible;
            XlibWrapper.XFlush(XToolkit.getDisplay());
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    MouseEvent makeMouseEvent(MouseEvent me, int x, int y) {
        MouseEvent nme = new MouseEvent(getEventSource(), me.getID(), me.getWhen(), me.getModifiers(), x, y, me.getClickCount(), false, me.getButton());
        return nme;
    }

    protected boolean isEventDisabled(IXAnyEvent e) {
        switch (e.get_type()) {
          case (int) XlibWrapper.Expose :
          case (int) XlibWrapper.GraphicsExpose :  
          case (int) XlibWrapper.ButtonPress:
          case (int) XlibWrapper.ButtonRelease:
          case (int) XlibWrapper.MotionNotify:
          case (int) XlibWrapper.KeyPress:
          case (int) XlibWrapper.KeyRelease:
          case (int) XlibWrapper.DestroyNotify:
              return super.isEventDisabled(e);
          default:
              return true;
        }
    }

    void postEvent(final AWTEvent event) {
        if (event instanceof ActionEvent || event instanceof ItemEvent) {
            super.postEvent(event);
        }

        // Since Menu component is not a Component, we don't want this X event
        // to reach Java Queue. But we'd like to work with Java events
        // instead of X events. So we dispatch it right here and skip.
        EventQueue.invokeLater(new Runnable() {
                public void run() {
                    if (!isVisible() || isDisposed()) {
                        return;
                    }
                    handleEvent(event);
                }
            });
    }

    private void handleEvent(AWTEvent event) {
        switch(event.getID()) {
          case MouseEvent.MOUSE_PRESSED:
          case MouseEvent.MOUSE_RELEASED:
          case MouseEvent.MOUSE_CLICKED:
          case MouseEvent.MOUSE_MOVED:
          case MouseEvent.MOUSE_ENTERED:
          case MouseEvent.MOUSE_EXITED:
          case MouseEvent.MOUSE_DRAGGED:
              handleJavaMouseEvent((MouseEvent)event);
              break;
          case KeyEvent.KEY_PRESSED:
          case KeyEvent.KEY_RELEASED:
              handleJavaKeyEvent((KeyEvent)event);
              break;
          case PaintEvent.PAINT:
              handleJavaPaintEvent((PaintEvent)event);
              break;
        }        
    }

    void handleJavaPaintEvent(PaintEvent event) {
        Rectangle rect = event.getUpdateRect();
        repaint(rect.x, rect.y, rect.width, rect.height);
    }

    void handleJavaMouseEvent(MouseEvent event) {
    }

    void handleJavaKeyEvent(KeyEvent event) {
    }

    void setVisible(boolean b) {
        xSetVisible(b);
    }

    public void doDispose() {
	setVisible(false);
	XToolkit.targetDisposedPeer(getMenuTarget(), this);        
	// XMenuItemPeer which are not XMenuPeer have no surfaceData,
	// because they have no X Window, since they are drawn lightweight
	// on the XMenuPeer's X Window, so they have no additional resources
	// to dispose.
	if (surfaceData != null) {  
            XToolkit.removeSourceEvents(XToolkit.getEventQueue(menuTarget), menuTarget, true);
	    super.dispose();
	}
    }

    public void dispose() {
        setDisposed(true);
	setVisible(false);
        EventQueue.invokeLater(new Runnable() {
	    public void run() {
                doDispose();
	    }
        });
    }    

    public Component getEventSource() {
        // We cannot use synchronized(getStateLock()) here
        // because this method is called on the Toolkit thread
        // while getAWTLock() is held. It would be acquired in
        // the wrong order.
            return frame;
    }

    MenuComponent getMenuTarget() {
        return menuTarget;
    }

    void setMenuTarget(MenuComponent m) {
        menuTarget = m;
    }
}
