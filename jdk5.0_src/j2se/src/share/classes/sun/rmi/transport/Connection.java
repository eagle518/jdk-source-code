/*
 * @(#)Connection.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport;

import java.io.*;

public interface Connection {
    /**
     * Gets the input stream for this connection.
     */
    public InputStream getInputStream() throws IOException;

    /*
     * Release the input stream for this connection.
     */
    public void releaseInputStream() throws IOException;

    /**
     * Gets the output stream for this connection
     */
    public OutputStream getOutputStream() throws IOException;

    /*
     * Release the output stream for this connection.
     */
    public void releaseOutputStream() throws IOException;

    /**
     * Return true if channel can be used for multiple operations.
     */
    public boolean isReusable();

    /**
     * Close connection.
     */
    public void close() throws IOException;

    /**
     * Returns the channel for this connection.
     */
    public Channel getChannel();
}
