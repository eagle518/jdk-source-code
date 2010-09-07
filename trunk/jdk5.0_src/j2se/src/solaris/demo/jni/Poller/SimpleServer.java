/*
 * @(#)SimpleServer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.io.*;
import java.net.*;
import java.lang.Byte;

/**
 * Simple Java "server" using a single thread to handle each connection.
 */

public class SimpleServer
{
  private final static int BYTESPEROP= PollingServer.BYTESPEROP;
  private final static int PORTNUM   = PollingServer.PORTNUM;
  private final static int MAXCONN   = PollingServer.MAXCONN;

  /*
   * This synchronization object protects access to certain
   * data (bytesRead,eventsToProcess) by concurrent Consumer threads.
   */
  private final static Object eventSync = new Object();

  private static InputStream[] instr = new InputStream[MAXCONN];
  private static int bytesRead;
  private static int bytesToRead;

  public SimpleServer() {
    Socket[] sockArr = new Socket[MAXCONN];
    long timestart, timestop;
    int bytes;
    int totalConn=0;


    System.out.println ("Serv: Initializing port " + PORTNUM);
    try {

      ServerSocket skMain = new ServerSocket (PORTNUM);

      bytesRead = 0;
      Socket ctrlSock = skMain.accept();

      BufferedReader ctrlReader =
	new BufferedReader(new InputStreamReader(ctrlSock.getInputStream()));
      String ctrlString = ctrlReader.readLine();
      bytesToRead = Integer.valueOf(ctrlString).intValue();
      ctrlString = ctrlReader.readLine();
      totalConn = Integer.valueOf(ctrlString).intValue();

      System.out.println("Receiving " + bytesToRead + " bytes from " +
			 totalConn + " client connections");
      
      timestart = System.currentTimeMillis();

      /*
       * Take connections, spawn off connection handling threads
       */
      ConnHandler[] connHA = new ConnHandler[MAXCONN];
      int conn = 0;
      while ( conn < totalConn ) {
	  Socket sock = skMain.accept();
	  connHA[conn] = new ConnHandler(sock.getInputStream());
	  connHA[conn].start();
	  conn++;
      }

      while ( bytesRead < bytesToRead ) {
	  java.lang.Thread.sleep(500);
      }
      timestop = System.currentTimeMillis();
      System.out.println("Time for all reads (" + totalConn +
			 " sockets) : " + (timestop-timestart));
      // Tell the client it can now go away
      byte[] buff = new byte[BYTESPEROP];
      ctrlSock.getOutputStream().write(buff,0,BYTESPEROP);
    } catch (Exception exc) { exc.printStackTrace(); }
  }

  /*
   * main ... just create invoke the SimpleServer constructor.
   */
  public static void main (String args[])
  {
    SimpleServer server = new SimpleServer();
  }

  /*
   * Connection Handler inner class...one of these per client connection.
   */
  class ConnHandler extends Thread {
    private InputStream instr;
    public ConnHandler(InputStream inputStr) { instr = inputStr; }

    public void run() {
      try {
	int bytes;
	byte[] buff = new byte[BYTESPEROP];
	
	while ( bytesRead < bytesToRead ) {
	  bytes = instr.read (buff, 0, BYTESPEROP);
	  if (bytes > 0 ) {
	    synchronized(eventSync) {
	      bytesRead += bytes; 
	    }
	    /*
	     * Any real server would do some synchronized and some
	     * unsynchronized work on behalf of the client, and
	     * most likely send some data back...but this is a
	     * gross oversimplification.
	     */
	  }
	  else {
	    if (bytesRead < bytesToRead)
	      System.out.println("instr.read returned : " + bytes);
	  }
	}
      }
      catch (Exception e) {e.printStackTrace();}
    }
  }
}  

