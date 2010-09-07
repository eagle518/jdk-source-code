/*
 * @(#)XMenuBarPeer.java	1.33 04/06/08
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.util.Vector;
import java.util.Iterator;
import java.awt.*;
import java.awt.event.*;
import java.awt.peer.*;
import sun.awt.motif.X11FontMetrics;
import sun.awt.X11GraphicsConfig;
import java.util.logging.*;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

public class XMenuBarPeer extends XMenuComponentPeer implements MenuBarPeer {

    private static Logger log = Logger.getLogger("sun.awt.X11.XMenuBarPeer");
    private final static int GAP = 10;
    private XMenuPeer helpMenu;
    private XMenuPeer menu;
    private int menuSelected = -1;
    // Vector of MenuData objects in display order, Help menu at the end.
    private Vector menus = new Vector(); 
    private static final String FRAME = "Frame";
    private static Field f_helpMenu;
    private static Field f_menus;
    private int lastPaintHeight = 0;

    static {
        f_helpMenu = XToolkit.getField(MenuBar.class, "helpMenu");
        f_menus = XToolkit.getField(MenuBar.class, "menus");
    }

    MenuData[] copyMenus() {
        return (MenuData[])getMenus().toArray(new MenuData[] {});
    }

    public XMenuBarPeer(MenuBar target) {
        super(target);
    }

    void init(Frame frame) {
	XCreateWindowParams params = getDelayedParams();
	params.add(FRAME, frame);
	params.remove(DELAYED);
	init(params);
    }

    void preInit(XCreateWindowParams params) {
	super.preInit(params);
	params.add(PARENT_WINDOW, ((XFramePeer)(getPeer(getFrame()))).getShell());
    }

    Menu[] copyTargetMenus() { 
        return (Menu[])getTargetMenus().toArray(new Menu[] {});
    }

    class MenuData {
	XMenuPeer menu;
	int index;  // Menu Index.
	MenuData (XMenuPeer menu, int index) {
	    this.menu = menu;
	    this.index = index;
	}
	public String toString() {
	    return getClass().getName() + "[menu=" + menu + ",index=" + index + "]";
	}
    }

    void postInit(XCreateWindowParams params) {
	super.postInit(params);
	// Get menus from the target.
        Vector l_menus = new Vector();
        Menu[] targetMenus = copyTargetMenus();
        int nitems = targetMenus.length;
        for (int i = 0; i < nitems; i++) {
            Menu menu = targetMenus[i];
            if (menu != null) {
                l_menus.add(new MenuData((XMenuPeer)getPeer(menu), i));
            }
        }
        setMenus(l_menus);
        addHelpMenu(targetGetHelpMenu());
	toFront();
    }

    int getHeight() {        
        int height = 0;
        MenuData[] l_menus = copyMenus();
        int nitems = l_menus.length;
        // Find the maximum font height of all menus.
	for (int i = 0 ; i < nitems ; i++) {
	    XMenuPeer mn = l_menus[i].menu;
	    height = Math.max(height, getFontMetrics(mn.getFont()).getHeight());
        }
        return height + 10;
    }

    public void addMenu(Menu m) {
        if (getHelpMenu() != null) {
            // Insert before the helpMenu.
	    getMenus().insertElementAt(new MenuData((XMenuPeer)getPeer(m), getMenus().size()), getMenus().size() - 1 );
        }
        else {
            // Append.
	    getMenus().addElement(new MenuData((XMenuPeer)getPeer(m), getMenus().size()));
        }
	repaint();
    }

    int peerToPeerIndex(XMenuPeer m) {
        for (int i = 0; i < getMenus().size(); i++) {
            if (m == getMenu(i)) {
                return i;
            }
        }
        return -1;
    }

    int menuIndexToPeerIndex(int index) {
        for (int i = 0; i < getMenus().size(); i++) {
            if (index == getMenuIndex(i)) {
                return i;
            }
        }
        return -1;
    }

    void updateMenuIndexesAfterDeletion(int index) {
        // If a Menu index is greater than index, decrement it by one.
        for (int i = 0; i < getMenus().size(); i++) {
            if (getMenuIndex(i) > index) {
                ((MenuData)(getMenus().elementAt(i))).index--;
            }
        }
    }

    /**
     * Delete a menu.
     */
    public void delMenu(int index) {
        synchronized(getMenus()) {
            if (index < getMenus().size()) {
  		if (getMenuSelected() >= 0) {
                    XMenuPeer l_menu = getMenu();
                    if (l_menu != null) {
			l_menu.popdown(null, false);
			l_menu.setPosted(false);
			setMenu(null);
                    }
                    setMenuSelected(-1);
  		}
                int i = menuIndexToPeerIndex(index);
		if (getMenu(i) == getHelpMenu()) {
		    setHelpMenu(null);
		}
	        getMenus().removeElementAt(i); 
                updateMenuIndexesAfterDeletion(index);
            }
        }
	repaint();
    }

    public void moveHelpMenuToLast() {
        synchronized(getMenus()) {
            int i = peerToPeerIndex(getHelpMenu());
	    if ((i >= 0) && (i < getMenus().size() - 1)) {
		MenuData mm = (MenuData)getMenus().elementAt(i);
		getMenus().removeElementAt(i);
		getMenus().addElement(mm);
	    }
        }
    }

    /**
     * Add help menu.
     */
    public void addHelpMenu(Menu m) {
        // Note that MenuBar.setHelpMenu(m) calls add(m) 
        // which calls peer.addMenu(m), which puts the menu
        // in the menus vector.
        if (m == null) {
            setHelpMenu(null);
            return;
        }
        setHelpMenu((XMenuPeer)getPeer(m));
        moveHelpMenuToLast();
	repaint();
    }

    XMenuPeer selectMenuByKeyPress(int l_menuSelected, int keyCode) {
	if (XAwtState.getGrabWindow() != this) {
	    return null;
	}
        setMenuSelected(l_menuSelected);
        XMenuPeer l_menu = getMenu();
	if (l_menu == null && 
	    !(keyCode == KeyEvent.VK_SPACE || 
              keyCode == KeyEvent.VK_ENTER ||
              keyCode == KeyEvent.VK_DOWN)) {
	    repaint();
	    return null;
	}
        XMenuPeer oldmenu = l_menu;
        XMenuPeer l_helpMenu = getHelpMenu();
        setMenu(getMenu(l_menuSelected));
        l_menu = getMenu();
	if (l_menu != null) {
	    MenuData[] l_menus = copyMenus();
	    int nitems = l_menus.length;
	    int mx = 0;
            boolean doPopup = true;
            for (int i = 0 ; i < nitems ; i++) {
		XMenuPeer mn = l_menus[i].menu;
		String item = mn.getLabel();
                FontMetrics fm = getFontMetrics(mn.getFont());
		int w = fm.stringWidth(item) + GAP*2;
                if (i == l_menuSelected) {
		    if (mn == l_helpMenu) {
                        mx = getWidth() - w;
		    }
                    if (mn.isViewable()) {
                        Point pt = toGlobal(mx + 1, height - 4);
                        l_menu.popup(pt.x, pt.y);
                        l_menu.select(l_menu.getDownSelected());
                    }
		    if ((oldmenu != null) && (oldmenu != l_menu)) {
			oldmenu.setPosted(false);
			oldmenu.popdown(null, false);
		    }
                    repaint();
                    return l_menu;
                }
		else {
		    mx += w;
		}
	    }
	}
        return null;
    }

    /**
     * Select which menu we are in.
     */
    XMenuPeer selectMenu(MouseEvent mouseEvent, int x, int y) {
	XMenuPeer l_menu = getMenu();
	XMenuPeer l_helpMenu = getHelpMenu();
        int l_width = getWidth();
	if (y > height) {
	    return l_menu;
	}
	if (l_menu != null) {
	    if (l_menu.cascadeContains(this, new Point(x,y), 0)) {
		return l_menu;
	    }
	}
	MenuData[] l_menus = copyMenus();
	int nitems = l_menus.length;
	int mx = 0;

	// See if non Help Menu is selected
	for (int i = 0 ; i < nitems ; i++) {
	    XMenuPeer mn = l_menus[i].menu; 
            if (!mn.isViewable()) {
                return l_menu;
            }
	    String item = mn.getLabel();
	    FontMetrics fm = getFontMetrics(mn.getFont());
	    int w = fm.stringWidth(item) + GAP*2;
	    if (mn == l_helpMenu) {
		mx = l_width - w;
	    }
	    if (mn.isEnabled() && (x > mx) && (x <= mx + w)) {
		if (l_menu != mn) {
		    XMenuPeer oldmenu = l_menu;
		    Point pt = toGlobal(mx + 1, height - 4);
		    l_menu = mn;
		    setMenu(l_menu);
		    if (l_menu != null) {
			l_menu.popup(pt.x, pt.y);
		    }
		    setMenuSelected(i);
		    grabInput();
		    if (oldmenu != null) {
			oldmenu.setPosted(false);
			oldmenu.popdown(mouseEvent, false);
		    }
		    repaint();
		}
		return l_menu;
	    }
	    mx += w;
	}
	return l_menu;
    }

    /**
     * Paint the menu
     */
    void paint(Graphics g) {
        int l_width = getWidth();
        int l_height = getHeight();
        XMenuPeer l_helpMenu = getHelpMenu();
        int l_menuSelected = getMenuSelected();
	if (g == null) return;
        if (l_height != lastPaintHeight) {
            // The required height of the window changed, most likely
            // because of a font change.
            lastPaintHeight = l_height;
            // Reshape the frame's children including the menubar.
            ((XFramePeer)(getPeer(getFrame()))).updateChildrenSizes();
        }
	g.setColor(getBackground());
	g.fillRect(1, 1, l_width-2, l_height-2);

	g.setColor(getLightShadow());
	g.drawLine(0, 0, l_width-1, 0);
	g.drawLine(0, 0, 0, l_height-1);
	g.setColor(getDarkShadow());
	g.drawLine(l_width-1, 0, l_width-1, l_height-1);
	g.drawLine(0, l_height-1, l_width-1, l_height-1);

	int x = 0;

        MenuData[] l_menus = copyMenus();
        int nitems = l_menus.length;

	// Draw all menus.
	for (int i = 0 ; i < nitems ; i++) {
	    XMenuPeer mn = l_menus[i].menu;
            mn.setViewable(true);
	    String item = mn.getLabel();
            g.setFont(mn.getFont());
            FontMetrics fm = g.getFontMetrics();
            int y = (l_height + fm.getAscent() - fm.getDescent()) / 2;
	    int w = fm.stringWidth(item) + GAP*2;
	    if (mn == l_helpMenu) {  
		int helpx = l_width - w;  // helpMenu is the last menu.
                if ((helpx) >= x) {
                    // There is enough space to draw the helpMenu.
                    x = helpx;
                }
                else {
                    // Do not draw the helpMenu.
                    mn.setViewable(false);
                }
	    }
            if (x >= l_width) {
                // Not enough space to draw another menu.
                mn.setViewable(false);
            }
            if (mn.isViewable()) {
                if (i == l_menuSelected) {
                    g.setColor(getSelect());
                    g.fillRect((x) + 1, 3, w - 1, l_height - 6);
                    draw3DRect(g, (x) + 1, 3, w - 2, l_height - 7, false);
                }
                g.setColor(mn.isEnabled() ? getForeground() : getDisabled());
                g.drawString(item, x + GAP, y);
            }
	    x += w;
	}
    }    

    boolean contains(int x, int y, int pad) {
	return (x >= pad) && (x < getWidth() - pad)
	    && (y >= pad) && (y < getHeight() - pad);
    }

    /**
     *
     * @see java.awt.event.MouseEvent
     * MouseEvent.MOUSE_CLICKED
     * MouseEvent.MOUSE_PRESSED
     * MouseEvent.MOUSE_RELEASED
     * MouseEvent.MOUSE_MOVED
     * MouseEvent.MOUSE_ENTERED
     * MouseEvent.MOUSE_EXITED
     * MouseEvent.MOUSE_DRAGGED
     */
    void handleJavaMouseEvent( MouseEvent mouseEvent ) {
        XMenuPeer l_menu = getMenu();
	if ((mouseEvent.getModifiers() & InputEvent.BUTTON1_MASK) == 0) {
	    return;
	}
	switch (mouseEvent.getID()) {
	  case MouseEvent.MOUSE_PRESSED:
	      if ((XAwtState.getGrabWindow() == this) 
		  && !contains(mouseEvent.getX(), mouseEvent.getY(), 0) 
		  && (l_menu == null 
		      || !l_menu.cascadeContains( this, mouseEvent.getPoint(), 0)) ) 
	      {
		  ungrabInput();
                  setMenuSelected(-1);
		  if (l_menu != null) {
		      l_menu.setPosted(false);
		      l_menu.popdown(mouseEvent, false);
                      setMenu(null);
		  }
		  repaint();
	      }
	      else {
                  if ((l_menu = selectMenu(mouseEvent, mouseEvent.getX(), mouseEvent.getY())) != null) {
		      Point pt = l_menu.toLocal(toGlobal(mouseEvent.getX(), mouseEvent.getY()));
		      MouseEvent me = makeMouseEvent(mouseEvent, pt.x, pt.y);
		      l_menu.handleJavaMouseEvent(me);
		  }
	      }
	      break;
	  case MouseEvent.MOUSE_RELEASED:
	      if (!(l_menu == null) && l_menu.lastMenuContains( this, mouseEvent.getPoint(), 0 )) {
		  l_menu.setPosted(false);
		  l_menu.popdown(mouseEvent, true);
                  setMenu(null);
                  setMenuSelected(-1);
		  repaint(); 
		  ungrabInput();
	      }
	      else { 
		  doPosting( this, mouseEvent.getPoint(), 0 );
	      }
	      break;
	  case MouseEvent.MOUSE_DRAGGED:
              if ((l_menu = selectMenu(mouseEvent, mouseEvent.getX(), mouseEvent.getY())) != null) {
		  Point pt = l_menu.toLocal(toGlobal(mouseEvent.getX(), mouseEvent.getY()));
		  MouseEvent me = makeMouseEvent(mouseEvent, pt.x, pt.y);
		  l_menu.handleJavaMouseEvent(me);
	      }
	      break;
	}
    }

    void handleEscapeKeyPress() {
	doPosting( this, new Point(0,0), 0 );
        setMenuSelected(-1);
	ungrabInput();
	repaint(); 
    }

    void handleF10KeyPress() {
        XMenuPeer l_menu = getMenu();
	if (getMenuSelected() == -1) {
            setMenuSelected(0);
	    grabInput();
	}
	else {
	    if (l_menu != null) {
		if (l_menu.isPosted()) {
		    l_menu.setPosted(false);
		    l_menu.popdown(null, false);
                    setMenu(null);
		}
	    }
            setMenuSelected(-1);
	    ungrabInput();
	}
	repaint(); 
    }

    int getLeftSelected() {
        MenuData[] l_menus = copyMenus();
        int nitems = l_menus.length;
        int sel = getMenuSelected();
	XMenuPeer mn;
	do {
	    sel = (sel==0) ? nitems-1 : (sel-1);
	    mn = l_menus[sel].menu;
	} while (!mn.isEnabled() || !mn.isViewable());
	return sel;
    }

    int getRightSelected() {
        MenuData[] l_menus = copyMenus();
        int nitems = l_menus.length;
        int sel = getMenuSelected();
	XMenuPeer mn;
	do {
	    sel = (sel+1) % nitems;
	    mn = l_menus[sel].menu;
	} while (!mn.isEnabled() || !mn.isViewable());
	return sel;
    }

    public void handleKeyPress(long ptr) {
        final int keyCode = nativeGetKeyCode(getFrame(),KeyEvent.KEY_PRESSED,ptr);
        EventQueue.invokeLater(new Runnable() {
	    public void run() {
		handleKeyPressOnEDT(keyCode);
	    }
        });
    }

    void handleKeyPressOnEDT(int keyCode) {
        XMenuPeer l_menu = getMenu();
        XMenuPeer oldMenu = null;
        int l_menuSelected = getMenuSelected();
	boolean isSubMenuChange = false;
	if (l_menu != null) {
	    isSubMenuChange = l_menu.handleKeyPress1(keyCode, (XMenuPeer) null);
	}
	switch(keyCode) {
	  case KeyEvent.VK_UP:
	      break;
	  case KeyEvent.VK_DOWN:
	      if (isSubMenuChange) return;
	      if (l_menuSelected != -1) {
		  if (l_menu==null) {
		      l_menu = selectMenuByKeyPress(l_menuSelected, keyCode);
		      doPosting( this, new Point(0,0), 0 );
		  }
              }
	      break;
	  case KeyEvent.VK_LEFT:
	      if (isSubMenuChange) return;
              oldMenu = getMenu();
	      l_menu = selectMenuByKeyPress(getLeftSelected(), keyCode);
              if (l_menu != oldMenu) {
	          doPosting( this, new Point(0,0), 0 );
              }
	      break;
	  case KeyEvent.VK_RIGHT:
	      if (isSubMenuChange) return;
              oldMenu = getMenu();
	      l_menu = selectMenuByKeyPress(getRightSelected(), keyCode);
              if (l_menu != oldMenu) {
	          doPosting( this, new Point(0,0), 0 );
              }
	      break;
	  case KeyEvent.VK_SPACE:
	  case KeyEvent.VK_ENTER:
	      if (isSubMenuChange) return;
	      if (l_menuSelected != -1) {
		  if (l_menu==null) {
		      l_menu = selectMenuByKeyPress(l_menuSelected, keyCode);
		      doPosting( this, new Point(0,0), 0 );
		  }
		  else {
		      l_menu.setPosted(false);
		      l_menu.popdown(new MouseEvent(getEventSource(), MouseEvent.MOUSE_RELEASED, 0, 0, 0, 0, 0, false),
				   true);
                      setMenu(null);
                      setMenuSelected(-1);
		      repaint();
		      ungrabInput();
		  }
	      }
	      break;
	  case KeyEvent.VK_ESCAPE:
	      handleEscapeKeyPress();
	      break;
	  case KeyEvent.VK_F10:
	      handleF10KeyPress();
	      break;
	  default:
	      break;
	}
    }

    XMenuPeer doPosting(XMenuWindow win, Point p, int pad) {
        XMenuPeer l_menu = getMenu();
	Point npt = new Point(p.x, p.y);
	if (contains(npt.x, npt.y, pad)) {
	    XMenuPeer return_menu = l_menu;
	    if (l_menu != null) {
		if (l_menu.isPosted()) {
		    l_menu.setPosted(false);
		    l_menu.popdown(null, false);
                    setMenu(null);
                    setMenuSelected(-1);
		    repaint();
		    ungrabInput();
		}
		else {
		    l_menu.setPosted(true);
		}
	    }
	    return return_menu;
	} else if (l_menu == null) {
	    return null;
	} else {
	    npt = l_menu.toLocal(win.toGlobal(npt));
	    return l_menu.doPosting(this, npt, pad);
	}
    }

    static final int W_DIFF = (XFramePeer.CROSSHAIR_INSET + 1) * 2;
    static final int H_DIFF = XFramePeer.BUTTON_Y + XFramePeer.BUTTON_H;

    /*
     * Print the native component by rendering the Motif look ourselves.
     */
    void print(Graphics g) {
	Frame f = getFrame();
	Dimension fd = f.size();
	Insets insets = f.insets();
	
	/* Calculate menubar dimension. */
	int width = fd.width;
	int height = insets.top;
	if (getPeer(f) instanceof XFramePeer) {
	    XFramePeer fpeer = (XFramePeer)getPeer(f);
	    if (fpeer.hasDecorations(XWindowAttributesData.AWT_DECOR_BORDER)) {
		width -= W_DIFF;
		height -= XFramePeer.BUTTON_Y;
	    }
	    if (fpeer.hasDecorations(XWindowAttributesData.AWT_DECOR_MENU)) {
		height -= XFramePeer.BUTTON_H;
	    }
	}
	Dimension d = new Dimension(width, height);

	Shape oldClipArea = g.getClip();
	g.clipRect(0, 0, d.width, d.height);

	Color bg = f.getBackground();
	Color fg = f.getForeground();
	Color highlight = bg.brighter();
	Color shadow = bg.darker();

	// because we'll most likely be drawing on white paper,
	// for aesthetic reasons, don't make any part of the outer border
	// pure white
	if (highlight.equals(Color.white)) {
	    g.setColor(new Color(230, 230, 230));
	}
	else {
	    g.setColor(highlight);
	}
	g.drawLine(0, 0, d.width, 0);
	g.drawLine(1, 1, d.width - 1, 1);
	g.drawLine(0, 0, 0, d.height);
	g.drawLine(1, 1, 1, d.height - 1);
	g.setColor(shadow);
	g.drawLine(d.width, 1, d.width, d.height);
	g.drawLine(d.width - 1, 2, d.width - 1, d.height);
	g.drawLine(1, d.height, d.width, d.height);
	g.drawLine(2, d.height - 1, d.width, d.height - 1);

	int x = GAP;
        MenuData[] l_menus = copyMenus();
        int nitems = l_menus.length;
	for (int i = 0 ; i < nitems ; i++) {
	    XMenuPeer mn = l_menus[i].menu;
	    String item = mn.getLabel();
	    Font menuFont = getFont();
	    g.setFont(menuFont);
	    FontMetrics menuMetrics = g.getFontMetrics();
	    int y = (d.height / 2) + menuMetrics.getMaxDescent();
	    int w = menuMetrics.stringWidth(item) + GAP * 2;

	    if (x >= d.width) {
		break;
	    }
	    if (mn.isEnabled()) {
		g.setColor(fg);
	    }
	    else {
		// draw text as grayed out
		g.setColor(shadow);
	    }
	    if (getHelpMenu() == mn) {
		g.drawString(item, d.width - w + GAP, y);
	    }
	    else {
		g.drawString(item, x, y);
		x += w;
	    }
	}
	g.setClip(oldClipArea);
    }

    public void dispose() {
	// Dispose of sub-menus
	Iterator iter = menus.iterator();
	while (iter.hasNext()) {
	    XMenuPeer menu = (XMenuPeer)((MenuData)iter.next()).menu;
	    if (menu != null) {
		menu.dispose();
	    }
	}
	super.dispose();
    }

    Menu targetGetHelpMenu() {
        try {
	    return (Menu)f_helpMenu.get(menuTarget);
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
            return null;
        }
    }

    XMenuPeer getHelpMenu() {
        return helpMenu;
    }

    void setHelpMenu(XMenuPeer h) {
        helpMenu = h;
    }

    int getMenuCount() {
        return menus.size();
    }

    final XMenuPeer getMenu(int i) {
        synchronized(getMenus()) {
            if (i < getMenus().size()) {
                return (XMenuPeer) ((MenuData)(getMenus().elementAt(i))).menu;
            }
            else {
                return null;
            }
        }
    }

    final int getMenuIndex(int i) {
        synchronized(getMenus()) {
            if (i < getMenus().size()) {
                return (int) ((MenuData)(getMenus().elementAt(i))).index;
            }
            else {
                return -1;
            }
        }
    }


    Vector getMenus() {
        return menus;
    }

    void setMenus(Vector m) {
        menus = m;
    }

    Vector getTargetMenus() {
        try {
            return (Vector)f_menus.get(menuTarget);
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
            return null;
        }
    }

    XMenuPeer getMenu() {
        return menu;
    }

    void setMenu(XMenuPeer m) {
        menu = m;
    }

    int getMenuSelected() {
        return menuSelected;
    }

    void setMenuSelected(int m) {
        menuSelected = m;
    }
}
