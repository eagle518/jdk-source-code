/*
 * @(#)SingleInstanceManager.java	1.12 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.si;

import java.net.*;
import java.io.*;
import java.util.StringTokenizer;
import java.lang.NumberFormatException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

// This is the Java Implementation of the Single Instance Service.
// This code will only be used if someone tries to use Single Instance
// Service by invoking "javaws http://jnlp.url" - one example of it
// is invoking from the Java Web Start application manager.

public class SingleInstanceManager {

    static private String _idString;

    static private int _currPort;

    // returns true if single instance server is running for the idString
    public static boolean isServerRunning(String idString) {
	File siDir = new File(SingleInstanceImpl.SI_FILEDIR);
	String[] fList = siDir.list();
	if (fList != null) {
	    for (int i = 0; i < fList.length; i++) {
		// if file with the same prefix already exist, server is running
		if (fList[i].startsWith(
		    SingleInstanceImpl.getSingleInstanceFilePrefix(idString))) {
		    try {
			_currPort = Integer.parseInt(
			    fList[i].substring(fList[i].lastIndexOf('_') + 1));
		    } catch (NumberFormatException nfe) {
			Trace.ignoredException(nfe);
			return false;
		    }
		    Trace.println("server running at port: " + _currPort, 
				   TraceLevel.TEMP);
		    _idString = idString;
		    return true;
		}
	    }
	}
	return false;
    }

    // returns true if we connect successfully to the server for the idString
    public static boolean connectToServer(String outputString) {
	Trace.println("connect to: " + _idString + " " + _currPort, 
			TraceLevel.TEMP);
	//Now we open the tcpSocket and the stream
	try {
	    Socket s_socket = new Socket("127.0.0.1",_currPort);
	  
	    PrintStream out = new PrintStream(s_socket.getOutputStream());
	    BufferedReader br = new BufferedReader(
		new InputStreamReader(s_socket.getInputStream()));

	    // send MAGICWORD
	    out.println(SingleInstanceImpl.SI_MAGICWORD);
	    
	    // send over the jnlp file	   
	    out.println(outputString);
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
	    Trace.println("no server is running - continue launch!", 
		TraceLevel.TEMP);
	    return false;
	} catch (java.net.SocketException se) {
	    // for windows
	    // no server is running - continue launch
	    Trace.println("no server is running - continue launch!", 
		TraceLevel.TEMP);
	} catch (Exception ioe) {
	    ioe.printStackTrace();
	}
	return false;
    }

}
