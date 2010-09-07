/*
 * @(#)UnixIPCFactory.java	1.4 08/03/09
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc.unix;

import java.lang.UnsupportedOperationException;
import java.util.Map;
import sun.plugin2.ipc.*;

public class UnixIPCFactory extends IPCFactory {
    public UnixIPCFactory() {
    }

    public NamedPipe createNamedPipe(Map parameters) {
        String socketServerName;

        if (parameters == null ||
            parameters.get("write_pipe_name") == null) {
            socketServerName = null; // server has to create it
        } else {
            // Get named pipe name (server socket name)
            socketServerName = (String) parameters.get("write_pipe_name");
        }
        return new DomainSocketNamedPipe(socketServerName);
    }

    public Event createEvent(Map parameters) {
        throw new UnsupportedOperationException("not supported on Unix");
    }
}

