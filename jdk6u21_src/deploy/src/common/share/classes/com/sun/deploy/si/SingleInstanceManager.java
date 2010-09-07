/*
 * @(#)SingleInstanceManager.java	1.18 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.si;

import java.net.*;
import java.io.*;
import java.util.StringTokenizer;
import java.lang.NumberFormatException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;

// This is the Java Implementation of the Single Instance Service.
// This code will only be used if someone tries to use Single Instance
// Service by invoking "javaws http://jnlp.url" - one example of it
// is invoking from the Java Web Start application manager.

public class SingleInstanceManager {

    static private final boolean DEBUG = false;

    static private String _idString;

    static private int _currPort;
    static private String _randomNumberString = null;
    
    static private String _openPrintFilePath = null;
    static private String _actionName = null;
    
    static public void setActionName(String s) {
        if (s == null || s.equals("-open") || s.equals("-print")) {
            _actionName = s;
        }
    }
    
    static public String getActionName() { return _actionName; }
    
    static public void setOpenPrintFilePath(String s) { _openPrintFilePath = s; }
    static public String getOpenPrintFilePath() { return _openPrintFilePath; }

    // returns true if single instance server is running for the idString
    public static boolean isServerRunning(String idString) {
        if(DEBUG)
            System.out.println("isServerRunning ? : "+idString);
        File siDir = new File(SingleInstanceImpl.SI_FILEDIR);
        String[] fList = siDir.list();
        if (fList != null) {
            for (int i = 0; i < fList.length; i++) {
                if(DEBUG) {
                    System.out.println("isServerRunning: "+i+": "+fList[i]);
                    System.out.println("\t sessionString: "+Config.getInstance().getSessionSpecificString());
                    System.out.println("\t SingleInstanceFilePrefix: "+SingleInstanceImpl.getSingleInstanceFilePrefix(idString +
                                 Config.getInstance().getSessionSpecificString()));
                }
                // if file with the same prefix already exist, server is 
                // running
                if (fList[i].startsWith(
                    SingleInstanceImpl.getSingleInstanceFilePrefix(idString +
                     Config.getInstance().getSessionSpecificString()))) {
                    try {
                        _currPort = Integer.parseInt(
                            fList[i].substring(fList[i].lastIndexOf('_') + 1));
                            if(DEBUG)
                                System.out.println("isServerRunning: "+i+": port: "+_currPort);
                    } catch (NumberFormatException nfe) {
                        if(DEBUG)
                            System.out.println("isServerRunning: "+i+": port parsing failed");
                        Trace.ignoredException(nfe);
                        return false;
                    }
                    Trace.println("server running at port: " + _currPort);
                    File siFile = new File(SingleInstanceImpl.SI_FILEDIR, 
                            fList[i]);
                    BufferedReader br = null;
                    // get random number from single instance file
                    try {
                        br = new BufferedReader(new FileReader(siFile));                        
                        _randomNumberString = br.readLine();
                        if(DEBUG)
                            System.out.println("isServerRunning: "+i+": magic: "+_randomNumberString);
                    } catch (IOException ioe ) {
                        if(DEBUG)
                            System.out.println("isServerRunning: "+i+": reading magic failed");
                        Trace.ignoredException(ioe);
                    } finally {
                        try {
                            if (br != null) {
                                br.close();
                            }
                        } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                        }
                    }
                    if(DEBUG)
                        System.out.println("isServerRunning: "+i+": setting id - OK");
                    _idString = idString;
                    return true;
                } else {
                    if(DEBUG)
                        System.out.println("isServerRunning: "+i+": prefix NOK");
                }
            }
        } else {
            if(DEBUG)
                System.out.println("isServerRunning: empty file list");
        }
        if(DEBUG)
            System.out.println("isServerRunning: false");
        return false;
    }

    // returns true if we connect successfully to the server for the idString
    public static boolean connectToServer(String outputString) {
        Trace.println("connect to: " + _idString + " " + _currPort, TraceLevel.TEMP);
        
        if (_randomNumberString == null) {
            // should not happen
            Trace.println("MAGIC number is null, bail out", TraceLevel.TEMP);
            return false;
        }
        
        //Now we open the tcpSocket and the stream
        try {
            Socket s_socket = new Socket("127.0.0.1",_currPort);
          
            PrintStream out = new PrintStream(s_socket.getOutputStream());
            BufferedReader br = new BufferedReader(
                new InputStreamReader(s_socket.getInputStream()));
            
            // send random number
            out.println(_randomNumberString);

            // check and see if javaws command line contains open/print action
            String openPrintFilePath = getOpenPrintFilePath();
            String actionName = getActionName();
            
            if (openPrintFilePath != null && actionName != null) {
                // send open/print command to single instance server
                out.println(SingleInstanceImpl.SI_MAGICWORD_OPENPRINT);
                out.println(actionName);
                out.println(openPrintFilePath);
            } else {
                
                // send MAGICWORD
                out.println(SingleInstanceImpl.SI_MAGICWORD);
                
                // send over the jnlp file
                out.println(outputString);
            }
            // indicate end of file transmission
            out.println(SingleInstanceImpl.SI_EOF);
            out.flush();         

            // wait for ACK (OK) response
            Trace.println("waiting for ack", TraceLevel.TEMP);
            int tries = 5;
            
            // try to listen for ACK
            for (int i=0; i<tries; i++) {                
                String str = br.readLine();
                if (str != null && str.equals(SingleInstanceImpl.SI_ACK)) {
                    Trace.println("GOT ACK", TraceLevel.TEMP);                  
                    s_socket.close();
                    return true;
                }
            }
            s_socket.close();
        
        } catch (java.net.ConnectException ce) {
            // for solaris
            // no server is running - continue launch
            Trace.println("no server is running - continue launch!", TraceLevel.TEMP);
            return false;
        } catch (java.net.SocketException se) {
            // for windows
            // no server is running - continue launch
            Trace.println("no server is running - continue launch!", TraceLevel.TEMP);
        } catch (Exception ioe) {
            ioe.printStackTrace();
        }
        Trace.println("no ACK from server, bail out", TraceLevel.TEMP);
        return false;
    }

}
