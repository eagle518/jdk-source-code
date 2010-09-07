/*
 * @(#)WindowsNamedPipe.java	1.7 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc.windows;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.util.*;

import sun.plugin2.ipc.*;
import sun.plugin2.os.windows.*;

import com.sun.deploy.util.SystemUtils;

public class WindowsNamedPipe extends NamedPipe {
    private long writeHandle;
    private long readHandle;
    private String writeName;
    private String readName;
    private boolean iAmServer;

    // The server side needs a hack to work around deadlocks in the Windows APIs
    private boolean connected;
    private IOException connectException;
    private boolean connectFailed;

    private IntBuffer numReadBuffer    = ByteBuffer.allocateDirect(4).order(ByteOrder.nativeOrder()).asIntBuffer();
    private IntBuffer numWrittenBuffer = ByteBuffer.allocateDirect(4).order(ByteOrder.nativeOrder()).asIntBuffer();

    /** Constructor takes the handle to the named pipe and its
        associated name. */
    public WindowsNamedPipe(long writeHandle, long readHandle, String writeName, String readName, boolean iAmServer) {
        this.writeHandle = writeHandle;
        this.readHandle = readHandle;
        this.writeName = writeName;
        this.readName = readName;
        this.iAmServer = iAmServer;
        if (iAmServer) {
            startConnectThread();
        }
    }

    private void startConnectThread() {
        new Thread(new Runnable() {
                public void run() {
                    try {
                        // Connect the writer
                        if (!Windows.ConnectNamedPipe(writeHandle, null)) {
                            int error = Windows.GetLastError();
                            // ERROR_PIPE_CONNECTED is a valid error, but
                            // for some unknown reason we're seeing a zero
                            // error code plus a false return value from
                            // ConnectNamedPipe even though the call
                            // actually succeeds
                            if (error != Windows.ERROR_PIPE_CONNECTED && error != 0) {
                                throw new IOException("Error " + error + " connecting named pipe");
                            }
                        }

                        // Connect the reader
                        if (!Windows.ConnectNamedPipe(readHandle, null)) {
                            int error = Windows.GetLastError();
                            // ERROR_PIPE_CONNECTED is a valid error, but
                            // for some unknown reason we're seeing a zero
                            // error code plus a false return value from
                            // ConnectNamedPipe even though the call
                            // actually succeeds

                            if (error != Windows.ERROR_PIPE_CONNECTED && error != 0) {
                                throw new IOException("Error " + error + " connecting named pipe");
                            }
                        }

                        synchronized (WindowsNamedPipe.this) {
                            connected = true;
                        }
                    } catch (IOException e) {
                        synchronized (WindowsNamedPipe.this) {
                            connectException = e;
                            connectFailed = true;
                        }
                    }
                    synchronized(WindowsNamedPipe.this) {
                        WindowsNamedPipe.this.notifyAll();
                    }
                }
            }, "Pipe Connector Thread").start();
    }

    private void waitForConnection() throws IOException {
        if (iAmServer) {
            synchronized(this) {
                while (!connected) {
                    if (connectFailed) {
                        if (connectException != null) {
                            throw connectException;
                        } else {
                            throw new IOException("Never received connection from client side");
                        }
                    } else {
                        try {
                            wait();
                        } catch (InterruptedException e) {
                        }
                    }
                }
            }
        }
    }


    public int read(ByteBuffer dest) throws IOException {
        waitForConnection();
        int numToRead = dest.remaining();
        int numRead = 0;
        do {
            long t0 = SystemUtils.microTime();
            boolean res = Windows.ReadFile(readHandle,
                                           dest,
                                           numToRead,
                                           numReadBuffer,
                                           null);
            int error = 0;
            if (!res) {
                error = Windows.GetLastError();
            }
            numRead = numReadBuffer.get(0);
            if (!res) {
                long t1 = SystemUtils.microTime();
                throw new IOException("Error " + error + " reading from named pipe, numRead "+numRead+
                                      ", ReadFile ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
            }
        } while (numRead == 0);
        dest.position(dest.position() + numRead);
        return numRead;
    }



    public int write(ByteBuffer src) throws IOException {
        waitForConnection();
        int numToWrite = src.remaining();
        while (src.hasRemaining()) {
            long t0 = SystemUtils.microTime();
            boolean res = Windows.WriteFile(writeHandle,
                                            src,
                                            numToWrite,
                                            numWrittenBuffer,
                                            null);
            int error = 0;
            if (!res) {
                error = Windows.GetLastError();
            }
            int numWritten = numWrittenBuffer.get(0);
            if (!res) {
                long t1 = SystemUtils.microTime();
                throw new IOException("Error " + error + " writing to named pipe, numWritten "+numWritten+
                                      ", WriteFile ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
            }
            src.position(src.position() + numWritten);
        }
        return numToWrite;
    }

    public void close() throws IOException {
        if (writeHandle == 0 ||
            readHandle == 0) {
            throw new IOException("Already closed");
        }
        if (iAmServer) {
            // These calls can deadlock if the other side never
            // connected to us and we are stuck in a call to
            // ConnectNamedPipe in another thread
            Thread disconnector = new Thread(new Runnable() {
                    public void run() {
                        Windows.DisconnectNamedPipe(writeHandle);
                        Windows.DisconnectNamedPipe(readHandle);
                    }
                }, "Pipe Disconnector Thread");
            disconnector.start();
            try {
                disconnector.join(500);
            } catch (InterruptedException e) {
            }
        }
        final boolean[] resBox = new boolean[1];
        Runnable r = new Runnable() {
                public void run() {
                    boolean res1 = Windows.CloseHandle(writeHandle);
                    boolean res2 = Windows.CloseHandle(readHandle);
                    resBox[0] = (res1 && res2);
                }
            };

        if (iAmServer) {
            // These calls can deadlock if the other side never
            // connected to us and we are stuck in a call to
            // ConnectNamedPipe in another thread
            Thread closer = new Thread(r, "Pipe Closer Thread");
            closer.start();
            try {
                closer.join(500);
            } catch (InterruptedException e) {
            }
        } else {
            r.run();
        }
        writeHandle = 0;
        readHandle = 0;
        synchronized(this) {
            connectFailed = true;
            notifyAll();
        }
        if (!resBox[0]) {
            throw new IOException("Error closing named pipes");
        }
    }

    public boolean isOpen() {
        return (writeHandle != 0 &&
                readHandle != 0);
    }

    public String toString() {
        return "WindowsNamedPipe: server: "+iAmServer+
                              "; readPipe: "+readName +", readBufferSz: " + IPCFactory.PIPE_BUF_SZ +
                              "; writePipe: "+writeName+", writeBufferSz: "+ IPCFactory.PIPE_BUF_SZ;
    }

    public Map getChildProcessParameters() {
        Map ret = new HashMap();
        // Reverse reader and writer pipes for child process
        ret.put("write_pipe_name", readName);
        ret.put("read_pipe_name", writeName);
        return ret;
    }
}
