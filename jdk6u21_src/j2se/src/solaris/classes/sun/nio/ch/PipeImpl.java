/*
 * @(#)PipeImpl.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;
import java.nio.channels.*;
import java.nio.channels.spi.*;


class PipeImpl
    extends Pipe
{

    // Source and sink channels
    private final SourceChannel source;
    private final SinkChannel sink;

    PipeImpl(SelectorProvider sp) {
        int[] fdes = new int[2];
        IOUtil.initPipe(fdes, true);
	FileDescriptor sourcefd = new FileDescriptor();
        IOUtil.setfdVal(sourcefd, fdes[0]);
	source = new SourceChannelImpl(sp, sourcefd);
	FileDescriptor sinkfd = new FileDescriptor();
        IOUtil.setfdVal(sinkfd, fdes[1]);
        sink = new SinkChannelImpl(sp, sinkfd);
    }

    public SourceChannel source() {
        return source;
    }

    public SinkChannel sink() {
	return sink;
    }

}
