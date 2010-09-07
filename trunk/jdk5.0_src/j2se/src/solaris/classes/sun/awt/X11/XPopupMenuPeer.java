/*
 * @(#)XPopupMenuPeer.java	1.17 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.X11GraphicsConfig;

public class XPopupMenuPeer extends XMenuPeer implements PopupMenuPeer
{
    static XPopupMenuPeer showingPopupMenu;
    // On MOUSE_RELEASE, hide popup if only drag or press was detected,
    // therefore don't hide on trigger click.
    private boolean dragDetected = false;
    private boolean pressDetected = false;
    
    public XPopupMenuPeer(PopupMenu target) {
        super((Menu)target, true);
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        setBackground(((Component)getParent()).getBackground());
    }

    public void show(Event e) {
        if (getParent() instanceof Component) {
            create();
            Point p = ((Component)e.target).getLocationOnScreen();
            e.x += p.x;
            e.y += p.y;
	    if (getShowingPopupMenu() != null) {
		getShowingPopupMenu().popdown(null, false);
	    }
	    popup(e.x, e.y);
            setShowingPopupMenu(this);
            grabInput();
            dragDetected = false;
            pressDetected = false;
        } else {
            throw new IllegalArgumentException("illegal popup menu container class");
        }
    }

    void popdown(MouseEvent mouseEvent, boolean action) {
	setPosted(false);
	ungrabInput();
	super.popdown(mouseEvent, action);
	if (getShowingPopupMenu() == this) {
	    setShowingPopupMenu(null);
	}
    }

    void handleJavaMouseEvent(MouseEvent mouseEvent) {
	switch (mouseEvent.getID()) {
	  case MouseEvent.MOUSE_PRESSED:
          pressDetected = true;
	      if ((XAwtState.getGrabWindow() == this) 
		  && (!contains(mouseEvent.getX(), mouseEvent.getY(), 0)) 
		  && ((getMenu() == null) || !getMenu().cascadeContains( this, mouseEvent.getPoint(), 0)) ) 
	      {
		  popdown(mouseEvent, false);
	      }
	      super.handleJavaMouseEvent(mouseEvent);
	      break;

	  case MouseEvent.MOUSE_RELEASED:
          if (isCreated()) {
              if (lastMenuContains( this, mouseEvent.getPoint(), 0 )) {
                  if( pressDetected || dragDetected ) {
                      popdown(mouseEvent, true);
                  }
              }
		      else {
                  doPosting( this, mouseEvent.getPoint(), 0 );
              }
          }
	      break;
	  case MouseEvent.MOUSE_DRAGGED:
          dragDetected = true;
	      super.handleJavaMouseEvent(mouseEvent);
	      break;
	}
    }

    public void dispose() {
	if (getShowingPopupMenu() == this) {
            setShowingPopupMenu(null);
	}
        super.dispose();
    }

    XPopupMenuPeer getShowingPopupMenu() {
        return showingPopupMenu;
    }

    void setShowingPopupMenu(XPopupMenuPeer m) {
        showingPopupMenu = m;
    }
}
