/*
 * @(#)XEmbed.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.misc.Unsafe;

import java.util.logging.*;


/**
 * Common class for all XEmbed protocol participating classes.
 * Contains constant definitions and helper routines.
 */
public class XEmbed {
    private static final Logger xembedLog = Logger.getLogger("sun.awt.X11.xembed");
    final static Unsafe unsafe = XlibWrapper.unsafe;
    
    final static int XEMBED_VERSION = 0,
        XEMBED_MAPPED = (1 << 0);
/* XEMBED messages */
    final static int XEMBED_EMBEDDED_NOTIFY	=	0;
    final static int XEMBED_WINDOW_ACTIVATE  =	1;
    final static int XEMBED_WINDOW_DEACTIVATE = 	2;
    final static int XEMBED_REQUEST_FOCUS	 	=3;
    final static int XEMBED_FOCUS_IN 	= 	4;
    final static int XEMBED_FOCUS_OUT  	=	5;
    final static int XEMBED_FOCUS_NEXT 	=	6;
    final static int XEMBED_FOCUS_PREV 	=	7;
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
    final static int XEMBED_MODALITY_ON 	=	10;
    final static int XEMBED_MODALITY_OFF 	=	11;
    final static int XEMBED_REGISTER_ACCELERATOR =    12;
    final static int XEMBED_UNREGISTER_ACCELERATOR=   13;
    final static int XEMBED_ACTIVATE_ACCELERATOR  =   14;
    
//     A detail code is required for XEMBED_FOCUS_IN. The following values are valid:    
/* Details for  XEMBED_FOCUS_IN: */
    final static int XEMBED_FOCUS_CURRENT	=	0;
    final static int XEMBED_FOCUS_FIRST 	=	1;
    final static int XEMBED_FOCUS_LAST	=	2;    

    static XAtom XEmbedInfo;
    static XAtom XEmbed;

    XEmbed() {
        if (XEmbed == null) {
            XEmbed = XAtom.get("_XEMBED");
            if (xembedLog.isLoggable(Level.FINER)) xembedLog.finer("Created atom " + XEmbed.toString());
        }        
        if (XEmbedInfo == null) {
            XEmbedInfo = XAtom.get("_XEMBED_INFO");
            if (xembedLog.isLoggable(Level.FINER)) xembedLog.finer("Created atom " + XEmbedInfo.toString());
        }
    }

    void sendMessage(long window, int message) {
        sendMessage(window, message, 0, 0, 0);
    }
    void sendMessage(long window, int message, long detail, long data1, long data2) {
        XClientMessageEvent msg = new XClientMessageEvent();
        msg.set_type((int)XlibWrapper.ClientMessage);
        msg.set_window(window);
        msg.set_message_type(XEmbed.getAtom());
        msg.set_format(32);
        msg.set_data(0, XToolkit.getCurrentServerTime());
        msg.set_data(1, message);
        msg.set_data(2, detail);
        msg.set_data(3, data1);
        msg.set_data(4, data2);
        try {
            XToolkit.awtLock();
            if (xembedLog.isLoggable(Level.FINE)) xembedLog.fine("Sending " + msg);
            XlibWrapper.XSendEvent(XToolkit.getDisplay(), window, false, XlibWrapper.NoEventMask, msg.pData);
        }
        finally {
            XToolkit.awtUnlock();
        }
        msg.dispose();
    }

    static String msgidToString(int msg_id) {
        switch (msg_id) {
          case XEMBED_EMBEDDED_NOTIFY:
              return "XEMBED_EMBEDDED_NOTIFY";
          case XEMBED_WINDOW_ACTIVATE:
              return "XEMBED_WINDOW_ACTIVATE";
          case XEMBED_WINDOW_DEACTIVATE:
              return "XEMBED_WINDOW_DEACTIVATE";
          case XEMBED_FOCUS_IN:
              return "XEMBED_FOCUS_IN";
          case XEMBED_FOCUS_OUT:
              return "XEMBED_FOCUS_OUT";
          case XEMBED_REQUEST_FOCUS:
              return "XEMBED_REQUEST_FOCUS";
          case XEMBED_FOCUS_NEXT:
              return "XEMBED_FOCUS_NEXT";
          case XEMBED_FOCUS_PREV:
              return "XEMBED_FOCUS_PREV";
          case XEMBED_MODALITY_ON:
              return "XEMBED_MODALITY_ON";
          case XEMBED_MODALITY_OFF:
              return "XEMBED_MODALITY_OFF";
          case XEMBED_REGISTER_ACCELERATOR:
              return "XEMBED_REGISTER_ACCELERATOR";
          case XEMBED_UNREGISTER_ACCELERATOR:
              return "XEMBED_UNREGISTER_ACCELERATOR";
          case XEMBED_ACTIVATE_ACCELERATOR:
              return "XEMBED_ACTIVATE_ACCELERATOR";
          default:
              return "unknown XEMBED id " + msg_id;
        }
    }
    static String focusIdToString(int focus_id) {
        switch(focus_id) {
          case XEMBED_FOCUS_CURRENT:
              return "XEMBED_FOCUS_CURRENT";
          case XEMBED_FOCUS_FIRST:
              return "XEMBED_FOCUS_FIRST";
          case XEMBED_FOCUS_LAST:
              return "XEMBED_FOCUS_LAST";
          default:
              return "unknown focus id " + focus_id;
        }
    }
}
