/*
 * @(#)XWM.java	1.41 04/04/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/**
 * Ported from awt_wm.c, SCCS v1.11, author Valeriy Ushakov
 * Author: Denis Mikhalkin
 */
package sun.awt.X11;

import sun.misc.Unsafe;
import java.util.regex.*;
import java.awt.Frame;
import java.awt.Rectangle;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;
import java.awt.Insets;

/**
 * Class incapsulating knowledge about window managers in general
 * Descendants should provide some information about specific window manager.
 */
class XWM implements MWMConstants, XUtilConstants {

    private final static Logger log = Logger.getLogger("sun.awt.X11.XWM");
    private final static Logger insLog = Logger.getLogger("sun.awt.X11.insets.XWM");
    private final static Logger stateLog = Logger.getLogger("sun.awt.X11.states.XWM");

    static final XAtom XA_MWM_HINTS = new XAtom();
    
    private static Unsafe unsafe = XlibWrapper.unsafe;


/* Good old ICCCM */
    static XAtom XA_WM_STATE = new XAtom();


    XAtom XA_UTF8_STRING = XAtom.get("UTF8_STRING");    /* like STRING but encoding is UTF-8 */

/* Currently we only care about max_v and max_h in _NET_WM_STATE */
    final static int AWT_NET_N_KNOWN_STATES=2;

/* Enlightenment */
    static XAtom XA_E_FRAME_SIZE = new XAtom();

/* KWin (KDE2) */
    static XAtom XA_KDE_NET_WM_FRAME_STRUT = new XAtom();

/* KWM (KDE 1.x) OBSOLETE??? */
    static XAtom XA_KWM_WIN_ICONIFIED = new XAtom();
    static XAtom XA_KWM_WIN_MAXIMIZED = new XAtom();

/* OpenLook */
    static XAtom XA_OL_DECOR_DEL = new XAtom();
    static XAtom XA_OL_DECOR_HEADER = new XAtom();
    static XAtom XA_OL_DECOR_RESIZE = new XAtom();
    static XAtom XA_OL_DECOR_PIN = new XAtom();
    static XAtom XA_OL_DECOR_CLOSE = new XAtom();
    
    final static int
    UNDETERMINED_WM = 1,
        NO_WM = 2,
        OTHER_WM = 3,
        OPENLOOK_WM = 4,
        MOTIF_WM = 5,
        CDE_WM = 6,
        ENLIGHTEN_WM = 7,
        KDE2_WM = 8,
        SAWFISH_WM = 9,
        ICE_WM = 10,
        METACITY_WM = 11;
    public String toString() {
        switch  (WMID) {
          case NO_WM:
              return "NO WM";
          case OTHER_WM:
              return "Other WM";
          case OPENLOOK_WM:
              return "OPENLOOK";
          case MOTIF_WM:
              return "MWM";
          case CDE_WM:
              return "DTWM";
          case ENLIGHTEN_WM:
              return "Enlightenment";
          case KDE2_WM:
              return "KWM2";
          case SAWFISH_WM:
              return "Sawfish";
          case ICE_WM:
              return "IceWM";
          case METACITY_WM:
              return "Metacity";              
          case UNDETERMINED_WM:
          default:
              return "Undetermined WM";
        }
    }


    int WMID;
    static final Insets zeroInsets = new Insets(0, 0, 0, 0);
    static final Insets defaultInsets = new Insets(25, 5, 5, 5);

    XWM(int WMID) {
        this.WMID = WMID;
        initializeProtocols();
        if (log.isLoggable(Level.FINE)) log.fine("Window manager: " + toString());
    }
    int getID() {
        return WMID;
    }
    

    static Insets normalize(Insets insets) {
        if (insets.top > 64 || insets.top < 0) {
            insets.top = 28;
        }
        if (insets.left > 32 || insets.left < 0) {
            insets.left = 6;
        }
        if (insets.right > 32 || insets.right < 0) {
            insets.right = 6;
        }
        if (insets.bottom > 32 || insets.bottom < 0) {
            insets.bottom = 6;
        }
        return insets;
    }

    static XNETProtocol net_protocol = null;
    static boolean isNetWMName(String name) {
        if (net_protocol != null) {
            return net_protocol.isWMName(name);
        } else {
            return false;
        }
    }

    
    static void initAtoms() {
        final Object[][] atomInitList ={        
            { XA_WM_STATE,                      "WM_STATE"                           },
                
            { XA_KDE_NET_WM_FRAME_STRUT,    "_KDE_NET_WM_FRAME_STRUT"       },

            { XA_E_FRAME_SIZE,              "_E_FRAME_SIZE"                 },

            { XA_KWM_WIN_ICONIFIED,          "KWM_WIN_ICONIFIED"             },
            { XA_KWM_WIN_MAXIMIZED,          "KWM_WIN_MAXIMIZED"             },

            { XA_OL_DECOR_DEL,               "_OL_DECOR_DEL"                 },
            { XA_OL_DECOR_HEADER,            "_OL_DECOR_HEADER"              },
            { XA_OL_DECOR_RESIZE,            "_OL_DECOR_RESIZE"              },
            { XA_OL_DECOR_PIN,               "_OL_DECOR_PIN"                 },
            { XA_OL_DECOR_CLOSE,             "_OL_DECOR_CLOSE"               },
            { XA_MWM_HINTS,                  "_MOTIF_WM_HINTS" }            
        };
        
        String[] names = new String[atomInitList.length];
        for (int index = 0; index < names.length; index++) {
            names[index] = (String)atomInitList[index][1];
        }
        
        int atomSize = XAtom.getAtomSize();
        long atoms = unsafe.allocateMemory(names.length*atomSize);
        try {
            XToolkit.awtLock();
            int status = XlibWrapper.XInternAtoms(XToolkit.getDisplay(), names, false, atoms);
            if (status == 0) {
                return;
            }            
            for (int atom = 0, atomPtr = 0; atom < names.length; atom++, atomPtr += atomSize) {
                ((XAtom)(atomInitList[atom][0])).setValues(XToolkit.getDisplay(), names[atom], XAtom.getAtom(atoms + atomPtr));
            }            
        } finally {
            XToolkit.awtUnlock();
            unsafe.freeMemory(atoms);
        }        
    }


    
    static XAtom XA_ENLIGHTENMENT_COMMS = new XAtom("ENLIGHTENMENT_COMMS", false);
    /*
     * Helper function for isEnlightenment().
     * Enlightenment uses STRING property for its comms window id.  Gaaa!
     * The property is ENLIGHTENMENT_COMMS, STRING/8 and the string format
     * is "WINID %8x".  Gee, I haven't been using scanf for *ages*... :-)
     */
    static long getECommsWindowIDProperty(long window) {        

        if (!XA_ENLIGHTENMENT_COMMS.isInterned()) {
            return 0;
        }

        WindowPropertyGetter getter = 
            new WindowPropertyGetter(window, XA_ENLIGHTENMENT_COMMS, 0, 14, false, 
                                     XAtom.XA_STRING);
        try {
            int status = getter.execute(XToolkit.IgnoreBadWindowHandler);
            if (status != XlibWrapper.Success || getter.getData() == 0) {
                return 0;
            }

            if (getter.getActualType() != XAtom.XA_STRING 
                || getter.getActualFormat() != 8
                || getter.getNumberOfItems() != 14 || getter.getBytesAfter() != 0) 
            {
                return 0;
            }
            
            // Convert data to String, ASCII
            byte[] bytes = XlibWrapper.getStringBytes(getter.getData());
            String id = new String(bytes);

            log.finer("ENLIGHTENMENT_COMMS is " + id);

            // Parse WINID
            Pattern winIdPat = Pattern.compile("WINID\\s+(\\p{XDigit}{0,8})");
            try {
                Matcher match = winIdPat.matcher(id);
                if (match.matches()) {
                    log.finest("Match group count: " + match.groupCount());
                    String longId = match.group(1);
                    log.finest("Match group 1 " + longId);
                    long winid = Long.parseLong(longId, 16);
                    log.finer("Enlightenment communication window " + winid);
                    return winid;
                } else {
                    log.finer("ENLIGHTENMENT_COMMS has wrong format");
                    return 0;
                }
            } catch (Exception e) {
                if (log.isLoggable(Level.FINER)) {
                    e.printStackTrace();
                }
                return 0;
            }
        } finally {
            getter.dispose();
        }
    }

    /*
     * Is Enlightenment WM running?  Congruent to awt_wm_checkAnchor, but
     * uses STRING property peculiar to Enlightenment.
     */
    static boolean isEnlightenment() {
        
        long root_xref = getECommsWindowIDProperty(XToolkit.getDefaultRootWindow());
        if (root_xref == 0) {
            return false;
        }
        
        long self_xref = getECommsWindowIDProperty(root_xref);
        if (self_xref != root_xref) {
            return false;
        }

        return true;
    }

    /*
     * Is CDE running?
     *
     * XXX: This is hairy...  CDE is MWM as well.  It seems we simply test
     * for default setup and will be bitten if user changes things...
     *
     * Check for _DT_SM_WINDOW_INFO(_DT_SM_WINDOW_INFO) on root.  Take the
     * second element of the property and check for presence of
     * _DT_SM_STATE_INFO(_DT_SM_STATE_INFO) on that window.
     *
     * XXX: Any header that defines this structures???     
     */
    static final XAtom XA_DT_SM_WINDOW_INFO = new XAtom("_DT_SM_WINDOW_INFO", false);
    static final XAtom XA_DT_SM_STATE_INFO = new XAtom("_DT_SM_STATE_INFO", false);
    static boolean isCDE() {

        if (!XA_DT_SM_WINDOW_INFO.isInterned()) {
            log.log(Level.FINER, "{0} is not interned", new Object[] {XA_DT_SM_WINDOW_INFO});
            return false;
        }

        WindowPropertyGetter getter = 
            new WindowPropertyGetter(XToolkit.getDefaultRootWindow(), 
                                     XA_DT_SM_WINDOW_INFO, 0, 2,
                                     false, XA_DT_SM_WINDOW_INFO);
        try {
            int status = getter.execute();
            if (status != XlibWrapper.Success || getter.getData() == 0) {
                log.finer("Getting of _DT_SM_WINDOW_INFO is not successfull");
                return false;
            }
            if (getter.getActualType() != XA_DT_SM_WINDOW_INFO.getAtom() 
                || getter.getActualFormat() != 32
                || getter.getNumberOfItems() != 2 || getter.getBytesAfter() != 0) 
            {
                log.finer("Wrong format of _DT_SM_WINDOW_INFO");
                return false;
            }
        
            long wmwin = Native.getWindow(getter.getData(), 1); //unsafe.getInt(getter.getData()+4);

            if (wmwin == 0) {
                log.fine("WARNING: DT_SM_WINDOW_INFO exists but returns zero windows");
                return false;
            }
                
            /* Now check that this window has _DT_SM_STATE_INFO (ignore contents) */
            if (!XA_DT_SM_STATE_INFO.isInterned()) {
                log.log(Level.FINER, "{0} is not interned", new Object[] {XA_DT_SM_STATE_INFO});            
                return false;
            }
            WindowPropertyGetter getter2 = 
                new WindowPropertyGetter(wmwin, XA_DT_SM_STATE_INFO, 0, 1,
                                         false, XA_DT_SM_STATE_INFO);
            try {
                status = getter2.execute(XToolkit.IgnoreBadWindowHandler);
        
        
                if (status != XlibWrapper.Success || getter2.getData() == 0) {
                    log.finer("Getting of _DT_SM_STATE_INFO is not successfull");
                    return false;
                }
                if (getter2.getActualType() != XA_DT_SM_STATE_INFO.getAtom() 
                    || getter2.getActualFormat() != 32) 
                {
                    log.finer("Wrong format of _DT_SM_STATE_INFO");
                    return false;
                }

                return true;                
            } finally {
                getter2.dispose();
            }
        } finally {
            getter.dispose();
        }
    }

    /*
     * Is MWM running?  (Note that CDE will test positive as well).
     * 
     * Check for _MOTIF_WM_INFO(_MOTIF_WM_INFO) on root.  Take the
     * second element of the property and check for presence of
     * _DT_SM_STATE_INFO(_DT_SM_STATE_INFO) on that window.
     */
    static final XAtom XA_MOTIF_WM_INFO = new XAtom("_MOTIF_WM_INFO", false);
    static final XAtom XA_DT_WORKSPACE_CURRENT = new XAtom("_DT_WORKSPACE_CURRENT", false);
    static boolean isMotif() {

        if (!(XA_MOTIF_WM_INFO.isInterned() && XA_DT_WORKSPACE_CURRENT.isInterned()) ) {
            return false;
        }

        WindowPropertyGetter getter = 
            new WindowPropertyGetter(XToolkit.getDefaultRootWindow(), 
                                     XA_MOTIF_WM_INFO, 0, 
                                     PROP_MOTIF_WM_INFO_ELEMENTS,
                                     false, XA_MOTIF_WM_INFO);
        try {
            int status = getter.execute();

            if (status != XlibWrapper.Success || getter.getData() == 0) {
                return false;
            }

            if (getter.getActualType() != XA_MOTIF_WM_INFO.getAtom() 
                || getter.getActualFormat() != 32
                || getter.getNumberOfItems() != PROP_MOTIF_WM_INFO_ELEMENTS 
                || getter.getBytesAfter() != 0) 
            {
                return false;
            }
        
            long wmwin = Native.getLong(getter.getData(), 1);
                
            /* Now check that this window has _DT_WORKSPACE_CURRENT */
            XAtom[] curws = XA_DT_WORKSPACE_CURRENT.getAtomListProperty(wmwin);
            if (curws.length == 0) {
                return false;
            }
            return true;
        } finally {
            getter.dispose();
        }
    }

    /*
     * Is Sawfish running?
     */    
    static boolean isSawfish() {
        return isNetWMName("Sawfish");
    }
    
    /*
     * Is KDE2 (KWin) running?
     */
    static boolean isKDE2() {
        return isNetWMName("KWin");
    }

    /*
     * Is Metacity running?
     */
    static boolean isMetacity() {
        return isNetWMName("Metacity");
//         || (
//             XA_NET_SUPPORTING_WM_CHECK.
//             getIntProperty(XToolkit.getDefaultRootWindow(), XA_NET_SUPPORTING_WM_CHECK.
//                            getIntProperty(XToolkit.getDefaultRootWindow(), XAtom.XA_CARDINAL)) == 0);
    }

    /*
     * Temporary error handler that ensures that we know if
     * XChangeProperty succeeded or not.
     */
    static XToolkit.XErrorHandler VerifyChangePropertyHandler = new XToolkit.XErrorHandler() {
            public int handleError(long display, XErrorEvent err) {
                XToolkit.XERROR_SAVE(err);
                if (err.get_request_code() == XlibWrapper.X_ChangeProperty) {
                    return 0;
                } else {
                    return XToolkit.SAVED_ERROR_HANDLER(display, err);
                }
            }
        };

    /*
     * Prepare IceWM check.
     *
     * The only way to detect IceWM, seems to be by setting
     * _ICEWM_WINOPTHINT(_ICEWM_WINOPTHINT/8) on root and checking if it
     * was immediately deleted by IceWM.
     *
     * But messing with PropertyNotify here is way too much trouble, so
     * approximate the check by setting the property in this function and
     * checking if it still exists later on.
     * 
     * Gaa, dirty dances...
     */
    static final XAtom XA_ICEWM_WINOPTHINT = new XAtom("_ICEWM_WINOPTHINT", false);
    static final char opt[] = {
        'A','W','T','_','I','C','E','W','M','_','T','E','S','T','\0',
        'a','l','l','W','o','r','k','s','p','a','c','e','s','\0',
        '0','\0'
    };
    static boolean prepareIsIceWM() {
        /*
         * Choose something innocuous: "AWT_ICEWM_TEST allWorkspaces 0".
         * IceWM expects "class\0option\0arg\0" with zero bytes as delimiters.
         */
        
        if (!XA_ICEWM_WINOPTHINT.isInterned()) {
            log.log(Level.FINER, "{0} is not interned", new Object[] {XA_ICEWM_WINOPTHINT});
            return false;
        }

        try {
            XToolkit.awtLock();
            XToolkit.WITH_XERROR_HANDLER(VerifyChangePropertyHandler);
            XlibWrapper.XChangePropertyS(XToolkit.getDisplay(), XToolkit.getDefaultRootWindow(),
                                         XA_ICEWM_WINOPTHINT.getAtom(), 
                                         XA_ICEWM_WINOPTHINT.getAtom(),
                                         8, XlibWrapper.PropModeReplace, 
                                         new String(opt));
            XToolkit.RESTORE_XERROR_HANDLER();

            if (XToolkit.saved_error != null && XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
                log.finer("Erorr getting XA_ICEWM_WINOPTHINT property");
                return false;
            }
            log.finer("Prepared for IceWM detection");
            return true;
        } finally {
            XToolkit.awtUnlock();
        }
    }
    
    /*
     * Is IceWM running?
     *
     * Note well: Only call this if awt_wm_prepareIsIceWM succeeded, or a
     * false positive will be reported.
     */
    static boolean isIceWM() {
        if (!XA_ICEWM_WINOPTHINT.isInterned()) {
            log.log(Level.FINER, "{0} is not interned", new Object[] {XA_ICEWM_WINOPTHINT});
            return false;
        }

        WindowPropertyGetter getter = 
            new WindowPropertyGetter(XToolkit.getDefaultRootWindow(), 
                                     XA_ICEWM_WINOPTHINT, 0, 0xFFFF,
                                     true, XA_ICEWM_WINOPTHINT);
        try {
            int status = getter.execute();
            boolean res = (status == XlibWrapper.Success && getter.getActualType() != 0);
            log.finer("Status getting XA_ICEWM_WINOPTHINT: " + !res);
            return !res || isNetWMName("IceWM");
        } finally {
            getter.dispose();
        }
    }

    /*
     * Is OpenLook WM running?
     * 
     * This one is pretty lame, but the only property peculiar to OLWM is
     * _SUN_WM_PROTOCOLS(ATOM[]).  Fortunately, olwm deletes it on exit.
     */
    static final XAtom XA_SUN_WM_PROTOCOLS = new XAtom("_SUN_WM_PROTOCOLS", false);
    static boolean isOpenLook() {
        
        if (!XA_SUN_WM_PROTOCOLS.isInterned()) {
            return false;
        }

        XAtom[] list = XA_SUN_WM_PROTOCOLS.getAtomListProperty(XToolkit.getDefaultRootWindow());
        return (list.length != 0);
    }

    /*
     * Temporary error handler that checks if selecting for
     * SubstructureRedirect failed.
     */
    static boolean winmgr_running = false;
    static XToolkit.XErrorHandler DetectWMHandler = new XToolkit.XErrorHandler() {
            public int handleError(long display, XErrorEvent err) {
                XToolkit.XERROR_SAVE(err);
                if (err.get_request_code() == XlibWrapper.X_ChangeWindowAttributes 
                    && err.get_error_code() == XlibWrapper.BadAccess) 
                {
                    winmgr_running = true;
                    return 0;
                } else {
                    return XToolkit.SAVED_ERROR_HANDLER(display, err);
                }
            }
        };
    

    /*
     * Make an educated guess about running window manager.
     * XXX: ideally, we should detect wm restart.
     */
    static int awt_wmgr = XWM.UNDETERMINED_WM;
    static XWM wm;
    static XWM getWM() {
        if (wm == null) {
            
            wm = new XWM(awt_wmgr = getWMID()/*XWM.OTHER_WM*/);
        }
        return wm;
    }
    static int getWMID() {
        /*
         * Ideally, we should support cases when a different WM is started
         * during a Java app lifetime.
         */

        String vendor_string;
        boolean doIsIceWM;
        
        if (awt_wmgr != XWM.UNDETERMINED_WM) {
            return awt_wmgr;
        }
        XSetWindowAttributes substruct = new XSetWindowAttributes();        
        XToolkit.awtLock();
        try {
            /*
             * Quick checks for specific servers.
             */
            vendor_string = XlibWrapper.ServerVendor(XToolkit.getDisplay());
            if (vendor_string.indexOf("eXcursion") != -1) {
                /*
                 * Use NO_WM since in all other aspects eXcursion is like not
                 * having a window manager running. I.e. it does not reparent
                 * top level shells.
                 */
                awt_wmgr = XWM.NO_WM;
                return awt_wmgr;
            }

            /*
             * If *any* window manager is running?
             *
             * Try selecting for SubstructureRedirect, that only one client
             * can select for, and if the request fails, than some other WM is
             * already running.
             */
            winmgr_running = false;
            substruct.set_event_mask(XlibWrapper.SubstructureRedirectMask);

            XToolkit.WITH_XERROR_HANDLER(DetectWMHandler);
            XlibWrapper.XChangeWindowAttributes(XToolkit.getDisplay(),
                                                XToolkit.getDefaultRootWindow(),
                                                XlibWrapper.CWEventMask,
                                                substruct.pData);
            XToolkit.RESTORE_XERROR_HANDLER();

            /*
             * If no WM is running than our selection for SubstructureRedirect
             * succeeded and needs to be undone (hey we are *not* a WM ;-).
             */
            if (!winmgr_running) {
                awt_wmgr = XWM.NO_WM;
                substruct.set_event_mask(0);
                XlibWrapper.XChangeWindowAttributes(XToolkit.getDisplay(),
                                                    XToolkit.getDefaultRootWindow(),
                                                    XlibWrapper.CWEventMask,
                                                    substruct.pData);
                return XWM.NO_WM;
            }

            // Initialize _NET protocol - used to detect Window Manager.
            // Later, WM will initialize its own version of protocol
            net_protocol = new XNETProtocol();
            net_protocol.detect();
            if (log.isLoggable(Level.FINE) && net_protocol.active()) {
                log.fine("_NET_WM_NAME is " + net_protocol.getWMName());
            }
            XWINProtocol win = new XWINProtocol();
            win.detect();

            /* actual check for IceWM to follow below */
            doIsIceWM = prepareIsIceWM(); /* and let IceWM to act */

            /*
             * Ok, some WM is out there.  Check which one by testing for
             * "distinguishing" atoms.
             */
            if (isEnlightenment()) {
                awt_wmgr = XWM.ENLIGHTEN_WM;
            } else if (isMetacity()) {
                awt_wmgr = XWM.METACITY_WM;
            } else if (isSawfish()) {
                awt_wmgr = XWM.SAWFISH_WM;
            } else if (isKDE2()) {
                awt_wmgr =XWM. KDE2_WM;
            } else 
                if (doIsIceWM && isIceWM()) {
                    awt_wmgr = XWM.ICE_WM;
                }
            /*
             * We don't check for legacy WM when we already know that WM
             * supports WIN or _NET wm spec.
             */
                else if (net_protocol.active()) {
                    awt_wmgr = XWM.OTHER_WM;
                } else if (win.active()) {
                    awt_wmgr = XWM.OTHER_WM;
                }
            /*
             * Check for legacy WMs.
             */
                else if (isCDE()) { /* XXX: must come before isMotif */
                    awt_wmgr = XWM.CDE_WM;
                } else if (isMotif()) {
                    awt_wmgr = XWM.MOTIF_WM;
                } else if (isOpenLook()) {
                    awt_wmgr = XWM.OPENLOOK_WM;
                } else {
                    awt_wmgr = XWM.OTHER_WM;
                }

            if (!net_protocol.active()) {
                net_protocol = null;
            }
            return awt_wmgr;
        } finally {
            XToolkit.awtUnlock();
            substruct.dispose();
        }
    }


/*****************************************************************************\
 *
 * Size and decoration hints ...
 *
\*****************************************************************************/


    /*
     * Remove size hints specified by the mask.
     * XXX: Why do we need this in the first place???
     */
    static void removeSizeHints(XDecoratedPeer window, long mask) {

        XToolkit.awtLock();
        mask &= PMaxSize | PMinSize;
        
        try {
            XSizeHints hints = window.getHints();
            if ((hints.get_flags() & mask) == 0) {
                return;
            }

            hints.set_flags(hints.get_flags() & ~mask);
            if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting hints, flags " + XlibWrapper.hintsToString(hints.get_flags()));
            XlibWrapper.XSetWMNormalHints(XToolkit.getDisplay(), 
                                          window.getWindow(),
                                          hints.pData);
        } finally {
            XToolkit.awtUnlock();
        }
    }    

    /*
     * If MWM_DECOR_ALL bit is set, then the rest of the bit-mask is taken
     * to be subtracted from the decorations.  Normalize decoration spec
     * so that we can map motif decor to something else bit-by-bit in the
     * rest of the code.
     */
    static int normalizeMotifDecor(int decorations) {
        if ((decorations & MWM_DECOR_ALL) == 0) {
            return decorations;
        }
        int d = MWM_DECOR_BORDER | MWM_DECOR_RESIZEH 
            | MWM_DECOR_TITLE 
            | MWM_DECOR_MENU | MWM_DECOR_MINIMIZE 
            | MWM_DECOR_MAXIMIZE;
        d &= ~decorations;
        return d;
    }
    
    /*
     * Infer OL properties from MWM decorations.
     * Use _OL_DECOR_DEL(ATOM[]) to remove unwanted ones.
     */
    static void setOLDecor(XWindow window, boolean resizable, int decorations) {
        if (window == null) {
            return;
        }

        XAtomList decorDel = new XAtomList();
        decorations = normalizeMotifDecor(decorations);
        if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting OL_DECOR to " + Integer.toBinaryString(decorations));
        if ((decorations & MWM_DECOR_TITLE) == 0) {
            decorDel.add(XA_OL_DECOR_HEADER);
        }
        if ((decorations & (MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE)) == 0) {
            decorDel.add(XA_OL_DECOR_RESIZE);
        }
        if ((decorations & (MWM_DECOR_MENU | 
                            MWM_DECOR_MAXIMIZE |
                            MWM_DECOR_MINIMIZE)) == 0) 
        {
            decorDel.add(XA_OL_DECOR_CLOSE);
        }
        if (decorDel.size() == 0) {
            insLog.finer("Deleting OL_DECOR");
            XA_OL_DECOR_DEL.DeleteProperty(window);
        } else {
            if (insLog.isLoggable(Level.FINER)) insLog.finer("Setting OL_DECOR to " + decorDel);
            XA_OL_DECOR_DEL.setAtomListProperty(window, decorDel);
        }        
    }

    /*
     * Set MWM decorations.  Set MWM functions depending on resizability.
     */
    static void setMotifDecor(XWindow window, boolean resizable, int decorations) {
        /* Apparently some WMs don't implement MWM_*_ALL semantic correctly */
        if ((decorations & MWM_DECOR_ALL) != 0 
            && (decorations != MWM_DECOR_ALL)) 
        {
            decorations = normalizeMotifDecor(decorations);
        }

        int functions = 0;

        if ( (decorations & MWM_DECOR_ALL) != 0) {
            functions |= MWM_FUNC_ALL;
        } else {
            /*
             * Functions we always want to be enabled as mwm(1) and
             * descendants not only hide disabled functions away from
             * user, but also ignore corresponding requests from the
             * program itself (e.g. 4442047).
             */
            functions |= MWM_FUNC_CLOSE | MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
            if( resizable ) {
                functions |= MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE;
            }
        }
        
        
        PropMwmHints hints = new PropMwmHints();
        if (XA_MWM_HINTS.getAtomData(window.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS)) {
            hints.set_flags(hints.get_flags() | MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS);
        } else {
            hints.set_flags(MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS);
        }
        hints.set_functions(functions);
        hints.set_decorations(decorations);

        if (stateLog.isLoggable(Level.FINER)) stateLog.finer("Setting MWM_HINTS to " + hints);
        XA_MWM_HINTS.setAtomData(window.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS);
        
        hints.dispose();
    }

    /*
     * Under some window managers if shell is already mapped, we MUST
     * unmap and later remap in order to effect the changes we make in the
     * window manager decorations.
     * 
     * N.B.  This unmapping / remapping of the shell exposes a bug in
     * X/Motif or the Motif Window Manager.  When you attempt to map a
     * widget which is positioned (partially) off-screen, the window is
     * relocated to be entirely on screen. Good idea.  But if both the x
     * and the y coordinates are less than the origin (0,0), the first
     * (re)map will move the window to the origin, and any subsequent
     * (re)map will relocate the window at some other point on the screen.
     * I have written a short Motif test program to discover this bug.
     * This should occur infrequently and it does not cause any real
     * problem.  So for now we'll let it be.
     */
    static boolean needRemap() {
        return true;
    }

    /*
     * Set decoration hints on the shell to wdata->decor adjusted
     * appropriately if not resizable.
     */
    static void setShellDecor(XDecoratedPeer window) {
        int decorations = window.getDecorations();
        boolean resizable = window.isResizable();

        if (!resizable) {
            if ((decorations & MWM_DECOR_ALL) != 0) {
                decorations |= MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE;
            } else {
                decorations &= ~(MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE);
            }
        }
        setMotifDecor(window, resizable, decorations);
        setOLDecor(window, resizable, decorations);

        /* Some WMs need remap to redecorate the window */
        if (window.isShowing() && needRemap()) {
            /*
             * Do the re/mapping at the Xlib level.  Since we essentially
             * work around a WM bug we don't want this hack to be exposed
             * to Intrinsics (i.e. don't mess with grabs, callbacks etc).
             */
            window.xSetVisible(false);
            XToolkit.XSync();
            window.xSetVisible(true);
        }
    }

    /*
     * Make specified shell resizable.
     */
    static void setShellResizable(XDecoratedPeer window) {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting shell resizable " + window);
        synchronized(XToolkit.getAWTLock()) {
            Rectangle shellBounds = window.getShellBounds();
            shellBounds.translate(-window.currentInsets.left, -window.currentInsets.top);
            window.updateSizeHints(window.getDimensions());
            XlibWrapper.XMoveResizeWindow(XToolkit.getDisplay(), window.getShell(), 
                                          shellBounds.x, shellBounds.y, shellBounds.width, shellBounds.height);
            /* REMINDER: will need to revisit when setExtendedStateBounds is added */
            removeSizeHints(window, PMinSize|PMaxSize);

            /* Restore decorations */
            setShellDecor(window);        
        }
    }

    /*
     * Make specified shell non-resizable.
     * If justChangeSize is false, update decorations as well.
     * @param shellBounds bounds of the shell window
     */
    static void setShellNotResizable(XDecoratedPeer window, WindowDimensions newDimensions, Rectangle shellBounds,
                                     boolean justChangeSize) 
    {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting non-resizable shell " + window + ", dimensions " + newDimensions + 
                                                       ", shellBounds " + shellBounds +", just change size: " + justChangeSize);
        synchronized(XToolkit.getAWTLock()) {
            /* Fix min/max size hints at the specified values */
            if (!shellBounds.isEmpty()) {
                window.updateSizeHints(newDimensions);
                XToolkit.XSync();
                XlibWrapper.XMoveResizeWindow(XToolkit.getDisplay(), window.getShell(), 
                                              shellBounds.x, shellBounds.y, shellBounds.width, shellBounds.height);
            }
            if (!justChangeSize) {  /* update decorations */
                setShellDecor(window);
            }
        }
    }

    /*
     * Helper function for awt_wm_getInsetsFromProp.
     * Read property of type CARDINAL[4] = { left, right, top, bottom }
     */
    static boolean readInsetsArray(long window, XAtom insets_property,
                                   int[] insets)
    {
        WindowPropertyGetter getter = 
            new WindowPropertyGetter(window, insets_property,
                                     0, 4, false, XAtom.XA_CARDINAL);
        try {
            int status = getter.execute();
            if (status != XlibWrapper.Success || getter.getData() == 0) {
                return false;
            }

            if (getter.getActualType() != XAtom.XA_CARDINAL || 
                getter.getActualFormat() != 32) 
            {
                return false;
            }
            for (int i = 0; i < 4; i++) {
                insets[i] = (int)Native.getCard32(getter.getData(), i);
            }
            return true;
        } finally {
            getter.dispose();
        }
    }
    

    
/*****************************************************************************\
 * Protocols support
 */ 
    HashMap protocolsMap = new HashMap();
    /**
     * Returns all protocols supporting given protocol interface
     */
    Collection getProtocols(Class protocolInterface) {
        Object res = protocolsMap.get(protocolInterface);
        if (res != null) {
            return (Collection)res;
        } else {
            return new LinkedList();
        }
    }

    void addProtocol(Class protocolInterface, XProtocol protocol) {
        Collection protocols = getProtocols(protocolInterface);
        protocols.add(protocol);
        protocolsMap.put(protocolInterface, protocols);
    }

    boolean supportsDynamicLayout() {
        int wm = getWMID();
        switch (wm) {
          case XWM.ENLIGHTEN_WM: 
          case XWM.KDE2_WM:
          case XWM.SAWFISH_WM:
          case XWM.ICE_WM:
          case XWM.METACITY_WM:
              return true;
          case XWM.OPENLOOK_WM:
          case XWM.MOTIF_WM:
          case XWM.CDE_WM:
              return false;
          default:
              return false;
        }
    }


    /**
     * Check if state is supported.
     * Note that a compound state is always reported as not supported. 
     * Note also that MAXIMIZED_BOTH is considered not a compound state. 
     * Therefore, a compound state is just ICONIFIED | anything else.
     *
     */
    boolean supportsExtendedState(int state) {
        switch (state) {
          case Frame.MAXIMIZED_VERT:
          case Frame.MAXIMIZED_HORIZ:
              /*
               * WMs that talk NET/WIN protocol, but do not support
               * unidirectional maximization.
               */
              if (getWMID() == METACITY_WM) {
                  /* "This is a deliberate policy decision." -hp */
                  return false;
              }
              /* FALLTROUGH */
          case Frame.MAXIMIZED_BOTH:
              Iterator iter = getProtocols(XStateProtocol.class).iterator();
              while (iter.hasNext()) {
                  XStateProtocol proto = (XStateProtocol)iter.next();
                  if (proto.supportsState(state)) {
                      return true;
                  }
              }
          default:
              return false;
        }
    }
    
/*****************************************************************************\
 *
 * Reading state from different protocols
 *
\*****************************************************************************/


    int getExtendedState(XWindowPeer window) {
        Iterator iter = getProtocols(XStateProtocol.class).iterator();
        int state = 0;
        while (iter.hasNext()) {
            XStateProtocol proto = (XStateProtocol)iter.next();
            state |= proto.getState(window);
        }
        if (state != 0) {
            return state;
        } else {
            return Frame.NORMAL;
        }
    }
    
/*****************************************************************************\
 *
 * Notice window state change when WM changes a property on the window ...
 *
\*****************************************************************************/


    /*
     * Check if property change is a window state protocol message.
     * If it is - return the new state as Integer, otherwise return null
     */
    Integer isStateChange(XDecoratedPeer window, XPropertyEvent e) {
        if (!window.isShowing()) {
            stateLog.finer("Window is not showing");
            return null;
        }

        int wm_state = window.getWMState();
        if (wm_state == XlibWrapper.WithdrawnState) {
            stateLog.finer("WithdrawnState");
            return null;
        } else {
            stateLog.finer("Window WM_STATE is " + wm_state);
        }
        boolean is_state_change = false;
        if (e.get_atom() == XA_WM_STATE.getAtom()) {
            is_state_change = true;
        }

        Iterator iter = getProtocols(XStateProtocol.class).iterator();
        while (iter.hasNext()) {
            XStateProtocol proto = (XStateProtocol)iter.next();
            is_state_change |= proto.isStateChange(e);
        }
        int res = 0;

        if (is_state_change) {
            if (wm_state == XlibWrapper.IconicState) {
                res = Frame.ICONIFIED;
            } else {
                res = Frame.NORMAL;
            }
            res |= getExtendedState(window);
        }
        if (is_state_change) {
            return new Integer(res);
        } else {
            return null;
        }
    }

/*****************************************************************************\
 *
 * Setting/changing window state ...
 *
\*****************************************************************************/

    /**
     * Moves window to the specified layer, layer is one of the constants defined
     * in XLayerProtocol
     */
    void setLayer(XWindowPeer window, int layer) {
        Iterator iter = getProtocols(XLayerProtocol.class).iterator();
        while (iter.hasNext()) {
            XLayerProtocol proto = (XLayerProtocol)iter.next();            
            if (proto.supportsLayer(layer)) {
                proto.setLayer(window, layer);                
            }
        }
        XToolkit.XSync();
    }

    void setExtendedState(XWindowPeer window, int state) {
        Iterator iter = getProtocols(XStateProtocol.class).iterator();
        while (iter.hasNext()) {
            XStateProtocol proto = (XStateProtocol)iter.next();
            if (proto.supportsState(state)) {
                proto.setState(window, state);
                break;
            }
        }
        
        if (!window.isShowing()) {
            /*
             * Purge KWM bits.
             * Not really tested with KWM, only with WindowMaker.
             */
            try {
                XToolkit.awtLock();
                XlibWrapper.XDeleteProperty(XToolkit.getDisplay(), 
                                            window.getWindow(), 
                                            XA_KWM_WIN_ICONIFIED.getAtom());
                XlibWrapper.XDeleteProperty(XToolkit.getDisplay(), 
                                            window.getWindow(), 
                                            XA_KWM_WIN_MAXIMIZED.getAtom());
            }
            finally {
                XToolkit.awtUnlock();
            }
        }
        XToolkit.XSync();
    }


    /*
     * Work around for 4775545.
     *
     * If WM exits while the top-level is shaded, the shaded hint remains
     * on the top-level properties.  When WM restarts and sees the shaded
     * window it can reparent it into a "pre-shaded" decoration frame
     * (Metacity does), and our insets logic will go crazy, b/c it will
     * see a huge nagative bottom inset.  There's no clean solution for
     * this, so let's just be weasels and drop the shaded hint if we
     * detect that WM exited.  NB: we are in for a race condition with WM
     * restart here.  NB2: e.g. WindowMaker saves the state in a private
     * property that this code knows nothing about, so this workaround is
     * not effective; other WMs might play similar tricks.
     */
    void unshadeKludge(XDecoratedPeer window) {
        assert(window.isShowing());

        Iterator iter = getProtocols(XStateProtocol.class).iterator();
        while (iter.hasNext()) {
            XStateProtocol proto = (XStateProtocol)iter.next();
            proto.unshadeKludge(window);
        }
        XToolkit.XSync();
    }

    static boolean inited = false;
    static void init() {
        if (inited) {
            return;
        }
        
        initAtoms();
        getWM();
        inited = true;
    }

    void initializeProtocols() {
        net_protocol = new XNETProtocol();
        if (!net_protocol.active()) {
            net_protocol = null;
        } else {
            if (net_protocol.doStateProtocol()) {
                addProtocol(XStateProtocol.class, net_protocol);
            }
            if (net_protocol.doLayerProtocol()) {
                addProtocol(XLayerProtocol.class, net_protocol);
            }
            if (net_protocol.doModalityProtocol()) {
                addProtocol(XModalityProtocol.class, net_protocol);
            }
        }

        XWINProtocol win = new XWINProtocol();
        if (win.active()) {
            if (win.doStateProtocol()) {
                addProtocol(XStateProtocol.class, win);
            }
            if (win.doLayerProtocol()) {
                addProtocol(XLayerProtocol.class, win);
            }
        }

        XMWMModality modal = new XMWMModality();
        addProtocol(XModalityProtocol.class, modal);
    }

    HashMap storedInsets = new HashMap();
    Insets guessInsets(XDecoratedPeer window) {
        Insets res = (Insets)storedInsets.get(window.getClass());
        if (res == null) {
            switch (WMID) {
              case ENLIGHTEN_WM:
                  res = new Insets(19, 4, 4, 4);
                  break;
              case CDE_WM:
                  res = new Insets(28, 6, 6, 6);
                  break;
              case NO_WM:
                  res = zeroInsets;
                  break;
              case MOTIF_WM:
              case OPENLOOK_WM:
              default:
                  res = defaultInsets;
            }            
        }
        if (insLog.isLoggable(Level.FINEST)) insLog.finest("WM guessed insets: " + res);
        return res;
    }
    /*
     * Some buggy WMs ignore window gravity when processing
     * ConfigureRequest and position window as if the gravity is Static.
     * We work around this in MWindowPeer.pReshape().
     *
     * Starting with 1.5 we have introduced an Environment variable 
     * _JAVA_AWT_WM_STATIC_GRAVITY that can be set to indicate to Java
     * explicitly that the WM has this behaviour, example is FVWM.
     */

    static int awtWMStaticGravity = -1;
    static boolean configureGravityBuggy() {

        if (awtWMStaticGravity == -1) {
            awtWMStaticGravity = (XToolkit.getEnv("_JAVA_AWT_WM_STATIC_GRAVITY") != null) ? 1 : 0; 
        }
        
        if (awtWMStaticGravity == 1) {
            return true;
        }
        
        switch(getWMID()) {
          case XWM.ICE_WM:
              /*
               * See bug #228981 at IceWM's SourceForge pages.
               * Latest stable version 1.0.8-6 still has this problem.
               */
              /**
               * Version 1.2.2 doesn't have this problem
               */
              return true;
          case XWM.ENLIGHTEN_WM:
              /* At least E16 is buggy. */
              return true;
          default:
              return false;
        }
    }

    /*
     * If WM implements the insets property - fill insets with values
     * specified in that property.
     */
    static boolean getInsetsFromProp(long window, int[] insets) {
        switch(getWMID()) {
          case XWM.ENLIGHTEN_WM:
              return readInsetsArray(window, XA_E_FRAME_SIZE,
                                     insets);
          default:
              return false;
        }
    }
    /* syncTopLEvelPos() is necessary to insure that the window manager has in
     * fact moved us to our final position relative to the reParented WM window.
     * We have noted a timing window which our shell has not been moved so we
     * screw up the insets thinking they are 0,0.  Wait (for a limited period of
     * time to let the WM hava a chance to move us
     */
    void syncTopLevelPos(long window, XWindowAttributes attrs) {
        int tries = 0;        
        try {
            XToolkit.awtLock();
            do {
                XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(), window, attrs.pData);
                if (attrs.get_x() != 0 || attrs.get_y() != 0) {
                    break;
                }
                tries++;
                XToolkit.XSync();                
            } while (tries < 50);
        }
        finally {
            XToolkit.awtUnlock();
        }
    } 

    Insets getInsets(XDecoratedPeer win, XReparentEvent xe) {
        XWindowAttributes lwinAttr = new XWindowAttributes(), 
            actualAttr = new XWindowAttributes();
        Insets correctWM = new Insets(0,0,0,0);
                
        correctWM.top = -1;
        correctWM.left = -1;
        /*
         * Unfortunately the concept of "insets" borrowed to AWT
         * from Win32 is *absolutely*, *unbelievably* foreign to
         * X11.  Few WMs provide the size of frame decor
         * (i.e. insets) in a property they set on the client
         * window, so we check if we can get away with just
         * peeking at it.  [Future versions of wm-spec might add a
         * standardized hint for this].
         * 
         * Otherwise we do some special casing.  Actually the
         * fallback code ("default" case) seems to cover most of
         * the existing WMs (modulo Reparent/Configure order
         * perhaps?).
         * 
         * Fallback code tries to account for the two most common cases:
         * 
         * . single reparenting
         *       parent window is the WM frame
         *       [twm, olwm, sawfish]
         * 
         * . double reparenting
         *       parent is a lining exactly the size of the client
         *       grandpa is the WM frame
         *       [mwm, e!, kwin, fvwm2 ... ]
         */
        int[] guessing = new int[4];
        if (XWM.getInsetsFromProp(xe.get_window(), guessing)) {
            // array: left, right, top, bottom
            // constructor: top, left, bottom, right
            correctWM = new Insets(guessing[2], guessing[0], guessing[3], guessing[1]);
            insLog.log(Level.FINER, "Got insets from property: {0}", new Object[]{guessing});
        } else {
            switch (XWM.getWMID()) {
                /* should've been done in awt_wm_getInsetsFromProp */
              case XWM.ENLIGHTEN_WM: {
                  /* enlightenment does double reparenting */
                  syncTopLevelPos(xe.get_parent(), lwinAttr);
                  XQueryTree qt = new XQueryTree(xe.get_parent());
                  try {
                      qt.execute();
                      correctWM.left = lwinAttr.get_x();
                      correctWM.top = lwinAttr.get_y();
                      /*
                       * Now get the actual dimensions of the parent window
                       * resolve the difference.  We can't rely on the left
                       * to be equal to right or bottom...  Enlightment
                       * breaks that assumption.
                       */
                      XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                       qt.get_parent(),
                                                       actualAttr.pData);
                      correctWM.right = actualAttr.get_width() -
                          (lwinAttr.get_width() + correctWM.left);
                      correctWM.bottom = actualAttr.get_height() -
                          (lwinAttr.get_height() + correctWM.top);
                      break;                                                           
                  } finally {
                      qt.dispose();
                  }
              }
              case XWM.ICE_WM: // for 1.2.2.
              case XWM.KDE2_WM: /* should've been done in getInsetsFromProp */
              case XWM.CDE_WM:
              case XWM.MOTIF_WM: {
                  /* these are double reparenting too */
                  syncTopLevelPos(xe.get_parent(), lwinAttr);

                  correctWM.top = lwinAttr.get_y();
                  correctWM.left = lwinAttr.get_x();
                  correctWM.right = correctWM.left;
                  correctWM.bottom = correctWM.left;
                  // The code below didn't work for CDE and IceWM 1.2.2,
                  // but might work for KDE, needs more testing
//                           XTranslateCoordinates tc = 
//                               new XTranslateCoordinates(xe.get_window(),
//                                                         root,
//                                                         0, 0);
//                           tc.execute();
//                            long containerWindow;
//                           containerWindow = tc.get_child();
//                            int screenX, screenY;
//                           screenX = tc.get_dest_x();
//                           screenY = tc.get_dest_y();
//                           if ((screenX != x + guessedInsets.left)  ||
//                               (screenY != y + guessedInsets.top)) 
//                           {
//                               /* 
//                                * looks like the window manager has placed us somewhere 
//                                * other than where we asked for, lets respect the window
//                                * and go where he put us, not where we tried to put us
//                                */
//                               x = screenX - correctWM.left;
//                               y = screenY - correctWM.top;
//                           }
                  break;
              }
              case XWM.SAWFISH_WM: 
              case XWM.OPENLOOK_WM: {
                  /* single reparenting */
                  syncTopLevelPos(xe.get_window(), lwinAttr);
                  correctWM.top    = lwinAttr.get_y();
                  correctWM.left   = lwinAttr.get_x();
                  correctWM.right  = correctWM.left;
                  correctWM.bottom = correctWM.left;
                  break;                          
              }
              case XWM.OTHER_WM:
              default: {                /* this is very similar to the E! case above */
                  syncTopLevelPos(xe.get_parent(), lwinAttr);
                  long w = xe.get_window();
                  long parent = xe.get_parent();
                  XWindowAttributes pattr = new XWindowAttributes();
                  int status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                                w, lwinAttr.pData);
                  status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                            parent, pattr.pData);
                  /*
                   * Check for double-reparenting WM.
                   * 
                   * If the parent is exactly the same size as the
                   * top-level assume taht it's the "lining" window and
                   * that the grandparent is the actual frame (NB: we
                   * have already handled undecorated windows).
                   * 
                   * XXX: what about timing issues that syncTopLevelPos
                   * is supposed to work around?
                   */
                  if (lwinAttr.get_x() == 0 && lwinAttr.get_y() == 0
                      && lwinAttr.get_width()+2*lwinAttr.get_border_width() == pattr.get_width()
                      && lwinAttr.get_height()+2*lwinAttr.get_border_width() == pattr.get_height())
                  {
                      XQueryTree qt = new XQueryTree(parent);
                      try {
                          qt.execute();
                          lwinAttr.set_x(pattr.get_x());
                          lwinAttr.set_y(pattr.get_y());
                          lwinAttr.set_border_width(lwinAttr.get_border_width()+pattr.get_border_width());
                              
                          parent = qt.get_parent();
                          XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                           parent,
                                                           pattr.pData);                              
                      } finally {
                          qt.dispose();
                      }       
                  }
                  /*
                   * XXX: To be absolutely correct, we'd need to take
                   * parent's border-width into account too, but the
                   * rest of the code is happily unaware about border
                   * widths and inner/outer distinction, so for the time
                   * being, just ignore it.
                   */
                  correctWM = new Insets(lwinAttr.get_y() + lwinAttr.get_border_width(),
                                         lwinAttr.get_x() + lwinAttr.get_border_width(),
                                         pattr.get_height() - (lwinAttr.get_y() + lwinAttr.get_height() + 2*lwinAttr.get_border_width()),
                                         pattr.get_width() -  (lwinAttr.get_x() + lwinAttr.get_width() + 2*lwinAttr.get_border_width()));
                  break;
              } /* default */
            } /* switch (runningWM) */
        }
        if (storedInsets.get(win.getClass()) == null) {
            storedInsets.put(win.getClass(), correctWM);
        }
        return correctWM;
    }
    boolean isDesktopWindow( long w ) {
        if (net_protocol != null) {
            XAtomList wtype = XAtom.get("_NET_WM_WINDOW_TYPE").getAtomListPropertyList( w );
            return wtype.contains( XAtom.get("_NET_WM_WINDOW_TYPE_DESKTOP") );
        } else {
            return false;
        }
    }

}
