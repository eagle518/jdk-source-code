/*
 * @(#)SocketTraceListener.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.util;

import java.io.PrintStream;
import java.io.IOException;
import java.io.BufferedOutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

public class SocketTraceListener implements TraceListener {

    private String host;
    private int port;
    private PrintStream socketTraceStream = null;
    private Socket socket;

    public SocketTraceListener(String host, int port) {
	this.host = host;
	this.port = port;
	init();
    }

    // setup socket
    private void init() {
	try {
	    socket = new Socket(host,port);
	    socketTraceStream = new PrintStream(new BufferedOutputStream(socket.getOutputStream()));
	} catch (UnknownHostException uhe) {
	    uhe.printStackTrace();
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
    }

    public Socket getSocket() {
	return socket;
    }
    
    public void print(String msg) {	
	// send the msg to the socket specified by the user
	if (socketTraceStream == null) {
	    return;
	}
	socketTraceStream.print(msg);
	socketTraceStream.flush();
    }
}
