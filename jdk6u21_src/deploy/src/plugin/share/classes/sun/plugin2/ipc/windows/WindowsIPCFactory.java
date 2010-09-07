/*
 * @(#)WindowsIPCFactory.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc.windows;

import java.math.*;
import java.nio.*;
import java.util.*;

import sun.plugin2.ipc.*;
import sun.plugin2.os.windows.*;

public class WindowsIPCFactory extends IPCFactory {
    public WindowsIPCFactory() {
        securityAttributes = SECURITY_ATTRIBUTES.create();
        securityAttributes.nLength(SECURITY_ATTRIBUTES.size());
        securityAttributes.bInheritHandle(1);
    }

    public Event createEvent(Map parameters) {
        String name = null;
        long handle = 0;
        if (parameters == null ||
            parameters.get("evt_name") == null) {
            // Create a new event
            name = nextHandleName("evt");
            handle = Windows.CreateEventA(securityAttributes, false, false, name);
            if (handle == 0) {
                throw new RuntimeException("Error creating Event object");
            }
        } else {
            // Open an existing event
            name = (String) parameters.get("evt_name");
            handle = Windows.OpenEventA(Windows.EVENT_ALL_ACCESS, true, name);
            if (handle == 0) {
                throw new RuntimeException("Error opening Event object \"" + name + "\"");
            }
        }
        return new WindowsEvent(handle, name);
    }

    private static final String PIPE_NAME_PREFIX = "\\\\.\\pipe\\";
    private static final int TIMEOUT = 5000;

    public NamedPipe createNamedPipe(Map parameters) {
        String writeName = null;
        String readName = null;
        long writeHandle = 0;
        long readHandle = 0;
        boolean iAmServer = false;

        // For some reason I don't understand, we aren't able to use a
        // single, duplex, blocking named pipe for this purpose. It
        // appears that simultaneous blocking read and write
        // operations on both the server and client side actually
        // interfere with each other in this mode. To work around this
        // we create two named pipes.

        if (parameters == null ||
            parameters.get("write_pipe_name") == null ||
            parameters.get("read_pipe_name") == null) {
            // Create new named pipes
            writeName = nextHandleName("pipe");
            writeHandle = Windows.CreateNamedPipeA(PIPE_NAME_PREFIX + writeName,
                                                   Windows.PIPE_ACCESS_OUTBOUND,
                                                   Windows.PIPE_TYPE_BYTE,
                                                   Windows.PIPE_UNLIMITED_INSTANCES,
                                                   PIPE_BUF_SZ,
                                                   PIPE_BUF_SZ,
                                                   TIMEOUT,
                                                   null);
            if (writeHandle == 0) {
                throw new RuntimeException("Error creating named pipe for writing");
            }
            readName = nextHandleName("pipe");
            readHandle = Windows.CreateNamedPipeA(PIPE_NAME_PREFIX + readName,
                                                  Windows.PIPE_ACCESS_INBOUND,
                                                  Windows.PIPE_TYPE_BYTE,
                                                  Windows.PIPE_UNLIMITED_INSTANCES,
                                                  PIPE_BUF_SZ,
                                                  PIPE_BUF_SZ,
                                                  TIMEOUT,
                                                  null);
            if (readHandle == 0) {
                throw new RuntimeException("Error creating named pipe for reading");
            }
            iAmServer = true;
        } else {
            // Open existing named pipes
            writeName = (String) parameters.get("write_pipe_name");
            writeHandle = Windows.CreateFileA(PIPE_NAME_PREFIX + writeName,
                                              (int) (Windows.GENERIC_WRITE),
                                              0,
                                              null,
                                              Windows.OPEN_EXISTING,
                                              Windows.FILE_ATTRIBUTE_NORMAL,
                                              0);
            if (writeHandle == 0) {
                throw new RuntimeException("Error opening named pipe \"" + PIPE_NAME_PREFIX + writeName + "\"  for writing");
            }
            readName = (String) parameters.get("read_pipe_name");
            readHandle = Windows.CreateFileA(PIPE_NAME_PREFIX + readName,
                                             (int) (Windows.GENERIC_READ),
                                             0,
                                             null,
                                             Windows.OPEN_EXISTING,
                                             Windows.FILE_ATTRIBUTE_NORMAL,
                                             0);
            if (readHandle == 0) {
                throw new RuntimeException("Error opening named pipe \"" + PIPE_NAME_PREFIX + readName + "\" for reading");
            }
        }
        return new WindowsNamedPipe(writeHandle, readHandle, writeName, readName, iAmServer);
    }
        

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    // Might as well keep this around
    private SECURITY_ATTRIBUTES securityAttributes;

    private String nextHandleName(String handleType) {
        return ("jpi2_pid" + Windows.GetCurrentProcessId() +
                "_" + handleType + nextHandleID());
    }

    private static int currentHandleID;
    private synchronized static int nextHandleID() {
        return ++currentHandleID;
    }
}
