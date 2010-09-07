/*
 * @(#)DomainSocketNamedPipe.java	1.3 08/02/11
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc.unix;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.util.*;

import sun.plugin2.ipc.*;

import com.sun.deploy.net.socket.UnixDomainSocket;
import com.sun.deploy.net.socket.UnixDomainSocketException;

import com.sun.deploy.util.SystemUtils;

public class DomainSocketNamedPipe extends NamedPipe {
    private String sockServerName;
    private UnixDomainSocket sockServer;
    private boolean iAmServer;

    private Object connectionSync = new Object();
    private boolean connectFailed=false;
    private IOException connectException=null;
    private volatile UnixDomainSocket sockClient;
    private boolean connectionThreadDone=false;

    public DomainSocketNamedPipe(String _sockServerName) {
        if(!UnixDomainSocket.isSupported()) {
            throw new RuntimeException("UnixDomainSocket not supported"); // bail out
        }
        this.sockServerName = _sockServerName;
        this.sockServer     = null;
        this.iAmServer = (null==_sockServerName);
        this.sockClient     = null;

        if (iAmServer) {
            // Create new named pipes (server socket)
            try {
                sockServer = UnixDomainSocket.CreateServerBindListen(0, 1);
            } catch (UnixDomainSocketException e) {
                throw new RuntimeException("Error creating AF_UNIX: "+e.getMessage()); // bail out
            }
        }
        startConnectThread();
    }

    private void startConnectThread() {
        new Thread(new Runnable() {
            public void run() {
                try {
                    if ( iAmServer ) {
                        sockClient = sockServer.accept();
                    } else {
                        sockClient = UnixDomainSocket.CreateClientConnect(sockServerName, false, 0);
                        // Since we have a 1:1 client/server connection,
                        // we take our chance to register for pipe file deletion.
                        // Problem: The browser JVM is not shut down properly.
                        sockClient.deleteFileOnClose();
                    }
                    synchronized ( connectionSync ) {
                        connectionThreadDone=true;
                        connectionSync.notifyAll();
                    }
                } catch (IOException e) {
                    synchronized ( connectionSync ) {
                        connectException = e;
                        connectFailed = true;
                        connectionThreadDone=true;
                        connectionSync.notifyAll();
                    }
                }
            }
        }, "Pipe Connector Thread").start();
    }

    private void waitForConnectionThread() throws IOException {
        // wait until the connection is established ..
        if(!connectionThreadDone) {
            synchronized ( connectionSync ) {
                while(!connectionThreadDone) {
                    try {
                        connectionSync.wait();
                    } catch (InterruptedException ie) {}
                }
            }
        }
        if (connectFailed) {
            if (connectException != null) {
                throw connectException;
            } else {
                throw new IOException("Never received connection from client side");
            }
        }
    }

    /** 
     * blocking, i.e. don't leave without having read any bytes
     */
    public int read(ByteBuffer dest) throws IOException {
        waitForConnectionThread();
        int nT=0;
        while ( nT==0 ) {
            int n;
            long t0 = SystemUtils.microTime();
            try {
                // blocking message read ..
                n=sockClient.read(dest);
            } catch (UnixDomainSocketException udse) {
                long t1 = SystemUtils.microTime();
                throw new IOException("Error reading from AF_UNIX: "+udse.getMessage()+
                                      ", read ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
            } catch (IllegalArgumentException iae) {
                // quietly bail out .. sockClient may became null
                break;
            }
            nT+=n;
        }
        return nT;
    }

    /** 
     * blocking, i.e. don't leave until the whole ByteBuffer has been written
     */
    public int write(ByteBuffer src) throws IOException {
        waitForConnectionThread();
        int nT=0;
        while ( src.hasRemaining() ) {
            int n;
            long t0 = SystemUtils.microTime();
            try {
                // blocking message write ..
                n=sockClient.write(src);
            } catch (UnixDomainSocketException udse) {
                long t1 = SystemUtils.microTime();
                throw new IOException("Error writing to AF_UNIX: "+udse.getMessage()+
                                      ", write ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
            } catch (IllegalArgumentException iae) {
                // quietly bail out .. sockClient may became null
                break;
            }
            nT+=n;
        }
        return nT;
    }

    public void close() throws IOException {
        if(null!=sockClient) {
            sockClient.close();
        }
        if (null!=sockServer) {
            sockServer.close();
        }
        synchronized ( connectionSync ) {
            connectFailed = true;
            connectionThreadDone=true;
            connectionSync.notifyAll();
        }
    }

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    public boolean isOpen() {
        return null!=sockClient && sockClient.isOpen();
    }

    public String toString() {
        if(iAmServer) {
            return "UnixNamedPipe: serverSocket: " + sockServer+
                                ", clientSocket: " + sockClient;
        }
        return "UnixNamedPipe: clientSocket: " + sockClient;
    }

    public Map getChildProcessParameters() {
        Map ret = new HashMap();
        ret.put("write_pipe_name", sockServer.getFilename());
        return ret;
    }
}
