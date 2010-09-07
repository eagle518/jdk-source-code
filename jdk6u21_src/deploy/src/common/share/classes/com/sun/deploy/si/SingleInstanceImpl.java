/*
 * @(#)SingleInstanceImpl.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.si;

import java.net.*;
import java.io.*;
import java.util.*;
import java.security.PrivilegedAction;
import java.security.AccessController;
import java.security.SecureRandom;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.services.ServiceManager;

public class SingleInstanceImpl {
   
    public static final String SI_FILEDIR = Config.getTempDirectory() + File.separator + "si" + File.separator;
    public static final String SI_MAGICWORD = "javaws.singleinstance.init";
    public static final String SI_MAGICWORD_OPENPRINT = "javaws.singleinstance.init.openprint";
    public static final String SI_ACK = "javaws.singleinstance.ack";
    public static final String SI_STOP = "javaws.singleinstance.stop";
    public static final String SI_EOF = "EOF";

    private ArrayList _sil_list = new ArrayList();
    private static boolean _serverStarted = false;
    private Object _lock = new Object();
    private SingleInstanceServer _sis;
    private static int DEFAULT_FILESIZE = Integer.MAX_VALUE;
    
    // Get the platform dependent Random generator
    private static SecureRandom random = null;
    private static int _randomNumber;

    public static String getSingleInstanceFilePrefix(final String url) {
	String filePrefix = url.replace('/','_');
	filePrefix = filePrefix.replace(':','_');
	return filePrefix;
    }
   
    public void addSingleInstanceListener(DeploySIListener sil, String id) {

	if (sil == null) return;

	// start a new server thread for this unique id 
	// first time
	synchronized(_lock) {
	    if (!_serverStarted) {
		Trace.println("unique id: " + id, TraceLevel.BASIC);
		try {
		    String sessionID = id + 
			Config.getInstance().getSessionSpecificString();

		    _sis = new SingleInstanceServer(sessionID, this);
		} catch (Exception e) {
		    Trace.println("addSingleInstanceListener failed", 
				  TraceLevel.BASIC);
		    Trace.ignoredException(e);
		    return;	// didn't start
		}
		_serverStarted = true;
	    }
	}

	synchronized(_sil_list) {
	    // add the sil to the arrayList
	    if (!_sil_list.contains(sil)) {
		_sil_list.add(sil);
	    }	    
	}
	 
    }

    public boolean isSame(String inputString, String idString) {
	// default impl is just true
	return true;
    }

    public String[] getArguments(String inputString, String idString) { 
	// default implementation is one arg, the inputString
	String[] ret = new String[1];
	ret[0] = inputString;
	return ret;
    }
    
    private static SecureRandom getSecureRandom() {
        // Get the platform dependent Random generator
        
        if (random == null) {
            random = ServiceManager.getService().getSecureRandom();
            random.nextInt();
        }
        return random;
    }

  
    class SingleInstanceServer extends Thread {

	ServerSocket _ss;
	int _port;
	String _idString;
	String[] _arguments;
	SingleInstanceImpl _impl;

	int getPort() {
	    return _port;
	}

	SingleInstanceServer(String idString, SingleInstanceImpl impl) 
		throws IOException {
	    _idString = idString;
	    _impl = impl;
	  
	    // open a free ServerSocket
	    _ss = null;
	    // fix for 5047763: XP SP2 puts up warning dialog
	    // we should bind the server to the local InetAddress 127.0.0.1
	    _ss = new ServerSocket(0, 0, InetAddress.getByName("127.0.0.1"));

	    // get the port number
	    _port = _ss.getLocalPort();
	    Trace.println("server port at: " + _port, TraceLevel.BASIC);
	    
	    this.start();

	    // create the single instance file with canonical home and port number	    
	    createSingleInstanceFile(_idString, _port);
	}

	private String getSingleInstanceFilename(final String id, final int port) {
	    String name = SI_FILEDIR + getSingleInstanceFilePrefix(id) + "_" + port;
	    Trace.println("getSingleInstanceFilename: " + name, TraceLevel.BASIC);
	    return name;
        }

	private void removeSingleInstanceFile(final String id, final int port) {
	    new File(getSingleInstanceFilename(id, port)).delete();
	    Trace.println("removed SingleInstanceFile: " + getSingleInstanceFilename(id, port), TraceLevel.BASIC);
	}

	private void createSingleInstanceFile(final String id, final int port) {
	    String filename = getSingleInstanceFilename(id, port);
	    final File siFile = new File(filename);
	    final File siDir = new File(SI_FILEDIR);
	    AccessController.doPrivileged(new PrivilegedAction() {
	        public Object run() {
		    siDir.mkdirs();
		    String[] fList = siDir.list();
		    if (fList != null) {
		        for (int i = 0; i < fList.length; i++) {
		     
			    // if file with the same prefix already exist, remove it
			    if (fList[i].startsWith(getSingleInstanceFilePrefix(id))) {
			        Trace.println("file should be removed: " + SI_FILEDIR + fList[i], TraceLevel.BASIC);
			        new File(SI_FILEDIR + fList[i]).delete();
			    }
		        }
		    }
		    try {
		        siFile.createNewFile();
                        // write random number to single instance file
                        PrintStream out = new PrintStream(new FileOutputStream(
                                siFile));
                        _randomNumber = 
                                SingleInstanceImpl.getSecureRandom().nextInt();
                        out.print(_randomNumber);
                        out.close();

		    } catch (IOException ioe) {
		        Trace.ignoredException(ioe);
		    }
		    return null;
	        }
	    });
	}
	

	// check to make sure the recvString href matches with the
	// current instance
	boolean isSameInstance(String inputString) {

	    if (_impl.isSame(inputString, _idString)) {
		_arguments = _impl.getArguments(inputString, _idString);
	        return true;
	    }
	    return false;
	}
	
	public void run() {
	    // start sil to handle all the incoming request 
	    // from the server port of the current url
	    AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    while (true) {
			InputStream is = null;
			Socket s = null;
			String line = null;
			String recvString = "";
			boolean sendAck = false;
			int port = -1;
			try {
			    Trace.println("waiting connection", TraceLevel.BASIC);
			    s = _ss.accept();
			    is = s.getInputStream();
			    BufferedReader in = new BufferedReader(new InputStreamReader(is));
                            // first read the random number
                            line = in.readLine();
                            if (line.equals(String.valueOf(_randomNumber))
                                == false) {
                                // random number does not match
                                // should not happen
                                // shutdown server socket
                                removeSingleInstanceFile(_idString, _port);
                                _ss.close();
                                _serverStarted = false;
                                Trace.println("Unexpected Error, " +
                                        "SingleInstanceService disabled",
                                        TraceLevel.BASIC);
                                return null;
                            } else {
                                line = in.readLine();
                                // no need to continue reading if MAGICWORD
                                // did not come first
                                Trace.println("recv: " + line, TraceLevel.BASIC);
                                if (line.equals(SI_MAGICWORD)) {
                                    Trace.println("got magic word!!!", TraceLevel.BASIC);
                                    while (true) {
                                        // Get input string
                                        try {
                                            line = in.readLine();
                                            
                                            if (line != null && line.equals(SI_EOF)) {
                                                // end of file reached
                                                break;
                                            } else {
                                                recvString += line;
                                            }
                                        } catch (IOException ioe) {
                                            Trace.ignoredException(ioe);
                                        }
                                    }
                                    Trace.println(recvString, TraceLevel.BASIC);
                                    // check and make sure jnlp href matches
                                    if (isSameInstance(recvString)) {
                                        sendAck = true;
                                    }
                                } else if (line.equals(SI_STOP)) {
                                    // remove the SingleInstance file
                                    removeSingleInstanceFile(_idString, _port);
                                    break;
                                } else if (line.equals(SI_MAGICWORD_OPENPRINT)) {
                                    int i = 0;
                                    _arguments = new String[2];
                                    
                                    Trace.println("GOT OPENPRINT MAGICWORD", TraceLevel.BASIC);
                                    // we expect three strings from the socket
                                    // 1.  -open or -print
                                    // 2.  the filename
                                    // 3.  SI_EOF
                                    for (int j = 0; j < 3; j++) {
                                        // Get input string
                                        try {
                                            line = in.readLine();
                                            
                                            if (line != null && line.equals(SI_EOF)) {
                                                // end of file reached
                                                break;
                                            } else {
                                                Trace.println(line, TraceLevel.BASIC);
                                                // for -open/print and filename arg
                                                if ( i < 2 ) {
                                                    _arguments[i] = line;
                                                    i++;
                                                }
                                            }
                                        } catch (IOException ioe) {
                                            Trace.ignoredException(ioe);
                                        }
                                    }
                                    if (i == 2) {
                                        // set action and file name to indicate
                                        // it is coming from javaws command line
                                        SingleInstanceManager.setActionName(
                                                _arguments[0]);
                                        SingleInstanceManager.setOpenPrintFilePath(
                                                _arguments[1]);
                                    }
                                    sendAck = true;
                                }
                            }
			} catch (IOException ioe) {
			    Trace.ignoredException(ioe);	
			} finally {
			    try {
				if (sendAck) {
				   
				    // let the action listener handle the rest
				    for (int i=0; i<_arguments.length; i++) {
					Trace.println("Starting new instance with arguments: " + 
						_arguments[i], TraceLevel.BASIC);
				    }
				    
				    // enumerate the sil list and call
				    // each sil with _arguments
				    ArrayList silal = (ArrayList)_sil_list.clone();

				    Iterator i = silal.iterator();

				    DeploySIListener sil;

				    for (; i.hasNext(); ) {		   
					sil = (DeploySIListener)i.next();
					sil.newActivation(_arguments);
				    }

				    // now the event is handled, we can send
				    // out the ACK
				    Trace.println("sending out ACK..", TraceLevel.BASIC);
				   
				    PrintStream ps = new PrintStream(s.getOutputStream());
				    // send OK (ACK)
				    ps.println(SI_ACK);
				    ps.flush();
				}			
				if (s != null) s.close();
			
			    } catch (IOException ioe) {
				Trace.ignoredException(ioe);
			    }
			}
		    } // while
		    return null;
		}
	    });
	    
	}

    }


    public void removeSingleInstanceListener(DeploySIListener sil) {
	synchronized(_sil_list) {	
	    Object jnlpsil = sil.getSingleInstanceListener();
	    Object cursil = null;
	    int index = -1;
	    for (int i = 0; i < _sil_list.size(); i++) {
		cursil = ((DeploySIListener)_sil_list.get(i)).getSingleInstanceListener();
		if (cursil.equals(jnlpsil)) {
		    index = i;
		    break;
		}
	    }
	   
	    if (index < 0 || index >= _sil_list.size()) return;

	    _sil_list.remove(index);
	
	    if (_sil_list.isEmpty()) {
		 AccessController.doPrivileged(new PrivilegedAction() {
			 public Object run() {
			     // stop server
			     try {
				 Socket s_socket = new Socket("127.0.0.1", _sis.getPort());
				 PrintStream out = new PrintStream(s_socket.getOutputStream());
                                 out.println(_randomNumber);
				 out.println(SingleInstanceImpl.SI_STOP);
				 out.flush();
				 s_socket.close();
				 _serverStarted = false;
			     } catch (IOException ioe) {
				 Trace.ignoredException(ioe);
			     } 
			     return null;
			 }
		     });
	    }
	}
    }
}
