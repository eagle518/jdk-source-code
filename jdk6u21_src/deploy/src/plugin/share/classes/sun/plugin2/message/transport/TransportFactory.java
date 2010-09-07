/*
 * @(#)TransportFactory.java	1.7 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.transport;

import java.io.*;

import sun.plugin2.util.*;

/** This class abstracts the creation of Transport instances. Its
    primary purpose is to abstract away the differences between the
    setup of the SharedMemoryTransport, SocketTransport and
    NamedPipeTransport. */

public abstract class TransportFactory {
    public static final int NAMED_PIPE    = 2;

    /** Creates a new TransportFactory of the given type. This is used
        on the server side to set up the IPC constructs passed to the
        client side. */
    public static TransportFactory create(int type) throws IOException {
        switch (type) {
            case NAMED_PIPE:
                return new NamedPipeTransportFactory();
            default:
                throw new IllegalArgumentException("Unknown type " + type);
        }
    }

    /** Creates a new TransportFactory of the default type for the
        current OS on which we are running. This method is used on the
        server side. */
    public static TransportFactory createForCurrentOS() throws IOException {
        return create(NAMED_PIPE);
    }

    /** Used on the client side to attach to the server side. */
    public static TransportFactory create(int type, String[] parameters) throws IOException {
        switch (type) {
            case NAMED_PIPE:
                return new NamedPipeTransportFactory(parameters);
            default:
                throw new IllegalArgumentException("Unknown type " + type);
        }
    }

    /** Convenience method used on the client side to attach to the
        server side. */
    public static TransportFactory createForCurrentOS(String[] parameters) throws IOException {
        return create(NAMED_PIPE, parameters);
    }

    /** Gets the parameters to be passed to the child process so it
        can connect to the IPC constructs created on the server
        side. */
    public abstract String[] getChildProcessParameters();
    
    /** Returns the Transport instance created by this factory instance. */
    public abstract SerializingTransport getTransport();

    /** Disposes of all resources associated with the Transport instance. */
    public abstract void dispose() throws IOException;
}
