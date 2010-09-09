/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.xml.internal.org.jvnet.mimepull;

import java.nio.ByteBuffer;
import java.io.File;
import java.io.IOException;
import java.util.logging.Logger;

/**
 * Keeps the Part's partial content data in memory.
 *
 * @author Kohsuke Kawaguchi
 * @author Jitendra Kotamraju
 */
final class MemoryData implements Data {
    private static final Logger LOGGER = Logger.getLogger(MemoryData.class.getName());

    private final byte[] data;
    private final int len;
    private final MIMEConfig config;

    MemoryData(ByteBuffer buf, MIMEConfig config) {
        data = buf.array();
        len = buf.limit();
        this.config = config;
    }

    // size of the chunk given by the parser
    public int size() {
        return len;
    }

    public byte[] read() {
        return data;
    }

    public long writeTo(DataFile file) {
        return file.writeTo(data, 0, len);
    }

    /**
     * 
     * @param dataHead
     * @param buf
     * @return
     */
    public Data createNext(DataHead dataHead, ByteBuffer buf) {
        if (!config.isOnlyMemory() && dataHead.inMemory >= config.memoryThreshold) {
            try {
                String prefix = config.getTempFilePrefix();
                String suffix = config.getTempFileSuffix();
                File dir = config.getTempDir();
                File tempFile = (dir == null)
                        ? File.createTempFile(prefix, suffix)
                        : File.createTempFile(prefix, suffix, dir);
                LOGGER.fine("Created temp file = "+tempFile);
                dataHead.dataFile = new DataFile(tempFile);
            } catch(IOException ioe) {
                throw new MIMEParsingException(ioe);
            }

            if (dataHead.head != null) {
                for(Chunk c=dataHead.head; c != null; c=c.next) {
                    long pointer = c.data.writeTo(dataHead.dataFile);
                    c.data = new FileData(dataHead.dataFile, pointer, len);
                }
            }
            return new FileData(dataHead.dataFile, buf);
        } else {
            return new MemoryData(buf, config);
        }
    }
}
