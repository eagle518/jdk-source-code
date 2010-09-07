/*
 * @(#)Client.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import  java.util.*;
import  java.net.*;
import  java.io.*;

public class Client
{
  private final static int BYTESPEROP= PollingServer.BYTESPEROP;
  private final static int PORTNUM   = PollingServer.PORTNUM;
  private final static int MAXCONN   = PollingServer.MAXCONN;

  private static Socket[] sockArr = new Socket[MAXCONN];
  private static int totalConn =10;
  private static int bytesToSend =1024000;
  private static int connections = 0;
  private static int sends = 0;

  public static void main (String args[]) {

    String host = "localhost";

    if (args.length < 1 || args.length > 3) {
      System.out.println("Usage : java Client <num_connects>");
      System.out.println("      | java Client <num_connects> <server_name>");
      System.out.println("      | java Client <num_connects> <server_name>" +
			 " <max_Kbytes>");
      System.exit(-1);
    }

    if (args.length >= 1)
      totalConn = java.lang.Integer.valueOf(args[0]).intValue();
    if (args.length >= 2)
      host = args[1];
    if (args.length == 3)
      bytesToSend = java.lang.Integer.valueOf(args[2]).intValue() * 1024;


    if (totalConn <= 0 || totalConn > MAXCONN) {
      System.out.println("Connections out of range.  Terminating.");
      System.exit(-1);
    }

    System.out.println("Using " + totalConn + " connections for sending " +
		       bytesToSend + " bytes to " + host);
    

    try {
      Socket ctrlSock = new Socket (host, PORTNUM);
      PrintStream ctrlStream =
	new PrintStream(ctrlSock.getOutputStream());
      ctrlStream.println(bytesToSend);
      ctrlStream.println(totalConn);
      
      while (connections < totalConn ) {
	sockArr[connections] = new Socket (host, PORTNUM);
	connections ++;
      }
      System.out.println("Connections made : " + connections);
      
      byte[] buff = new byte[BYTESPEROP];
      for (int i = 0; i < BYTESPEROP; i++) // just put some junk in!
	buff[i] = (byte) i;
      
      Random rand = new Random(5321L);
      while (sends < bytesToSend/BYTESPEROP) {
	int idx = java.lang.Math.abs(rand.nextInt()) % totalConn;
	sockArr[idx].getOutputStream().write(buff,0,BYTESPEROP);
	sends++;
      }
      // Wait for server to say done.
      int bytes = ctrlSock.getInputStream().read(buff, 0, BYTESPEROP);
      System.out.println (" Total connections : " + connections +
			  " Bytes sent : " + sends * BYTESPEROP +
			  "...Done!");
    } catch (Exception e) { e.printStackTrace(); }
  }
}
