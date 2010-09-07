/*
 * @(#)ClientPrintHelper.java	1.6 10/03/24 12:02:03
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import sun.plugin2.util.*;
import sun.plugin2.applet.*;
import sun.plugin2.main.client.*;
import sun.plugin2.message.*;
import sun.awt.windows.*;
import sun.awt.*;
import java.io.*;
import java.applet.Applet;


/** Helps abstract platform dependent code for applet printing calling from Plugin client side. 
 */

public class ClientPrintHelper {

    private ClientPrintHelper() {}

    /** Printing applet by band. 
     */
    public static boolean print(Plugin2Manager manager, int appletID, long hdc, boolean isPrinterDC, 
                                int x, int y, int width, int height, Pipe pipe) {
        boolean res = false;

        if (manager != null) {
            EmbeddedFrame container = (EmbeddedFrame) manager.getAppletParentContainer();        

            if (container != null) {
                
                Pipe localPipe = pipe;
                if (localPipe != null) {
                    Conversation c = pipe.beginConversation();
                    try {
                        if (Class.forName("sun.plugin2.main.client.PluginEmbeddedFrame").isInstance(container)) {

                            switch (SystemUtil.getOSType()) {

                                case SystemUtil.WINDOWS: {
                                                           
                                    PrintBandReplyMessage reply = null;
                                    PrintBandDescriptor bandInfo = null;
                                    boolean isLastBand = false;

                                    if (manager.isAppletStarted()) {
                                        java.lang.reflect.Method m = 
                                            (Class.forName("sun.plugin2.main.client.PluginEmbeddedFrame")).
                                            getMethod("printPlugin", new Class[] { PrintBandDescriptor.class, 
                                                                                   Boolean.TYPE });
                                        
                                        if (m != null) {
                                            do {
                                                if (!isLastBand) {
                                                    Object[] args = new Object[2];
                                                    args[0] = bandInfo;
                                                    args[1] = new Boolean(isPrinterDC);
                                                    bandInfo = (PrintBandDescriptor) (m.invoke(container, args));

                                                    if (bandInfo != null) {
                                                        isLastBand = bandInfo.isLastBand();
                                                    } else { // If gets here, it's an error
                                                        break;
                                                    }
                                                } // !isLastBand

                                                localPipe.send(new PrintBandMessage(c, appletID, hdc,
                                                    bandInfo.getData(), bandInfo.getOffset(),
                                                    bandInfo.getSrcX(), bandInfo.getSrcY(),
                                                    bandInfo.getSrcWidth(), bandInfo.getSrcHeight(),
                                                    bandInfo.getDestX(), bandInfo.getDestY(),
                                                    bandInfo.getDestWidth(), bandInfo.getDestHeight()));

                                                reply = (PrintBandReplyMessage) localPipe.receive(0, c);
                                                if ( reply != null) {
                                                        if ( reply.getAppletID() != appletID && 
														    reply.getDestY() != bandInfo.getDestY()) {
                                                            break;
                                                        }
                                                        res = reply.getRes();
                                                }

                                                if (isLastBand) { // finish printing
                                                    res = true;
                                                    break;
                                                }

                                            } while (bandInfo != null && manager.isAppletStarted());
                                        } // m!=null
                                    } // isAppletStarted()
                                    break;
                                }

                                case SystemUtil.UNIX: {
                                    PrintBandReplyMessage reply = null;
                                    byte[] data = null;

                                    if (manager.isAppletStarted()) {

                                        java.lang.reflect.Method m = 
                                            (Class.forName("sun.plugin2.main.client.PluginEmbeddedFrame")).
                                             getMethod("printPlugin", new Class[] { Applet.class,
                                                                                   Integer.TYPE, Integer.TYPE,
                                                                                   Integer.TYPE, Integer.TYPE });

                                        if (m != null) {
                                            Object[] args = new Object[5];
                                            args[0] = manager.getApplet();
                                            args[1] = new Integer(x);
                                            args[2] = new Integer(y);
                                            args[3] = new Integer(width);
                                            args[4] = new Integer(height);
                                            data = (byte[]) (m.invoke(container, args));
                                        } else { // If gets here, it's an error
                                            break;
                                        } 

                                        localPipe.send(new PrintBandMessage(c, appletID, hdc,
                                            data, 0,
                                            0, 0, 0, 0,
                                            0, 0, 0, 0));

                                        reply = (PrintBandReplyMessage) localPipe.receive(0, c);
                                        if ( reply != null) {
                                            if ( reply.getAppletID() != appletID ) {
                                                break;
                                            }
                                            res = reply.getRes();
                                        }

                                    } // isAppletStarted()
                                
                                    break;
                                }

                                default:
                                    break;
                            } // end switch
                        } // container instanceof PluginEmbeddedFrame
                    } catch (Throwable t) {
                        t.printStackTrace();
                    } finally {
                        localPipe.endConversation(c);
                    }

                } // localPipe!=null
                
            } // container!=null
        }  // manager != null

        return res;

    }

}

