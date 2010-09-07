/*
 * @(#)NamedPipeTransport.java	1.5 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.transport;

import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.channels.*;
import java.lang.reflect.*;
import java.util.*;

import com.sun.deploy.util.BufferUtil;
import sun.plugin2.ipc.*;
import sun.plugin2.message.*;
import sun.plugin2.util.*;

/** A subclass of SerializingTransport which uses a named pipe to
    communicate between two processes on the same machine. */

public class NamedPipeTransport extends SerializingTransport {
    private static final int BUFFER_SIZE = 8 * BufferUtil.KB;

    private volatile NamedPipe namedPipe;

    private ByteBuffer    input;
    private ByteBuffer    output;

    private SerializerImpl serializer;

    /** Creates a new NamedPipeTransport pointing at the given named
        pipe. */
    public NamedPipeTransport(NamedPipe namedPipe) throws IOException {
        this.namedPipe = namedPipe;
        input  = ByteBuffer.allocateDirect(BUFFER_SIZE);
        input.order(ByteOrder.nativeOrder());
        input.limit(0);
        output = ByteBuffer.allocateDirect(BUFFER_SIZE);
        output.order(ByteOrder.nativeOrder());
        serializer = new SerializerImpl();
    }

    /** Shuts down this NamedPipeTransport. */
    public synchronized void shutdown() {
        try {
            if (namedPipe != null) {
                namedPipe.close();
                namedPipe = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void waitForData(long millis) throws IOException {
        serializer.waitForData();
    }

    protected void signalDataWritten() throws IOException{
        getSerializer().flush();
    }

    protected void signalDataRead() {}

    protected boolean isDataAvailable() throws IOException {
        return true;
    }
    
    protected Serializer getSerializer() {
        return serializer;
    }
    
    class SerializerImpl extends AbstractSerializer {

        void waitForData() throws IOException {
            if (input.remaining() > 0) {
                return;
            }
            read();
        }

        void read() throws IOException {
            input.rewind();
            input.limit(input.capacity());
            if(null==namedPipe) {
                throw new IOException("namedPipe shutdown");
            }
            namedPipe.read(input);
            input.flip();
        }

        public void writeByte(byte b) throws IOException {
            if (output.remaining() < 1)
                flush();

            output.put(b);
        }

        public void flush() throws IOException {
            if (output.position() == 0)
                return;

            output.flip();
            if(null==namedPipe) {
                throw new IOException("namedPipe shutdown");
            }
            namedPipe.write(output);
            output.rewind();
            output.limit(output.capacity());
        }

        public byte readByte() throws IOException {
            if (input.remaining() > 0) {
                return input.get();
            }

            read();
            return input.get();
        }
    }

    public String toString() {
        if(null==namedPipe) {
            return "NamedPipe[shutdown]";
        }
        return namedPipe.toString();
    }
}
