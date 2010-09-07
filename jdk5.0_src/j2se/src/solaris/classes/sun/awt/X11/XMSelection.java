/*
 * @(#)XMSelection.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 /* 
   * This code is ported to XAWT from MAWT based on awt_mgrsel.c 
   * code written originally by Valeriy Ushakov 
   * Author : Bino George 
   */ 

package sun.awt.X11;

import sun.misc.Unsafe;
import java.util.*;
import java.awt.*;
import sun.awt.XSettings;
import sun.awt.*;
import java.util.logging.*;


public class  XMSelection { 

    /*
     * A method for a subsytem to express its interest in a certain
     * manager selection.
     *
     * If owner changes, the ownerChanged of the XMSelectionListener 
     * will be called with the screen
     * number and the new owning window when onwership is established, or
     * None if the owner is gone.
     *
     * Events in extra_mask are selected for on owning windows (exsiting
     * ones and on new owners when established) and otherEvent of the
     * XMWSelectionListener will be called with the screen number and an event.
     * 
     * The function returns an array of current owners.  The size of the
     * array is ScreenCount(awt_display).  The array is "owned" by this
     * module and should be considered by the caller as read-only.
     */


    private static Logger log = Logger.getLogger("sun.awt.X11.XMSelection");
    /* Name of the selection */
    String selectionName;

    /* list of listeners to be called for events */
    Vector listeners;

    /* X atom array (one per screen) for this selection */ 
    XAtom atoms[];

    /* Window ids of selection owners */
    long owners[];

    /* event mask to set */
    long eventMask;

    static int numScreens;

    static XAtom XA_MANAGER;

    static HashMap selectionMap;

    static {
        long display = XToolkit.getDisplay(); 
        synchronized(XToolkit.getAWTLock()) {    
            numScreens = XlibWrapper.ScreenCount(display);
        }
        XA_MANAGER = XAtom.get("MANAGER");
        for (int screen = 0; screen < numScreens ; screen ++) {
            initScreen(display,screen); 
        }

        selectionMap = new HashMap();
    }

    static void initScreen(long display, final int screen) {
        synchronized(XToolkit.getAWTLock()) {    

            long root = XlibWrapper.RootWindow(display,screen);
            XlibWrapper.XSelectInput(display, root, XlibWrapper.StructureNotifyMask);
            XToolkit.addEventDispatcher(root, 
                    new XEventDispatcher() {
                        public void dispatchEvent(IXAnyEvent ev) {
                                processRootEvent((XAnyEvent)ev, screen);
                            }
                        });

        }
    }


    public int getNumberOfScreens() {
        return numScreens;
    }

    void select(long extra_mask) {
        eventMask = extra_mask;
        for (int screen = 0; screen < numScreens ; screen ++) {
            selectPerScreen(screen,extra_mask);
        }
    }

    void resetOwner(long owner, final int screen) {
        synchronized(XToolkit.getAWTLock()) {
            long display = XToolkit.getDisplay(); 
            synchronized(this) {
                setOwner(owner, screen); 
                if (log.isLoggable(Level.FINE)) log.fine("New Selection Owner for screen " + screen + " = " + owner );
                XlibWrapper.XSelectInput(display, owner, XlibWrapper.StructureNotifyMask | eventMask);
                XToolkit.addEventDispatcher(owner, 
                        new XEventDispatcher() {
                            public void dispatchEvent(IXAnyEvent ev) {
                                dispatchSelectionEvent(ev, screen);
                            }
                        });

            }
        } 

    } 

    void selectPerScreen(final int screen, long extra_mask) {

        synchronized(XToolkit.getAWTLock()) { 
            try {  
                long display = XToolkit.getDisplay(); 
                if (log.isLoggable(Level.FINE)) log.fine("Grabbing XServer");
                XlibWrapper.XGrabServer(display);

                synchronized(this) {
                    String selection_name = getName()+"_S"+screen;
                    if (log.isLoggable(Level.FINE)) log.fine("Screen = " + screen + " selection name = " + selection_name);
                    XAtom atom = XAtom.get(selection_name);
                    selectionMap.put(new Long(atom.getAtom()),this); // add mapping from atom to the instance of XMSelection
                    setAtom(atom,screen);
                    long owner = XlibWrapper.XGetSelectionOwner(display, atom.getAtom());
                    if (owner != 0) {
                        setOwner(owner, screen); 
                        if (log.isLoggable(Level.FINE)) log.fine("Selection Owner for screen " + screen + " = " + owner );
                        XlibWrapper.XSelectInput(display, owner, XlibWrapper.StructureNotifyMask | extra_mask);
                        XToolkit.addEventDispatcher(owner, 
                                new XEventDispatcher() {
                                        public void dispatchEvent(IXAnyEvent ev) {
                                            dispatchSelectionEvent(ev, screen);
                                        }
                                    });
                    }
                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }
            finally {
                if (log.isLoggable(Level.FINE)) log.fine("UnGrabbing XServer");
                XlibWrapper.XUngrabServer(XToolkit.getDisplay());
            }

        }

    }


    static boolean processClientMessage(XClientMessageEvent xce, int screen) {
        if (xce.get_message_type() == XA_MANAGER.getAtom()) {
            if (log.isLoggable(Level.FINE)) log.fine("client messags = " + xce);
            long timestamp = xce.get_data(0);    
            long atom = xce.get_data(1);    
            long owner = xce.get_data(2);
            long data = xce.get_data(3);

            XMSelection sel = getInstance(atom);
            if (sel != null) {
                sel.resetOwner(owner,screen);    
                sel.dispatchOwnerChangedEvent(xce,screen,owner,data, timestamp);
            }
        }
        return false;
    }


    static  boolean processRootEvent(XAnyEvent ev, int screen) {
        switch (ev.get_type()) {
            case (int)XlibWrapper.ClientMessage: {
                                                     XClientMessageEvent xclient = new XClientMessageEvent(ev.pData);
                                                     return processClientMessage(xclient, screen);
                                                 }
        }

        return false; 

    }


    static XMSelection getInstance(long selection) {
        return (XMSelection) selectionMap.get(new Long(selection));
    }


    /* 
     * Default constructor specifies PropertyChangeMask as well
     */

    public XMSelection (String selname) {
        this(selname, XlibWrapper.PropertyChangeMask);  
    }

   
   /*
    * Some users may not need to know about selection changes,
    * just owner ship changes, They would specify a zero extra mask.
    */

    public XMSelection (String selname, long extraMask) {

        synchronized (this) {
            selectionName = selname;
            atoms = new XAtom[getNumberOfScreens()];
            owners = new long[getNumberOfScreens()];
        }
        select(extraMask);  
    }

 

    public synchronized void addSelectionListener(XMSelectionListener listener) {
        if (listeners == null) {
            listeners = new Vector(); 
        }
        listeners.add(listener);
    }

    public synchronized void removeSelectionListener(XMSelectionListener listener) {
        if (listeners == null) {
            listeners.remove(listener);
        }
    }

    synchronized Collection getListeners() {
        return listeners; 
    }

    synchronized XAtom getAtom(int screen) {
        if (atoms != null) {
            return atoms[screen];
        }
        return null;
    }

    synchronized void setAtom(XAtom a, int screen) {
        if (atoms != null) {
            atoms[screen] = a;
        }
    }

    synchronized long getOwner(int screen) {
        if (owners != null) {
            return owners[screen];
        }
        return 0;
    }

    synchronized void setOwner(long owner, int screen) {
        if (owners != null) {
            owners[screen] = owner;
        }
    }

    synchronized String getName() {
        return selectionName; 
    }


    synchronized void dispatchSelectionChanged( XPropertyEvent ev, int screen) {
        if (log.isLoggable(Level.FINE)) log.fine("Selection Changed : Screen = " + screen + "Event =" + ev);
        if (listeners != null) {
            Iterator iter = listeners.iterator();
            while (iter.hasNext()) {
                XMSelectionListener disp = (XMSelectionListener) iter.next();
                disp.selectionChanged(screen, this, ev.get_window(), ev);
            }
        }
    }

    synchronized void dispatchOwnerDeath(XDestroyWindowEvent de, int screen) {
        if (log.isLoggable(Level.FINE)) log.fine("Owner dead : Screen = " + screen + "Event =" + de);
        if (listeners != null) {
            Iterator iter = listeners.iterator();
            while (iter.hasNext()) {
                XMSelectionListener disp = (XMSelectionListener) iter.next();
                disp.ownerDeath(screen, this, de.get_window());

            }
        }
    }

    void dispatchSelectionEvent(IXAnyEvent ev, int screen) {
        if (log.isLoggable(Level.FINE)) log.fine("Event =" + ev);
        if (ev.get_type() == XlibWrapper.DestroyNotify) {
            XDestroyWindowEvent de = new XDestroyWindowEvent(ev.getPData());
            dispatchOwnerDeath( de, screen);  
        }
        else if (ev.get_type() == XlibWrapper.PropertyNotify)  {
            XPropertyEvent xpe = new XPropertyEvent(ev.getPData());
            dispatchSelectionChanged( xpe, screen);  
        }
    }


    synchronized void dispatchOwnerChangedEvent(IXAnyEvent ev, int screen, long owner, long data, long timestamp) {
        if (listeners != null) {
            Iterator iter = listeners.iterator();
            while (iter.hasNext()) {
                XMSelectionListener disp = (XMSelectionListener) iter.next();
                disp.ownerChanged(screen,this, owner, data, timestamp);
            }
        }
    }


}


