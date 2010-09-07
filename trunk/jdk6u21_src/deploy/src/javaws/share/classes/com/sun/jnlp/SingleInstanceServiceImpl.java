/*
 * @(#)SingleInstanceServiceImpl.java	1.23 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.net.*;
import javax.jnlp.*;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.XMLFormat;
import com.sun.javaws.jnl.ApplicationDesc;
import com.sun.javaws.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.si.*;
import java.security.AccessController;
import java.security.PrivilegedAction;

public final class SingleInstanceServiceImpl extends SingleInstanceImpl implements SingleInstanceService {
    static private SingleInstanceServiceImpl _sharedInstance = null;
    static private boolean listenerExists = false;

    public static synchronized SingleInstanceServiceImpl getInstance() {
        if (_sharedInstance == null) {
            _sharedInstance = new SingleInstanceServiceImpl();
        }
        return _sharedInstance;
    }


    public synchronized void addSingleInstanceListener(javax.jnlp.SingleInstanceListener sil) {
        if (sil == null) return;

        // generate the unique jnlp url
        final LaunchDesc ld = JNLPClassLoaderUtil.getInstance().getLaunchDesc();
        URL urlNoQuery = ld.getCanonicalHome();
        int index = urlNoQuery.toString().lastIndexOf('?');
        if (index != -1) {
            try {            
                urlNoQuery = new URL(urlNoQuery.toString().substring(0, index));
            } catch (MalformedURLException mue) {
                // should not happen
                Trace.ignoredException(mue);
            }
        }

        final String jnlpUrlString = urlNoQuery.toString();

        // if there is no listener added by this application before, first try
        // to see if there is already a single instance server running;
        // otherwise just go ahead and add the listener
        if (listenerExists == false) {
            AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        if (SingleInstanceManager.isServerRunning(
                               jnlpUrlString)) {                        
                
                            String[] appArgs = Globals.getApplicationArgs();
                        
                            if (appArgs != null) {
				// FIXME: in the context of a dragged-out applet, the 
				// appDesc will be null. For now, adding a null check
				// for appDesc to avoid NPE. We need to investigate how to 
				// setup AppletDesc for the dragged-out applet case and 
				// whether it makes sense to add a setArguments method to AppletDesc.
				ApplicationDesc appDesc = ld.getApplicationDescriptor();
				if (appDesc != null) {
				    appDesc.setArguments(appArgs);
				}
                            }
                            // send the JNLP file to the server port
                            if (SingleInstanceManager.connectToServer(
                                   ld.toString())) {

                                // if we get OK from server, we are done
                                try {
                                    Main.systemExit(0);
                                } catch (ExitException ee) { 
                                    Trace.println("systemExit: "+ee, TraceLevel.BASIC);
                                    Trace.ignoredException(ee);
                                }
                            }
                            // else continue                            
                        }
                        return null;
                    }
                });            
        }

        super.addSingleInstanceListener(new TransferListener(sil), jnlpUrlString);
        listenerExists = true;

    }

    public void removeSingleInstanceListener(javax.jnlp.SingleInstanceListener sil) {
        super.removeSingleInstanceListener(new TransferListener(sil));
    }


    // overrides isSame in com.sun.deploy.SingleInstanceImpl
    public boolean isSame(String inputString, String idString) {
        LaunchDesc ld = null;
        // extract the jnlp href
        try {
            ld = XMLFormat.parse(inputString.getBytes(), null, null, null);
        } catch (Exception e) {
            Trace.ignoredException(e);
        }

        if (ld != null) {
            URL urlNoQuery = ld.getCanonicalHome();
            int index = urlNoQuery.toString().lastIndexOf('?');
            if (index != -1) {
                try {            
                    urlNoQuery = new URL(urlNoQuery.toString().substring(0, index));
                } catch (MalformedURLException mue) {
                    // should not happen
                    Trace.ignoredException(mue);
                }
            }
            String sessionID = urlNoQuery.toString() +
                Config.getInstance().getSessionSpecificString();
            Trace.println("GOT: " + sessionID, TraceLevel.BASIC);

            if (idString.equals(sessionID)) {
                return true;
            }
        }
        return false;

    }

    // overrides getArguments in com.sun.deploy.SingleInstanceImpl
    public String[] getArguments(String inputString, String idString) {
        LaunchDesc ld = null; 
        // extract the jnlp href 
        try { 
            ld = XMLFormat.parse(inputString.getBytes(), null, null, null); 
        } catch (Exception e) { 
            Trace.ignoredException(e); 
        } 
 
        // IF valid LaunchDesc THEN
        //  IF isApplication() THEN return the application parameters
        //  ELSE return LaunchDesc itself (input string).
        // Else, we return null
        if (ld != null ) {
            if ( ld.isApplication() ) { 
                return ld.getApplicationDescriptor().getArguments();
            } 
            return super.getArguments(inputString, idString);
        }
        return new String[0];
    }

    private class TransferListener implements DeploySIListener {
        SingleInstanceListener _sil;

        public TransferListener( SingleInstanceListener sil) {
            _sil = sil;
        }

        public void newActivation(String [] params) {
            if (params.length == 2) {
                // check and see if there is open/print command coming from
                // javaws command line
                String filePath = SingleInstanceManager.getOpenPrintFilePath();
                String actionName = SingleInstanceManager.getActionName();
                // allows open/print of specific file, if the open/print
                // action is coming from javaws command file
                if (filePath != null && actionName != null &&
                        (actionName.equals("-open") ||
                        actionName.equals("-print"))) {
                    if (filePath.equals(params[1]) &&
                            actionName.equals(params[0])) {
                        JnlpxArgs.getFileReadWriteList().add(params[1]);
                        SingleInstanceManager.setOpenPrintFilePath(null);
                        SingleInstanceManager.setActionName(null);
                    }
                }
            }
            _sil.newActivation(params);
        }

        public Object getSingleInstanceListener() {
            return _sil;
        }
    }

}
