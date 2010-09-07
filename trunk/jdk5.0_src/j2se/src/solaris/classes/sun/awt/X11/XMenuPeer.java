/*
 * @(#)XMenuPeer.java	1.33 04/05/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.motif.X11FontMetrics;
import javax.swing.Timer;
import javax.swing.AbstractAction;
import sun.awt.X11GraphicsConfig;
import java.util.Vector;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import java.util.logging.*;

public class XMenuPeer extends XMenuItemPeer implements MenuPeer {
    private static final Logger log = Logger.getLogger("sun.awt.X11.menu.XMenuPeer");

    private final static int shortCutSpacing = 15;
    private int xIndent = 20;
    private int shortcutOffset = 0;
    private XMenuPeer menu;
    private XMenuComponentPeer parentMenu;
    private int selected;
    private boolean created;
    private Point pt;
    private int titleOffset = 0;
    private boolean titleShowing = false;
    private boolean  posted = false;
    private int popup_i = -1;
    private Timer popupTimer = null;
    private Vector items = new Vector(); // List of XMenuItemPeer objects
    private static Field f_items;
    private boolean viewable = false;
    final static int checkBorder = 6;

    static {
        f_items = XToolkit.getField(Menu.class, "items");
    }

    XMenuItemPeer[] copyItems() {
        return (XMenuItemPeer[])getItems().toArray(new XMenuItemPeer[] {});
    }

    public XMenuPeer(Menu target) {
        super(target);
        created = false;
        pt = new Point(0,0);
    }

    public XMenuPeer(Menu target, boolean titleShowing) {
        this(target);
        this.titleShowing = titleShowing;
    }

    public void instantPreInit(XCreateWindowParams params) {
	super.instantPreInit(params);
	MenuContainer cont = getParent();
	if (cont instanceof MenuComponent) {
	    parentMenu = (XMenuComponentPeer)getPeer((MenuComponent)cont);
	}        
    }

    void preInit(XCreateWindowParams params) {
	super.preInit(params);
	params.add(OVERRIDE_REDIRECT, Boolean.TRUE);
	if (parentMenu == null) {
	    MenuContainer cont = getParent();
	    if (cont instanceof MenuComponent) {
		parentMenu = (XMenuComponentPeer)getPeer((MenuComponent)cont);
	    }        
	}
    }

    void create() {
        setSelected(-1);
	if (!isCreated()) {
	    XCreateWindowParams params = getDelayedParams();
	    params.remove(DELAYED);
	    init(params);  
	    initItems();  
	}
        setCreated(true);
    }

    boolean isTitleShowing() {
	String l_title = getLabel();
	if ((l_title == null) || l_title.equals("")) {
            setTitleShowing(false);
	}
        return titleShowing;
    }

    void setTitleShowing(boolean t) {
        titleShowing = t;
    }

    int getWidestItemWidth() {
	FontMetrics fm = getFontMetrics(getFont());
	XMenuItemPeer[] localItems = copyItems();
	int nitems = localItems.length;
	int w = (isTitleShowing() ? fm.stringWidth(getLabel()) : 0);
	Graphics g = getGraphics();
        if (g == null) {
            return 0;
        }
	for (int i = 0 ; i < nitems ; i++) {
	    w = Math.max(w, localItems[i].getWidth(g));
	}
	g.dispose();
	return w;
    }

    int getWidestShortcutWidth() {
	FontMetrics fm = getFontMetrics(getFont());
	XMenuItemPeer[] localItems = copyItems();
	int nitems = localItems.length;
	int w = 0;
	Graphics g = getGraphics();
        if (g == null) {
            return 0;
        }
	for (int i = 0 ; i < nitems ; i++) {
	    w = Math.max(w, localItems[i].getShortcutWidth(g));
	}
	g.dispose();
	return w;
    }

    Point getTopBot(int index) {
	int top = 0;
	int bot = 2 + getTitleOffset();
	Graphics g = getGraphics();
        if (g == null) {
            return new Point(0,0);
        }
	XMenuItemPeer[] localItems = copyItems();
        int nitems = localItems.length;
	if (index >= nitems) {
	    index = nitems - 1;
	}
	for (int i = 0; i < index+1; i++) {
	    XMenuItemPeer mpeer = localItems[i];
	    String str = mpeer.getLabel();
	    if (str.equals("-")) {
		top = bot + 1;
		bot = top + 2;
	    } else {
		top = bot + 1;
		bot = top + mpeer.getHeight(g) - 1;
	    }
	}
	g.dispose();
	return new Point(top, bot);
    }
    
    int getTop(int i) {
        return getTopBot(i).x;
    }

    int getBot(int i) {
        return getTopBot(i).y;
    }

    MenuItem[] copyTargetItems() {
        return (MenuItem[])getTargetItems().toArray(new MenuItem[] {});
    }

    // Initialize the items vector.
    void initItems() {
	Vector localItems = new Vector();
	MenuItem[] targetItems = copyTargetItems();
	int nitems = targetItems.length;
	for (int i = 0; i < nitems; i++) {
	    MenuItem mi = targetItems[i];
	    XMenuItemPeer mpeer = (XMenuItemPeer)getPeer(mi);
            if (mpeer != null) {
	        mpeer.setMenuPeer(this);
	        localItems.add(mpeer);
            }
	}
        setItems(localItems);
    }

    int computeXIndent() {
	int xIndent = 0;
	XMenuItemPeer[] localItems = copyItems();
	int nitems = localItems.length;
	Graphics g = getGraphics();
        if (g == null) {
            return 0;
        }
	int maxHeight = 0;
	for (int i = 0 ; i < nitems ; i++) {
	    XMenuItemPeer mpeer = localItems[i];
	    if (mpeer instanceof XCheckboxMenuItemPeer) {
		int itemHeight = ((XCheckboxMenuItemPeer)mpeer).getHeight(g);
		maxHeight = Math.max(maxHeight, itemHeight);
	    }
	}
	if (maxHeight != 0) {
	    xIndent = maxHeight + checkBorder*2 + 4;
	} else {
	    xIndent = 4;
	}
	g.dispose();
	return xIndent;
    }

    /**
     * Update the x, y, width, height, and other menu peer data.
     */
    void updateMenu(int x, int y) {
        int w, h;
	FontMetrics fm = getFontMetrics(getFont());

	setTitleOffset(isTitleShowing() ? (fm.getMaxAscent() + fm.getMaxDescent() + 2) : 0);

	// Compute the height of the menu.
	h = getBot(getItems().size()-1) + 2;

	setXIndent(computeXIndent());

	// Compute the width by searching for the widest item.
	int shortcutWidth = getWidestShortcutWidth();
        int widestItemWidth = getWidestItemWidth();
        setShortcutOffset(widestItemWidth + shortCutSpacing);
	w = widestItemWidth + getXIndent()*2 + ((shortcutWidth == 0) ? 0 : shortCutSpacing + shortcutWidth); 

	// Find the screen dimensions.
	Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
	int screenWidth  = d.width;
	int screenHeight = d.height;

	// Check for a menu that is partially offscreen.  
	// First we push the menu up and to the left. Then we push it
	// down and to the right.  As a result, if it is too big for
	// the screen, the top-left corner will show and the
	// bottom-right corner will be clipped.
	if ((x + w) >= screenWidth) {
	    x = screenWidth - w;
	}
	if ((y + h) >= screenHeight) {
	    y = screenHeight - h;
	}
	if (x < 0) {
	    x = 0;
	}
	if (y < 0) {
	    y = 0;
	}
	reshape(x, y, w, h);
    }

    /**
     * Show at an x,y location.
     */
    void popup(int x,int y) {
	if (log.isLoggable(Level.FINE)) log.fine("Popping up " + this + " at " + x + ", " + y);
	if (isDisposed()) {
	    return;
	}
	create();   
	updateMenu(x, y);
	setVisible(true);
	toFront();
	repaint();
    }

    /**
     * Hide the menu and execute the action if
     * action is true.
     */
    void popdown(MouseEvent mouseEvent, boolean action) {
        XMenuPeer l_menu = getMenu(); 
        int l_selected = getSelected();
	if (log.isLoggable(Level.FINE)) log.fine("Popping down " + this + " because " + mouseEvent + (action?" with action":""));
	setVisible(false);
        setPosted(false);      
	if (l_menu != null) {
	    if (log.isLoggable(Level.FINE)) log.fine("Popping down child menu " + l_menu);
	    l_menu.popdown(mouseEvent, action);
            setMenu(null);
	} else if (action && (l_selected >= 0)) {
	    XMenuItemPeer mi = getItem(l_selected);
	    if ((mi != null) && !(mi instanceof XMenuPeer)) {
		try {
		    if (mi instanceof XCheckboxMenuItemPeer) {
			XCheckboxMenuItemPeer cb = (XCheckboxMenuItemPeer)mi;
			cb.action( mouseEvent.getWhen(), 
				   mouseEvent.getModifiers(),
				   !cb.getState() );
		    }
		    else {
			mi.action( mouseEvent.getWhen(), 
				   mouseEvent.getModifiers() );
		    }
		}
		catch (ThreadDeath e) {
		    throw e;
		}
		catch (Throwable e) {
		    e.printStackTrace();
		}
	    }
	}
        setSelected(-1);
    }

    /**
     * Paint the entire menu.
     */
    void paint(Graphics g) {
        String l_label = getLabel();
        int l_titleOffset = getTitleOffset();
	draw3DRect(g, 0, 0, width-1, height-1, true);
	g.setFont(getFont());
	FontMetrics fm = g.getFontMetrics();
	if (isTitleShowing()) {
	    int strW = fm.stringWidth(l_label);
	    int xPos = (width - strW) >> 1;
	    g.setColor(getBackground());
	    g.fillRect(1, 2, width-4, l_titleOffset);
	    draw3DRect(g, 1, l_titleOffset, width-3, 1, false);
	    g.setColor(getForeground());
	    g.drawString(l_label, xPos, 2 + fm.getMaxAscent());
	}

	XMenuItemPeer[] localItems = copyItems();
	int nitems = localItems.length;
	for (int i = 0 ; i < nitems ; i++) {
	    localItems[i].paint(g, getTop(i), getBot(i), width, getShortcutOffset(), (i == selected));
	}
    }

    void repaintMenuItem(XMenuItemPeer menuItemPeer) {
        if (menuItemPeer == null) {
            return;
        }
        XMenuItemPeer[] localItems = copyItems();
        int index = -1;
        for (int i = 0; i < localItems.length; i++) {
            if (localItems[i].equals(menuItemPeer)) {
                index = i;
                break;
            }
        }
        if (index >= 0) {
            Graphics g = getGraphics();
            try {
                localItems[index].paint(g, getTop(index), getBot(index), width, getShortcutOffset(), index == selected);
            } finally {
                 g.dispose();
            }
        }
    }

    /**
     * Select an item.
     */
    void select(int i) {
	if (log.isLoggable(Level.FINEST)) log.finest("Selecting item " + i);
        int l_selected = getSelected();
	if (i == l_selected) {
	    return;
	}
	Graphics g = getGraphics();
	try {
	    XMenuItemPeer[] localItems = copyItems();
	    int l_shortcutOffset = getShortcutOffset();
	    g.setFont(getFont());
	    FontMetrics fm = g.getFontMetrics();
	    if (l_selected >= 0) {
		int old = l_selected;
                setSelected(-1);
		if (old < localItems.length) {
		    localItems[old].paint(g, getTop(old), getBot(old), width, l_shortcutOffset, false);
		}
	    } 
	    if (i < 0) {
		// we only deselect
		return; 
	    } 
	    XMenuItemPeer mi = getItem(i);
	    if ((mi != null) && mi.isEnabled() && mi.getLabel() != null && !mi.getLabel().equals("-")) {
		if (i < localItems.length) {
		    localItems[i].paint(g, getTop(i), getBot(i), width, l_shortcutOffset, true);
		}
                setSelected(i);
	    } else {
                setSelected(-1);
	    } 
	}
	finally {
	    g.dispose();
	}
    }

    boolean contains(int x, int y, int pad) {
	return (x >= pad) && (x < getWidth() - pad)
	    && (y >= pad) && (y < getHeight() - pad);
    }
                
    boolean cascadeContains(XMenuWindow win, Point p, int pad) {
        XMenuPeer l_menu = getMenu();
	Point npt = new Point(p.x, p.y);
	npt = toLocal(win.toGlobal(npt));
	if (l_menu == null) {
	    return contains(npt.x, npt.y, pad);
	} else {
	    return contains(npt.x, npt.y, pad) || l_menu.cascadeContains(this, npt, pad);
	}
    }

    boolean cascadeContains(Point p, int pad) {
        XMenuPeer l_menu = getMenu();
	if (l_menu == null) {
	    return contains(p.x, p.y, pad);
	} else {
	    Point npt = new Point(p.x, p.y);
	    npt = l_menu.toLocal(toGlobal(npt));
	    return contains(p.x, p.y, pad) || l_menu.cascadeContains(npt, pad);
	}
    }

    boolean lastMenuContains(XMenuWindow win, Point p, int pad) {
        XMenuPeer l_menu = getMenu();
	Point npt = new Point(p.x, p.y);
	npt = toLocal(win.toGlobal(npt));
	if (l_menu == null) {
	    return contains(npt.x, npt.y, pad);
	} else {
	    return l_menu.lastMenuContains(this, npt, pad);
	}
    }

    public boolean hasMenu() {
        return (menu != null);
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
		}
		else {
		    l_menu.setPosted(true);
		}
	    }
	    return return_menu;
	}
	else if (l_menu == null) {
	    return null;
	} else {
	    npt = l_menu.toLocal(win.toGlobal(npt));
	    return l_menu.doPosting(this, npt, pad);
	}
    }

    int getItemIndex(MouseEvent mouseEvent) {
	int my = mouseEvent.getY();
	for (int i = 0; i < getItems().size(); i++) {
	    if ((my >= getTop(i)) && (my <= getBot(i))) {
		return i;
	    }
	}
	return -1;
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
        if (isDisposed()) {
            return;
        } 
        XMenuPeer l_menu = getMenu();
        final int l_width = getWidth();
        int l_height = getHeight();
	switch (mouseEvent.getID()) {
	    case MouseEvent.MOUSE_DRAGGED:
	    case MouseEvent.MOUSE_PRESSED:
		final int item_i = getItemIndex(mouseEvent);
		if (l_menu != null) {
		    pt = mouseEvent.getPoint();
		    int mx = pt.x;
		    int my = pt.y;
		    pt = l_menu.toLocal(toGlobal( pt ));
		    if (this.contains(mx, my, 5) && (item_i != popup_i)) {
			if (log.isLoggable(Level.FINE)) log.fine("Popping down child menu " + menu);
			l_menu.popdown(mouseEvent, false);
                        setMenu(null);
		    } else {
			MouseEvent me = makeMouseEvent(mouseEvent, pt.x, pt.y);
			l_menu.handleJavaMouseEvent(me);
			return;
		    }
		}
		if ((item_i == -1) || 
		    (mouseEvent.getX() < 0) || (mouseEvent.getX() > l_width)
		    || (mouseEvent.getY() < 0) || (mouseEvent.getY() > l_height)) {
		    select(-1);
		    return;
		}

		select(item_i);
                final int l_selected = getSelected();

                if (l_selected >= 0) {
		    XMenuItemPeer mi = getItem(l_selected);
		    if ((mi != null) && (mi instanceof XMenuPeer) && (mi.isEnabled()) && (l_menu == null)) {
			if (((mouseEvent.getID() == MouseEvent.MOUSE_DRAGGED) &&
			       (mouseEvent.getX() > (l_width - 20))) ||
			      (mouseEvent.getID() == MouseEvent.MOUSE_PRESSED)) {
			    setMenu((XMenuPeer)mi);
			    pt.x = l_width - 2;
			    pt.y = getTop(l_selected);
			    pt=toGlobal(pt);
			    popup_i = item_i;
			    getMenu().popup(pt.x, pt.y);
			    if (log.isLoggable(Level.FINE)) log.fine("Popping up child menu " + getMenu());
			} else if ((mouseEvent.getID() == MouseEvent.MOUSE_DRAGGED) && (l_selected >= 0) && (popupTimer == null)) {
			    AbstractAction popupAction = new AbstractAction() {
				public void actionPerformed(ActionEvent e) {
				    popupTimer.stop();
                                    popupTimer = null;
				    if (l_selected == getSelected()) {
					XMenuItemPeer mi = getItem(l_selected);

					if ((mi != null) && (mi instanceof XMenuPeer) && (mi.isEnabled()) && (getMenu() == null)) {
					    setMenu((XMenuPeer)mi);
					    pt.x = l_width - 2;
					    pt.y = getTop(l_selected);
					    pt=toGlobal(pt);
					    popup_i = l_selected;
					    getMenu().popup(pt.x, pt.y);
					    if (log.isLoggable(Level.FINE)) log.fine("Popping up(from drag) child menu " + getMenu());
					}
				    }
				}
			    };
			    popupTimer = new Timer(200, popupAction);
			    popupTimer.setRepeats(false);
			    popupTimer.start();

			}   
		    }
                }   
	}        
    }    

    public void addSeparator() {
    }

    public void addItem(MenuItem item) {
	XMenuItemPeer mpeer = (XMenuItemPeer)getPeer(item);
	mpeer.setMenuPeer(this);
	getItems().add(mpeer);
	updateMenuWindow();
    }

    public void delItem(int index) {
	synchronized(getItems()) {
	    if (index < getItems().size()) {
		XMenuItemPeer mi = (XMenuItemPeer) getItems().elementAt(index);
		mi.setVisible(false);
		getItems().removeElementAt(index);
		setSelected(-1);
	    }
        }
	updateMenuWindow();
    }

    private void updateMenuWindow() {
	if (!isVisible() || !isCreated()) {
            return;
        }
	Rectangle rect = getBounds();
	updateMenu(rect.x, rect.y);
	repaint();
    }

    public void setEnabled(boolean b) {
        boolean wasEnabled = isEnabled();
        super.setEnabled(b);
        if ((wasEnabled != b) && (parentMenu != null) && (parentMenu.isVisible())) {
            if (parentMenu instanceof XMenuBarPeer) {
                if (b == false) {
                    XMenuBarPeer pmb = (XMenuBarPeer)parentMenu;
                    int menuSelected = pmb.getMenuSelected();
                    if (menuSelected >= 0) {
                        XMenuPeer selectedMenu = pmb.getMenu(menuSelected);
                        if (selectedMenu == this) {
                            pmb.setMenuSelected(-1);
                        }
                    }
                }
                parentMenu.repaint();
            } else if (parentMenu instanceof XMenuPeer) {
                if (b == false) {
                    XMenuPeer pm = (XMenuPeer)parentMenu;
                    int itemSelected = pm.getSelected();
                    if (itemSelected >= 0) {
                        XMenuItemPeer selectedItem = pm.getItem(itemSelected);
                        if (selectedItem == this) {
                            pm.setSelected(-1);
                            pm.repaintMenuItem(this);
                        }
                    }
                }
            }
        }
    }

    public void enable() {
        setEnabled(true);
    }

    public void disable() {
        setEnabled(false); 
    }

    public void setLabel(String label) {
        super.setLabel(label);
        if ((parentMenu != null) && (parentMenu.isVisible())) {
            if (parentMenu instanceof XMenuBarPeer) {
                XMenuBarPeer pmb = (XMenuBarPeer)parentMenu;
                parentMenu.repaint();
            }
        }
    }

    public void dispose() {
        setDisposed(true);
        setCreated(false);
	super.dispose();
    }

    void setPosted(boolean posted) {
	this.posted = posted;
        XMenuPeer l_menu = getMenu();
	if (l_menu != null) {
	    l_menu.setPosted(posted);
	}
    }

    boolean isPosted() {
	XMenuPeer l_menu = getMenu();
	if (l_menu == null) {
	    return posted;
	} else {
	    return l_menu.isPosted();
	}
    }

    int getUpSelected() {
        XMenuItemPeer[] localItems = copyItems();
        int nitems = localItems.length;
        if (nitems == 0) {
            return -1;
        }
        XMenuItemPeer mi;
        int sel = getSelected();
        if (sel < 0) {
            return -1;
        }
        do {
	    sel = (sel==0) ? nitems-1 : (sel-1) % nitems;
	    mi = localItems[sel];
	} while ((!mi.isEnabled()) || (mi.getLabel() != null && mi.getLabel().equals("-")));
	return sel;
    }

    int getDownSelected() {
        XMenuItemPeer[] localItems = copyItems();
        int nitems = localItems.length;
        if (nitems == 0) {
            return -1;
        }
        XMenuItemPeer mi;
        int sel = getSelected();
	do {
	    sel = (sel+1) % nitems;
	    mi = localItems[sel];
	} while ((!mi.isEnabled()) || (mi.getLabel() != null && mi.getLabel().equals("-")));
	return sel;
    }

    boolean handleKeyPress1(int keyCode, XMenuPeer parent) {
        XMenuPeer l_menu = getMenu();
    //  System.out.println("XMenuPeer:handleKeyPress1:keyCode: " + KeyEvent.getKeyText(keyCode));
	if (l_menu != null) {
	    return l_menu.handleKeyPress1(keyCode, this);
	}
	switch(keyCode)
	{
	  case KeyEvent.VK_UP:
	      select(getUpSelected());
	      break;
	  case KeyEvent.VK_DOWN:
	      select(getDownSelected());
	      return true;
	  case KeyEvent.VK_LEFT:
	      if (parent != null) {
		  setPosted(false);
		  popdown(null, false);
		  parent.setMenu(null);
		  return true;
	      }               
	      break;
	  case KeyEvent.VK_RIGHT:
	  case KeyEvent.VK_SPACE:
	  case KeyEvent.VK_ENTER:
              int l_selected = getSelected();
	      if (l_selected >= 0) {
		  XMenuItemPeer mi = getItem(l_selected);
		  if ((mi != null) && (mi instanceof XMenuPeer) && (mi.isEnabled()) && (l_menu == null)) {
                      setMenu((XMenuPeer)mi);
                      l_menu = getMenu();
		      pt.x = getWidth() - 2;
		      pt.y = getTop(l_selected);
		      pt=toGlobal(pt);
		      l_menu.popup(pt.x, pt.y);
		      l_menu.select(0);
		      l_menu.setPosted(true);
		      return true;
		  }
	      }
	      break;
	  default:
	      break;
	}
	return false;
    }

    int getXIndent() {
        return xIndent;
    }

    void setXIndent(int x) {
        xIndent = x;
    }

    int getItemCount() {
        return getItems().size();
    }

    XMenuItemPeer getItem(int index) {
	synchronized(getItems()) {
	    if (index < getItems().size()) {
		return (XMenuItemPeer) getItems().elementAt(index);
	    }
	    else {
		return null;
	    } 
        }
    }

    Vector getTargetItems() {
        try {
            return (Vector)f_items.get(menuTarget);
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
            return null;
        }
    }

    void setCreated(boolean c) {
        created = c;
    }

    boolean isCreated() {
        return created;
    }

    XMenuPeer getMenu() {
        return menu;
    }

    void setMenu(XMenuPeer m) {
        menu = m;
    }

    int getSelected() {
        return selected;
    }

    void setSelected(int s) {
        selected = s;
    }

    int getTitleOffset() {
        return titleOffset;
    }

    void setTitleOffset(int t) {
        titleOffset = t;
    }

    int getShortcutOffset() {
        return shortcutOffset;
    }

    void setShortcutOffset(int s) {
        shortcutOffset = s;
    }

    Vector getItems() {
        return items;
    }

    void setItems(Vector i) {
        items = i;
    }

    void setVisible(boolean b) {
        xSetVisible(b);
    }

    boolean isViewable() {
        return viewable;
    }

    void setViewable(boolean b) {
        viewable = b;
    }


}
