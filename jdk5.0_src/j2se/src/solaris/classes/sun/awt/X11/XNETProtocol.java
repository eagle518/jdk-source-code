/*
 * @(#)XNETProtocol.java	1.12 1.12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.X11;

import java.awt.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.LogManager;

class XNETProtocol extends XProtocol implements XStateProtocol, XLayerProtocol, XModalityProtocol {
    final static Logger log = Logger.getLogger("sun.awt.X11.XNETProtocol");

    XModalityProtocol modalDelegate = new XMWMModality();

    /**
     * XStateProtocol
     */
    public boolean supportsState(int state) {
        return doStateProtocol() ; // TODO - check for Frame constants
    }

    public void setState(XWindowPeer window, int state) {
        if (log.isLoggable(Level.FINE)) log.fine("Setting state of " + window + " to " + state);
        if (window.isShowing()) {
            requestState(window, state);
        } else {
            setInitialState(window, state);
        }
    }

    private void setInitialState(XWindowPeer window, int state) {
        XAtomList old_state = XA_NET_WM_STATE.getAtomListPropertyList(window);
        log.log(Level.FINE, "Current state of the window {0} is {1}", new Object[] {window, old_state});
        if ((state & Frame.MAXIMIZED_VERT) != 0) {
            old_state.add(XA_NET_WM_STATE_MAXIMIZED_VERT);
        } else {
            old_state.remove(XA_NET_WM_STATE_MAXIMIZED_VERT);
        }
        if ((state & Frame.MAXIMIZED_HORIZ) != 0) {
            old_state.add(XA_NET_WM_STATE_MAXIMIZED_HORZ);
        } else {
            old_state.remove(XA_NET_WM_STATE_MAXIMIZED_HORZ);
        }
        log.log(Level.FINE, "Setting initial state of the window {0} to {1}", new Object[] {window, old_state});
        XA_NET_WM_STATE.setAtomListProperty(window, old_state);
    }

    private void requestState(XWindowPeer window, int state) {
        /*
         * We have to use toggle for maximization because of transitions
         * from maximization in one direction only to maximization in the
         * other direction only.
         */
        int old_net_state = getState(window);
        int max_changed = (state ^ old_net_state) & (Frame.MAXIMIZED_BOTH);

        XClientMessageEvent req = new XClientMessageEvent();
        try {
            switch(max_changed) {
              case 0:
                  return;
              case Frame.MAXIMIZED_HORIZ:
                  req.set_data(1, XA_NET_WM_STATE_MAXIMIZED_HORZ.getAtom());
                  req.set_data(2, 0);
                  break;
              case Frame.MAXIMIZED_VERT:
                  req.set_data(1, XA_NET_WM_STATE_MAXIMIZED_VERT.getAtom());
                  req.set_data(2, 0);
                  break;
              case Frame.MAXIMIZED_BOTH:
                  req.set_data(1, XA_NET_WM_STATE_MAXIMIZED_HORZ.getAtom());
                  req.set_data(2, XA_NET_WM_STATE_MAXIMIZED_VERT.getAtom());
                  break;
              default:
                  return;
            }
            if (log.isLoggable(Level.FINE)) log.fine("Requesting state on " + window + " for " + state);
            req.set_type((int)XlibWrapper.ClientMessage);
            req.set_window(window.getWindow());
            req.set_message_type(XA_NET_WM_STATE.getAtom());
            req.set_format(32);
            req.set_data(0, _NET_WM_STATE_TOGGLE);
            try {
                XToolkit.awtLock();
                XlibWrapper.XSendEvent(XToolkit.getDisplay(),
                        XlibWrapper.RootWindow(XToolkit.getDisplay(), window.getScreenNumber()),
                        false,
                        XlibWrapper.SubstructureRedirectMask | XlibWrapper.SubstructureNotifyMask,
                        req.pData);
            }
            finally {
                XToolkit.awtUnlock();
            }
        } finally {
            req.dispose();
        }
    }

    /*
     * New "NET" WM spec: _NET_WM_STATE/Atom[]
     */
    public int getState(XWindowPeer window) {
        XAtom[] net_wm_state = XA_NET_WM_STATE.getAtomListProperty(window);
        if (net_wm_state.length == 0) {
            return Frame.NORMAL;
        }
        int java_state = Frame.NORMAL;
        for (int i = 0; i < net_wm_state.length; i++) {
            if (XA_NET_WM_STATE_MAXIMIZED_VERT.equals(net_wm_state[i])) {
                java_state |= Frame.MAXIMIZED_VERT;
            } else if (XA_NET_WM_STATE_MAXIMIZED_HORZ.equals(net_wm_state[i])) {
                java_state |= Frame.MAXIMIZED_HORIZ;
            }
        }
        return java_state;
    }

    public boolean isStateChange(XPropertyEvent e) {
        return doStateProtocol() && (e.get_atom() == XA_NET_WM_STATE.getAtom()) ;
    }

    /*
     * Work around for 4775545.
     */
    public void unshadeKludge(XWindowPeer window) {
        XAtomList net_wm_state = XA_NET_WM_STATE.getAtomListPropertyList(window);
        net_wm_state.remove(XA_NET_WM_STATE_SHADED);
        XA_NET_WM_STATE.setAtomListProperty(window, net_wm_state);
    }

    /**
     * XLayerProtocol
     */
    public boolean supportsLayer(int layer) {
        return ((layer == LAYER_ALWAYS_ON_TOP) || (layer == LAYER_NORMAL)) && doLayerProtocol();
    }

    /**
     * Helper function to set/reset one state in NET_WM_STATE
     * If window is showing then it uses ClientMessage, otherwise adjusts NET_WM_STATE list
     * @param window Window which NET_WM_STATE property is being modified
     * @param state State atom to be set/reset
     * @param reset Indicates operation, 'set' if false, 'reset' if true
     */
    private void setStateHelper(XWindowPeer window, XAtom state, boolean set) {
        log.log(Level.FINER, "Window visibility is: withdrawn={0}, visible={1}, mapped={2} showing={3}", 
                new Object[] {Boolean.valueOf(window.isWithdrawn()), Boolean.valueOf(window.isVisible()), 
                              Boolean.valueOf(window.isMapped()), Boolean.valueOf(window.isShowing())});        
        if (window.isShowing()) {
            XClientMessageEvent req = new XClientMessageEvent();
            try {
                req.set_type((int)XlibWrapper.ClientMessage);
                req.set_window(window.getWindow());
                req.set_message_type(XA_NET_WM_STATE.getAtom());
                req.set_format(32);
                req.set_data(0, (!set) ? _NET_WM_STATE_REMOVE : _NET_WM_STATE_ADD);
                req.set_data(1, state.getAtom());
                log.log(Level.FINE, "Setting _NET_STATE atom {0} on {1} for {2}", new Object[] {state, window, Boolean.valueOf(set)});
                try {
                    XToolkit.awtLock();
                    XlibWrapper.XSendEvent(XToolkit.getDisplay(),
                            XlibWrapper.RootWindow(XToolkit.getDisplay(), window.getScreenNumber()),
                            false,
                            XlibWrapper.SubstructureRedirectMask | XlibWrapper.SubstructureNotifyMask,
                            req.pData);
                }
                finally {
                    XToolkit.awtUnlock();
                }
            } finally {
                req.dispose();
            }
        } else {
            XAtomList net_wm_state = XA_NET_WM_STATE.getAtomListPropertyList(window);
            log.log(Level.FINE, "Current state on {0} is {1}", new Object[] {window, net_wm_state});
            if (!set) {
                net_wm_state.remove(state);
            } else {
                net_wm_state.add(state);
            }
            log.log(Level.FINE, "Setting states on {0} to {1}", new Object[] {window, net_wm_state});
            XA_NET_WM_STATE.setAtomListProperty(window, net_wm_state);
            XToolkit.XSync();
        }
        XAtomList net_wm_state = XA_NET_WM_STATE.getAtomListPropertyList(window);
        log.log(Level.FINE, "State after set on window {0} is {1}", new Object[] {window, net_wm_state});
    }

    public void setLayer(XWindowPeer window, int layer) {
        setStateHelper(window, XA_NET_WM_STATE_ABOVE, layer == LAYER_ALWAYS_ON_TOP);
    }

    /**
     *  XModalityProtocol
     */
    public boolean setModal(XDialogPeer dialog, boolean modal) {
        log.log(Level.FINE, "Setting modality for {0} to {1}", new Object[] {dialog, Boolean.valueOf(modal)});
        XA_NET_WM_WINDOW_TYPE.setAtomListProperty(dialog, new XAtom[]{XA_NET_WM_WINDOW_TYPE_DIALOG});
        XToolkit.XSync();
        setStateHelper(dialog, XA_NET_WM_STATE_MODAL, modal);
        modalDelegate.setModal(dialog, modal);
        return false;
    }

    public boolean isBlocked(XDialogPeer dialog, XWindowPeer win) {
        // Assume _NET is smart enough to not send us request for focus when
        // there is modal dialog
//         return false;
        // TODO: Make sure the assumption is correct, otherwise either create
        // default instance of appropriate modal strategy and use it here,
        // or don't use _NET modality

        // The above assumtion is incorrect!
        // So we use modality delegate to work on modal dialogs.
        return modalDelegate.isBlocked(dialog, win);
        // But this also doesn't work with Metacity.
    }

    /* New "netwm" spec from www.freedesktop.org */
    XAtom XA_UTF8_STRING = XAtom.get("UTF8_STRING");   /* like STRING but encoding is UTF-8 */
    XAtom XA_NET_SUPPORTING_WM_CHECK = XAtom.get("_NET_SUPPORTING_WM_CHECK");
    XAtom XA_NET_SUPPORTED = XAtom.get("_NET_SUPPORTED");      /* list of protocols (property of root) */
    XAtom XA_NET_WM_NAME = XAtom.get("_NET_WM_NAME");  /* window property */
    XAtom XA_NET_WM_STATE = XAtom.get("_NET_WM_STATE");/* both window property and request */

/*
 * _NET_WM_STATE is a list of atoms.
 * NB: Standard spelling is "HORZ" (yes, without an 'I'), but KDE2
 * uses misspelled "HORIZ" (see KDE bug #20229).  This was fixed in
 * KDE 2.2.  Under earlier versions of KDE2 horizontal and full
 * maximization doesn't work .
 */
    XAtom XA_NET_WM_STATE_MAXIMIZED_HORZ = XAtom.get("_NET_WM_STATE_MAXIMIZED_HORZ");
    XAtom XA_NET_WM_STATE_MAXIMIZED_VERT = XAtom.get("_NET_WM_STATE_MAXIMIZED_VERT");
    XAtom XA_NET_WM_STATE_SHADED = XAtom.get("_NET_WM_STATE_SHADED");
    XAtom XA_NET_WM_STATE_ABOVE = XAtom.get("_NET_WM_STATE_ABOVE");
    XAtom XA_NET_WM_STATE_MODAL = XAtom.get("_NET_WM_STATE_MODAL");
    XAtom XA_NET_WM_STATE_FULLSCREEN = XAtom.get("_NET_WM_STATE_FULLSCREEN");
    XAtom XA_NET_WM_STATE_BELOW = XAtom.get("_NET_WM_STATE_BELOW");

    XAtom XA_NET_WM_WINDOW_TYPE = XAtom.get("_NET_WM_WINDOW_TYPE");
    XAtom XA_NET_WM_WINDOW_TYPE_DIALOG = XAtom.get("_NET_WM_WINDOW_TYPE_DIALOG");

/* For _NET_WM_STATE ClientMessage requests */
    final static int _NET_WM_STATE_REMOVE      =0; /* remove/unset property */
    final static int _NET_WM_STATE_ADD         =1; /* add/set property      */
    final static int _NET_WM_STATE_TOGGLE      =2; /* toggle property       */

    boolean supportChecked = false;
    long NetWindow = 0;
    void detect() {
        if (supportChecked) {
            // TODO: How about detecting WM-restart or exit?
            return;
        }
        NetWindow = checkAnchor(XA_NET_SUPPORTING_WM_CHECK, XAtom.XA_WINDOW);
        supportChecked = true;
        if (log.isLoggable(Level.FINE)) log.fine("### " + this + " is active: " + (NetWindow != 0));
    }

    boolean active() {
        detect();
        return NetWindow != 0;
    }

    boolean doStateProtocol() {
        boolean res = active() && checkProtocol(XA_NET_SUPPORTED, XA_NET_WM_STATE);
        return res;
    }
    boolean doLayerProtocol() {
        boolean res = active() && checkProtocol(XA_NET_SUPPORTED, XA_NET_WM_STATE_ABOVE);
        return res;
    }
    boolean doModalityProtocol() {
        boolean res = active() && checkProtocol(XA_NET_SUPPORTED, XA_NET_WM_STATE_MODAL);
        return res;
    }
    boolean isWMName(String name) {
        if (!active()) {
            return false;
        }
        String net_wm_name_string = getWMName();
        if (net_wm_name_string == null) {
            return false;
        }
        if (log.isLoggable(Level.FINE)) log.fine("### WM_NAME = " + net_wm_name_string);
        return net_wm_name_string.startsWith(name);
    }

    public String getWMName() {
        if (!active()) {
            return null;
        }

        /*
         * Check both UTF8_STRING and STRING.  We only call this function
         * with ASCII names and UTF8 preserves ASCII bit-wise.  wm-spec
         * mandates UTF8_STRING for _NET_WM_NAME but at least sawfish-1.0
         * still uses STRING.  (mmm, moving targets...).
         */
        String charSet = "UTF8";
        byte[] net_wm_name = XA_NET_WM_NAME.getByteArrayProperty(NetWindow, XA_UTF8_STRING.getAtom());
        if (net_wm_name == null) {
            net_wm_name = XA_NET_WM_NAME.getByteArrayProperty(NetWindow, XAtom.XA_STRING);
            charSet = "ASCII";
        }

        if (net_wm_name == null) {
            return null;
        }
        try {
            String net_wm_name_string = new String(net_wm_name, charSet);
            return net_wm_name_string;
        } catch (java.io.UnsupportedEncodingException uex) {
            return null;
        }            
    }

}
