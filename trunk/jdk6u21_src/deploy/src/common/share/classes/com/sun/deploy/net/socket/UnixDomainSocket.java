/*
 * @(#)UnixDomainSocket.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.socket;

import java.lang.IllegalArgumentException;
import java.lang.UnsupportedOperationException;

import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;

import com.sun.deploy.config.Config;

/**
 * Optional implementation of generic Unix Domain Socket functionality
 *
 * Currently supported:
 * <OL>
 *   <LI> Platform: Unix
 * </OL>
 *
 * If a functionality is not supported for a platform,
 * UnsupportedOperationException is thrown.
 *
 * One can also use the method: isSupported()
 * to query if the whole functional block is supported on the running platform.
 *
 * The constructor will open the socket, and the finalize method will
 * close it and delete the pipe name if this is a server socket.
 *
 * All blocking methods: accept(), connect(), read() and write()
 * are not synchronized for a purpose!
 * It is intended to allow these operations concurrently by many threads.
 * Since the native implementations are blocking, this must be fine.
 *
 * Look in the examples section for:
 *
 * @see client1RWThreads A multithreaded client example.
 * @see server1RWThreads A multithreaded server example.
 *
 * @see <a href="http://linux.die.net/man/7/unix">unix(7)</a>
 * @see <a href="http://linux.die.net/man/2/socket">socket(2)</a>
 */
public class UnixDomainSocket
{
    /* our socket file namespace */
    public static final String pipeFileNamePrefix = ".com.sun.deploy.net.socket.";
    public static final String pipeFileNameSuffix = ".AF_UNIX";

    /* reflecting the socket status */
    public static final int STATUS_CLOSE   = 0;
    public static final int STATUS_OPEN    = 1;
    public static final int STATUS_CONNECT = 2;
    public static final int STATUS_BIND    = 3;
    public static final int STATUS_LISTEN  = 4;
    public static final int STATUS_ACCEPT  = 5;

    /**
     * @return true, if the functional block: 'Streaming UnixDomain sockets' is supported.
     */
    public static boolean isSupported()
    {
        return UnixSocketImpl.unStreamSocketSupported();
    }

    /**
     * Convenient grouping of consequent create and connect
     *
     * Function: client, blocking
     *
     * @see #UnixDomainSocket(String,boolean,int)
     * @see #open()
     * @see #connect()
     */
    public static UnixDomainSocket CreateClientConnect(String fileName, boolean abstractNamespace, int protocol)
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        UnixDomainSocket uds = new UnixDomainSocket(fileName, abstractNamespace, protocol);
        uds.connect();
        return uds;
    }

    /**
     * Convenient grouping of consequent create, bind and listen call
     *
     * Function: server, non-blocking
     *
     * @see #UnixDomainSocket(String,boolean,int)
     * @see #open()
     * @see #bind()
     * @see #listen(int)
     */
    public static UnixDomainSocket CreateServerBindListen(String fileName, boolean abstractNamespace, int protocol,
                                                          int backlog) 
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        UnixDomainSocket uds = new UnixDomainSocket(fileName, abstractNamespace, protocol);
        uds.bind();
        uds.listen(backlog);
        return uds;
    }

    /**
     * Convenient grouping of consequent create, bind and listen call
     *
     * Function: server, non-blocking
     *
     * @see #UnixDomainSocket(int)
     * @see #open()
     * @see #bind()
     * @see #listen(int)
     */
    public static UnixDomainSocket CreateServerBindListen(int protocol, int backlog) 
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        UnixDomainSocket uds = new UnixDomainSocket(protocol);
        uds.bind();
        uds.listen(backlog);
        return uds;
    }

    /**
     * Creates and opens a AF_UNIX, STATUS_STREAM socket.
     *
     * Function: server and client, non-blocking.
     *
     * @param fileName the filename for the desired pipe
     * @param abstractNamespace see socket(2)
     * @param protocol see socket(2)
     *
     * @return handle for the new socket connection
     *
     * @throws RuntimeException 
     *         May be thrown by the native implementation,
     * @throws IllegalArgumentException 
     *         If an argument does not match the requirements
     * @throws UnixDomainSocketException 
     *         If AF_UNIX error occures
     * @throws UnsupportedOperationException 
     *         If this is not implemented for the running platform
     *
     * @see #open()
     * @see <a href="http://linux.die.net/man/7/unix">unix(7)</a>
     * @see <a href="http://linux.die.net/man/2/socket">socket(2)</a>
     */
    public UnixDomainSocket(String fileName, boolean abstractNamespace, int protocol)
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        setup(fileName, false, protocol);
    }

    /**
     * Creates and opens a AF_UNIX, STATUS_STREAM socket.
     * 
     * The pipe filename will be created automatically and is unique,
     * using the File.createTempFile and the native pid.
     * The temp file will be deleted first, 
     * then the AF_UNIX socket will be created.
     *
     * Use getFilename() to gather the used pipe filename,
     * your client might need it to connect to you!
     *
     * Function: server, non-blocking.
     *
     * @param protocol see socket(2)
     *
     * @return handle for the new socket connection
     *
     * @throws RuntimeException 
     *         If the temporary pipe file could not be created, or
     *         may be thrown by the native implementation,
     * @throws IllegalArgumentException 
     *         If an argument does not match the requirements
     * @throws UnixDomainSocketException 
     *         If AF_UNIX error occures
     * @throws UnsupportedOperationException 
     *         If this is not implemented for the running platform
     *
     * @see com.sun.deploy.config.Config#getNativePID()
     * @see java.io.File#createTempFile(String,String)
     * @see java.io.File#delete()
     * @see #UnixDomainSocket(String,boolean,int)
     * @see #open()
     * @see #getFilename()
     *
     * @see <a href="http://linux.die.net/man/7/unix">unix(7)</a>
     * @see <a href="http://linux.die.net/man/2/socket">socket(2)</a>
     */
    public UnixDomainSocket(int protocol) 
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        long threadId = Config.getNativePID();
        String pipeFileName=null;

        // try to utilize File.createTempFile()
        try {
            File pipeFile = pipeFile = File.createTempFile(pipeFileNamePrefix + String.valueOf(threadId) + ".",
                                                           pipeFileNameSuffix);
            pipeFileName = pipeFile.getAbsolutePath();
            if(pipeFile.exists()) {
                pipeFile.delete();
            } else {
                // j2se impl error ?
                pipeFileName=null; 
            }
        } catch (Exception e) {
            // j2se impl error ?
            e.printStackTrace();
            pipeFileName=null; 
        }

        // 2nd try to plain /tmp ..
        if(pipeFileName==null) {
            try {
                pipeFileName = new String( "/tmp/" + pipeFileNamePrefix + String.valueOf(threadId) + pipeFileNameSuffix);
                File pipeFile = pipeFile = new File(pipeFileName);
                if(pipeFile.exists()) {
                    pipeFile.delete();  // rare occasion ..
                }
            } catch (Exception e) {
                // j2se impl error ?
                e.printStackTrace();
                pipeFileName=null; 
            }
        }

        // we have to give up ..
        if(pipeFileName==null) {
            throw new RuntimeException("could not create a temp pipe filename");
        }

        setup(pipeFileName, false, protocol);
    }

    /**
     * Opens the socket.
     * This is only legal on manual, if you have closed the socket beforehand.
     * Hence it is to allow you to reopen it.
     *
     * Function: server and client, non-blocking.
     *
     * @see <a href="http://linux.die.net/man/2/socket">socket(2)</a>
     */
    public synchronized void open()
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        validateSocketStatusTransition(STATUS_OPEN);
        socketHandle = UnixSocketImpl.unStreamSocketCreate(this.fileName, this.abstractNamespace, this.protocol);
        socketStatus=STATUS_OPEN;
    }

    /**
     * Closes the socket.
     * If this is a server socket, the pipe file will be deleted.
     *
     * Will be called by finalize()
     *
     * Function: server and client, non-blocking.
     *
     * @see #deleteFileOnClose()
     * @see #finalize()
     * @see java.io.File#delete()
     * @see <a href="http://linux.die.net/man/2/close">close(2)</a>
     */
    public void close()
    {
        socketStatus=STATUS_CLOSE;
        if(0!=socketHandle) {
            long _socketHandle=socketHandle;
            socketHandle=0;
            try {
                UnixSocketImpl.unStreamSocketClose(_socketHandle);
            } catch (Exception e) { }
        }
        if(unlinkFile) {
            try {
                File pipeFile = new File(fileName);
                if(null!=pipeFile) {
                    pipeFile.delete();
                }
            } catch(Exception e) {}
        }
        backlog=0;
    }

    /**
     * Mark this socket's pipe file to be deleted on close()
     * and when the JVM shuts down, System.exit().
     *
     * This only works for pipe files not in the abstractNamspace.
     *
     * This will be done implicitly for server sockets,
     * within the bind().
     *
     * If you do this manually for client sockets,
     * you should know what you are doing,
     * Since you attempt to remove the pipe file.

     * @see #UnixDomainSocket(String,boolean,int)
     * @see #getIsAbstractNamespace()
     * @see #close()
     * @see java.lang.System#exit(int)
     * @see java.io.File#delete()
     */
    public void deleteFileOnClose() {
        if(!unlinkFile && !abstractNamespace) {
            unlinkFile=true;
            shutdownHookUnlinkFiles.add(fileName);
        }
    }

    /**
     * Binds the server socket
     * Function: server, non-blocking
     *
     * @see <a href="http://linux.die.net/man/2/bind">bind(2)</a>
     */
    public synchronized void bind()
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        validateSocketStatusTransition(STATUS_BIND);
        UnixSocketImpl.unStreamSocketBind(socketHandle);
        socketStatus=STATUS_BIND;

        deleteFileOnClose();
    }

    /**
     * Setup listening to the server socket
     * Function: server, non-blocking
     *
     * @see <a href="http://linux.die.net/man/2/listen">listen(2)</a>
     */
    public synchronized void listen(int backlog)
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        validateSocketStatusTransition(STATUS_LISTEN);
        UnixSocketImpl.unStreamSocketListen(socketHandle, backlog);
        this.backlog=backlog;
        socketStatus=STATUS_LISTEN;
    }

    /**
     * Waiting for a client connection
     * Function: server, blocking
     *
     * @return new UnixDomainSocket, representing the connection to the client
     *
     * @see <a href="http://linux.die.net/man/2/accept">accept(2)</a>
     */
    public UnixDomainSocket accept()
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        validateSocketStatusTransition(STATUS_ACCEPT);
        long clientSocketHandle = UnixSocketImpl.unStreamSocketAccept(socketHandle);
        socketStatus=STATUS_ACCEPT;
        return new UnixDomainSocket(this, clientSocketHandle, STATUS_CONNECT);
    }

    /**
     * Connecting to a server socket
     * Function: client, blocking
     *
     * @see <a href="http://linux.die.net/man/2/connect">connect(2)</a>
     */
    public void connect()
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        validateSocketStatusTransition(STATUS_CONNECT);
        UnixSocketImpl.unStreamSocketConnect(socketHandle);
        socketStatus=STATUS_CONNECT;
    }

    /**
     * @see #read(java.nio.ByteBuffer,int)
     * @see <a href="http://linux.die.net/man/2/read">read(2)</a>
     */
    public int read(java.nio.ByteBuffer buffer)
        throws UnixDomainSocketException, BufferOverflowException, UnsupportedOperationException
    {
        return read(buffer, buffer.remaining());
    }

    /**
     * Read from the socket
     * while writing in (put) the given ByteBuffer.
     *
     * We attempt to read count bytes, 
     * where count is lower or equal to ByteBuffer's remaining(): limit - position.
     *
     * The ByteBuffer position will be increased about the actual read bytes.
     *
     * Function: server and client, blocking.
     *
     * @param  buffer the destination ByteBuffer, where we write to
     * @param  count the number of bytes to read, 
     *               which will be corrected to buffer.remaining(), if greater.
     *
     * @return the number of bytes actually read
     *
     * @throws RuntimeException 
     *         May be thrown by the native implementation.
     * @throws IllegalArgumentException 
     *         If an argument does not match the requirements
     * @throws UnixDomainSocketException 
     *         If AF_UNIX error occures
     * @throws BufferOverflowException 
     *         If the ByteBuffer has no remaining() bytes left
     * @throws UnsupportedOperationException 
     *         If this is not implemented for the running platform
     *
     * @see <a href="http://linux.die.net/man/2/read">read(2)</a>
     */
    public int read(java.nio.ByteBuffer buffer, int count)
        throws UnixDomainSocketException, BufferOverflowException, UnsupportedOperationException
    {
        validateSocketStatusForReadWrite();
        if (null==buffer) {
          throw new IllegalArgumentException("Argument buffer is null");
        }
        if (!buffer.isDirect()) {
          throw new IllegalArgumentException("Argument buffer is not direct");
        }

        int limit = buffer.limit();
        int pos   = buffer.position();
        if(pos>=limit) {
          throw new BufferOverflowException();
        }
        if(pos+count>limit) {
            // fix overflow count
            count=limit-pos;
        }

        int n = UnixSocketImpl.unStreamSocketRead(socketHandle, buffer, pos, count );

        if( n > 0 ) {
            buffer.position(pos+n);
        }
        return n;
    }

    /**
     * @see #write(java.nio.ByteBuffer,int)
     * @see <a href="http://linux.die.net/man/2/write">write(2)</a>
     */
    public int write(java.nio.ByteBuffer buffer) 
        throws UnixDomainSocketException, BufferUnderflowException, UnsupportedOperationException
    {
        return write(buffer, buffer.remaining());
    }

    /**
     * Write to the socket,
     * while reading out (get) the given ByteBuffer.
     *
     * We attempt to write count bytes, 
     * where count is lower or equal to ByteBuffer's remaining(): limit - position.
     *
     * The ByteBuffer position will be increased about the actual written bytes.
     *
     * This method is not synchronized for a purpose.
     * It is intended to allow read and write operations concurrently by many threads.
     * Since the native read and write operation itself is blocking, this is fine.
     *
     * Function: server and client, blocking.
     *
     * @param  buffer the source ByteBuffer, where we read from
     * @param  count the number of bytes to write,
     *               which will be corrected to buffer.remaining(), if greater.
     *
     * @return the number of bytes actually written
     *
     * @throws RuntimeException 
     *         May be thrown by the native implementation.
     * @throws IllegalArgumentException 
     *         If an argument does not match the requirements
     * @throws UnixDomainSocketException 
     *         If AF_UNIX error occures
     * @throws BufferUnderflowException 
     *         If the ByteBuffer has no remaining() bytes left
     * @throws UnsupportedOperationException 
     *         If this is not implemented for the running platform
     *
     * @see <a href="http://linux.die.net/man/2/write">write(2)</a>
     */
    public int write(java.nio.ByteBuffer buffer, int count) 
        throws UnixDomainSocketException, BufferUnderflowException, UnsupportedOperationException
    {
        validateSocketStatusForReadWrite();
        if (null==buffer) {
          throw new IllegalArgumentException("Argument buffer is null");
        }
        if (!buffer.isDirect()) {
          throw new IllegalArgumentException("Argument buffer is not direct");
        }
        int limit = buffer.limit();
        int pos   = buffer.position();
        if(pos>=limit) {
          throw new BufferUnderflowException();
        }
        if(pos+count>limit) {
            // fix overflow count
            count=limit-pos;
        }

        int n = UnixSocketImpl.unStreamSocketWrite(socketHandle, buffer, pos, count );

        if( n > 0 ) {
            buffer.position(pos+n);
        }

        return n;
    }

    /**
     * Verifies if socket is an open and valid socket.
     * If the socket is open, a native test will be perfomed.
     *
     * Function: server and client, non-blocking.
     * @return true if this socket is in a open and valid,
     *         else false
     */
    public synchronized boolean isOpenAndValid()
        throws UnsupportedOperationException
    {
        boolean res = false;
        if(0!=socketHandle && socketStatus!=STATUS_CLOSE) {
            try {
                res = UnixSocketImpl.unStreamSocketIsValid(socketHandle);
            } catch (Exception e) {}
        }
        if(!res) {
            socketStatus=STATUS_CLOSE;
        }
        return res;
    }

    /**
     * Check if socket is open.
     * No native test will be perfomed.
     *
     * Function: server and client, non-blocking.
     * @return true if this socket is open
     */
    public boolean isOpen()
    {
        return socketStatus!=STATUS_CLOSE ;
    }

    /**
     * Check if socket is connected.
     * No native test will be perfomed.
     *
     * Function: server and client, non-blocking.
     * @return true if this socket is connected
     */
    public boolean isConnected()
    {
        return socketStatus!=STATUS_CONNECT ;
    }

    /**
     * @see #UnixDomainSocket(String,boolean,int)
     */
    public String getFilename()
    {
        return fileName;
    }

    /**
     * @see #UnixDomainSocket(String,boolean,int)
     */
    public boolean getIsAbstractNamespace()
    {
        return abstractNamespace;
    }

    /**
     * @see #UnixDomainSocket(String,boolean,int)
     */
    public int getProtocol()
    {
        return protocol;
    }

    /**
     * @see #listen(int)
     */
    public int getBacklog()
    {
        return backlog;
    }

    /**
     * @return this sockets current connection state
     */
    public int getStatus()
    {
        return socketStatus;
    }

    /**
     * @return this sockets current connection status,
     */
    public String getStatusAsString()
    {
        switch(socketStatus) {
            case STATUS_CLOSE: return "close";
            case STATUS_OPEN: return "open";
            case STATUS_CONNECT: return "connect";
            case STATUS_BIND: return "bind";
            case STATUS_LISTEN: return "listen";
            case STATUS_ACCEPT: return "accept";
            default: return "invalid";
        }
    }

    /**
     * @return true, if this socket is a server socket
     */
    public boolean isServer() {
        switch(socketStatus) {
            case STATUS_BIND:
            case STATUS_LISTEN:
            case STATUS_ACCEPT:
                return true;
            default:
                return false;
        }
    }

    /**
     * Get the socket's native information as a string.
     * Function: server and client, non-blocking.
     * @return String which contains some native information about the socket
     */
    public String getNativeInfo()
    {
        String res = "n.a.";
        if(0!=socketHandle) {
            try {
                res = UnixSocketImpl.unStreamSocketGetNativeInfo(socketHandle);
            } catch (Exception ex) { }
        }
        return res;
    }

    public String toString() {
        return "UnixDomainSocket["+getStatusAsString()+", pipe: "+getFilename()+
                                 ", ans: "+getIsAbstractNamespace()+
                                 ", proto: "+getProtocol()+
                                 ", backlog: "+getBacklog()+
                                 ", info: "+getNativeInfo()+"]";
    }

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    private static class ShutdownHookUnlinkFiles extends Thread {
        private List /*<File>*/ files;

        public ShutdownHookUnlinkFiles() {
            files = new ArrayList();
        }

        public synchronized void add(String filename) {
            try {
                File file = new File(filename);
                files.add(file);
            } catch (Exception e) {}
        }

        public void run() {
            for( Iterator iter = files.iterator(); iter.hasNext(); ) {
                File file = (File) iter.next();
                try {
                    if(null!=file) {
                        file.delete();
                    }
                } catch (Exception e) {}
            }
        }
    }

    private void setup(String fileName, boolean abstractNamespace, int protocol)
        throws UnixDomainSocketException, UnsupportedOperationException
    {
        if(null==fileName) {
          throw new IllegalArgumentException("Argument fileName is null");
        }
        this.socketHandle=0;
        this.fileName = fileName;
        this.abstractNamespace=abstractNamespace;
        this.protocol=protocol;
        this.backlog=0;
        this.socketStatus=STATUS_CLOSE;

        open();
    }

    /**
     * this is for accept() only,
     * and instantiates a new object in state CONNECT
     */
    private UnixDomainSocket(UnixDomainSocket serverSocket, long clientSocketHandle, int status)
    {
        this.socketHandle = clientSocketHandle;
        this.fileName = serverSocket.fileName;
        this.abstractNamespace=serverSocket.abstractNamespace;
        this.protocol=serverSocket.protocol;
        this.backlog=0;
        this.socketStatus=status;
    }

    private void validateSocketStatusTransition(int newStatus)
        throws UnixDomainSocketException
    {
      if(0==socketHandle && STATUS_CLOSE!=socketStatus) {
          throw new UnixDomainSocketException(toString(), UnixSocketException.EINVAL);
      }
      switch(newStatus) {
        case STATUS_OPEN:
            if(socketStatus==STATUS_CLOSE) {
                return;
            }
            break;
        case STATUS_CONNECT:
            // fall through intended
        case STATUS_BIND:
            if(socketStatus==STATUS_OPEN) {
                return;
            }
            break;
        case STATUS_LISTEN:
            if(socketStatus==STATUS_BIND) {
                return;
            }
            break;
        case STATUS_ACCEPT:
            if(socketStatus==STATUS_LISTEN || socketStatus==STATUS_ACCEPT) {
                return;
            }
      }
      throw new UnixDomainSocketException(toString(), UnixSocketException.EINVAL);
    }

    private void validateSocketStatusForReadWrite()
        throws UnixDomainSocketException
    {
      if(0!=socketHandle && STATUS_CONNECT==socketStatus) {
        return ; // ok
      }
      throw new UnixDomainSocketException(toString(), UnixSocketException.EINVAL);
    }

    private volatile int  socketStatus;
    private volatile long socketHandle;
    private volatile boolean unlinkFile;
    private String fileName;
    private boolean abstractNamespace;
    private int protocol;
    private int backlog;
    private static ShutdownHookUnlinkFiles shutdownHookUnlinkFiles;
    private static final boolean isConfigValid;

    // just to trigger the platform dependent initialization,
    // i.e. native library loading .. etc.
    static {
        isConfigValid = Config.isConfigValid();
        shutdownHookUnlinkFiles = new ShutdownHookUnlinkFiles();
        Runtime.getRuntime().addShutdownHook(shutdownHookUnlinkFiles);
    }
}

/**
 * \example client1RWThreads.java
 * This is a multithreaded client example.
 */

/**
 * \example server1RWThreads.java
 * This is a multithreaded server example.
 */

