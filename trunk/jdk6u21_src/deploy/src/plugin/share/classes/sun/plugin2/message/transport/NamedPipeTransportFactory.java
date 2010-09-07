/*
 * @(#)NamedPipeTransportFactory.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.transport;

import java.io.*;

import sun.plugin2.ipc.*;

class NamedPipeTransportFactory extends TransportFactory {
    private NamedPipe namedPipe;
    private NamedPipeTransport transport;

    NamedPipeTransportFactory() throws IOException {
        namedPipe = IPCFactory.getFactory().createNamedPipe(null);
        transport = new NamedPipeTransport(namedPipe);
    }

    NamedPipeTransportFactory(String[] parameters) throws IOException {
        if (parameters == null || parameters.length == 0)
            throw new IOException("Invalid parameters");

        // Set up the IPC mechanism
        namedPipe = IPCFactory.getFactory().createNamedPipe(IPCFactory.stringToMap(parameters[0]));
        if (namedPipe == null) {
            throw new IOException("Invalid parameters");
        }
        transport = new NamedPipeTransport(namedPipe);
    }

    public String[] getChildProcessParameters() {
        return new String[] { IPCFactory.mapToString(namedPipe.getChildProcessParameters()) };
    }

    public SerializingTransport getTransport() {
        return transport;
    }

    public void dispose() throws IOException {
        if (transport != null) {
            transport.shutdown();
            transport = null;
        }
    }
}
