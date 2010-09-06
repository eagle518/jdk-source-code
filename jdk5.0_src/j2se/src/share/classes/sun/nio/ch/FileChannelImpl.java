/*
 * @(#)FileChannelImpl.java	1.55 04/06/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.*;
import java.nio.channels.spi.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;
import java.lang.reflect.Field;
import sun.misc.Cleaner;


public class FileChannelImpl
    extends FileChannel
{

    // Used to make native read and write calls
    private static NativeDispatcher nd;

    // Memory allocation size for mapping buffers
    private static long allocationGranularity;

    // Cached field for MappedByteBuffer.isAMappedBuffer
    private static Field isAMappedBufferField;

    // File descriptor
    private FileDescriptor fd;

    // File access mode (immutable)
    private boolean writable;
    private boolean readable;
    private boolean appending;

    // Required to prevent finalization of creating stream (immutable)
    private Object parent;

    // Thread-safe set of IDs of native threads, for signalling
    private NativeThreadSet threads = new NativeThreadSet(2);

    // Keeps track of locks on this file; synchronize on list for access
    private List lockList = new ArrayList(2);

    // Lock for operations involving position and size
    private Object positionLock = new Object();


    private FileChannelImpl(FileDescriptor fd, boolean readable,
                            boolean writable, Object parent, boolean append)
    {
	this.fd = fd;
	this.readable = readable;
	this.writable = writable;
        this.parent = parent;
        this.appending = append;
    }

    // Invoked by getChannel() methods
    // of java.io.File{Input,Output}Stream and RandomAccessFile
    //
    public static FileChannel open(FileDescriptor fd,
				   boolean readable, boolean writable,
				   Object parent)
    {
	return new FileChannelImpl(fd, readable, writable, parent, false);
    }

    public static FileChannel open(FileDescriptor fd,
				   boolean readable, boolean writable,
				   Object parent, boolean append)
    {
	return new FileChannelImpl(fd, readable, writable, parent, append);
    }

    private void ensureOpen() throws IOException {
	if (!isOpen())
	    throw new ClosedChannelException();
    }


    // -- Standard channel operations --

    protected void implCloseChannel() throws IOException {

	nd.preClose(fd);
	threads.signal();

        synchronized(lockList) {
            Iterator i = lockList.iterator();
            while (i.hasNext()) {
                FileLockImpl fli = (FileLockImpl)i.next();
                fli.invalidate();
                release0(fd, fli.position(), fli.size());
            }
            lockList.clear();
        }

	if (parent != null) {

	    // Close the fd via the parent stream's close method.  The parent
	    // will reinvoke our close method, which is defined in the
	    // superclass AbstractInterruptibleChannel, but the isOpen logic in
	    // that method will prevent this method from being reinvoked.
	    //
	    if (parent instanceof FileInputStream)
		((FileInputStream)parent).close();
	    else if (parent instanceof FileOutputStream)
		((FileOutputStream)parent).close();
	    else if (parent instanceof RandomAccessFile)
		((RandomAccessFile)parent).close();
	    else
		assert false;

	} else {
	    nd.close(fd);
	}

    }

    public int read(ByteBuffer dst) throws IOException {
	ensureOpen();
	if (!readable)
	    throw new NonReadableChannelException();
	synchronized (positionLock) {
	    int n = 0;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return 0;
		ti = threads.add();
		do {
		    n = IOUtil.read(fd, dst, -1, nd, positionLock);
		} while ((n == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(n);
	    } finally {
		threads.remove(ti);
		end(n > 0);
		assert IOStatus.check(n);
	    }
	}
    }

    private long read0(ByteBuffer[] dsts) throws IOException {
	ensureOpen();
        if (!readable)
            throw new NonReadableChannelException();
	synchronized (positionLock) {
	    long n = 0;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return 0;
		ti = threads.add();
		do {
		    n = IOUtil.read(fd, dsts, nd);
		} while ((n == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(n);
	    } finally {
		threads.remove(ti);
		end(n > 0);
		assert IOStatus.check(n);
	    }
	}
    }

    public long read(ByteBuffer[] dsts, int offset, int length)
	throws IOException
    {
        if ((offset < 0) || (length < 0) || (offset > dsts.length - length))
           throw new IndexOutOfBoundsException();
	// ## Fix IOUtil.write so that we can avoid this array copy
	return read0(Util.subsequence(dsts, offset, length));
    }

    public int write(ByteBuffer src) throws IOException {
	ensureOpen();
        if (!writable)
            throw new NonWritableChannelException();
	synchronized (positionLock) {
	    int n = 0;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return 0;
		ti = threads.add();
                if (appending)
                    position(size());
		do {
		    n = IOUtil.write(fd, src, -1, nd, positionLock);
		} while ((n == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(n);
	    } finally {
		threads.remove(ti);
		end(n > 0);
		assert IOStatus.check(n);
	    }
	}
    }

    private long write0(ByteBuffer[] srcs) throws IOException {
	ensureOpen();
        if (!writable)
            throw new NonWritableChannelException();
	synchronized (positionLock) {
	    long n = 0;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return 0;
		ti = threads.add();
                if (appending)
                    position(size());
		do {
		    n = IOUtil.write(fd, srcs, nd);
		} while ((n == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(n);
	    } finally {
		threads.remove(ti);
		end(n > 0);
		assert IOStatus.check(n);
	    }
	}
    }

    public long write(ByteBuffer[] srcs, int offset, int length)
	throws IOException
    {
        if ((offset < 0) || (length < 0) || (offset > srcs.length - length))
           throw new IndexOutOfBoundsException();
	// ## Fix IOUtil.write so that we can avoid this array copy
	return write0(Util.subsequence(srcs, offset, length));
    }


    // -- Other operations --

    public long position() throws IOException {
	ensureOpen();
	synchronized (positionLock) {
	    long p = -1;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return 0;
		ti = threads.add();
		do {
		    p = position0(fd, -1);
		} while ((p == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(p);
	    } finally {
		threads.remove(ti);
		end(p > -1);
		assert IOStatus.check(p);
	    }
	}
    }

    public FileChannel position(long newPosition) throws IOException {
	ensureOpen();
        if (newPosition < 0)
            throw new IllegalArgumentException();
	synchronized (positionLock) {
	    long p = -1;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return null;
		ti = threads.add();
		do {
		    p  = position0(fd, newPosition);
		} while ((p == IOStatus.INTERRUPTED) && isOpen());
		return this;
	    } finally {
		threads.remove(ti);
		end(p > -1);
		assert IOStatus.check(p);
	    }
	}
    }

    public long size() throws IOException {
	ensureOpen();
	synchronized (positionLock) {
	    long s = -1;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return -1;
		ti = threads.add();
		do {
		    s = size0(fd);
		} while ((s == IOStatus.INTERRUPTED) && isOpen());
		return IOStatus.normalize(s);
	    } finally {
		threads.remove(ti);
		end(s > -1);
		assert IOStatus.check(s);
	    }
	}
    }

    public FileChannel truncate(long size) throws IOException {
	ensureOpen();
        if (size < 0)
            throw new IllegalArgumentException();
        if (size > size())
            return this;
	if (!writable)
	    throw new NonWritableChannelException();
	synchronized (positionLock) {
	    int rv = -1;
	    int ti = -1;
	    try {
		begin();
		if (!isOpen())
		    return null;
		ti = threads.add();
		do {
		    rv = truncate0(fd, size);
		} while ((rv == IOStatus.INTERRUPTED) && isOpen());
		return this;
	    } finally {
		threads.remove(ti);
		end(rv > -1);
		assert IOStatus.check(rv);
	    }
	}
    }

    public void force(boolean metaData) throws IOException {
	ensureOpen();
	int rv = -1;
	int ti = -1;
	try {
	    begin();
	    if (!isOpen())
		return;
	    ti = threads.add();
	    do {
		rv = force0(fd, metaData);
	    } while ((rv == IOStatus.INTERRUPTED) && isOpen());
	} finally {
	    threads.remove(ti);
	    end(rv > -1);
	    assert IOStatus.check(rv);
	}
    }

    // Assume at first that the underlying kernel supports sendfile();
    // set this to false if we find out later that it doesn't
    //
    private static volatile boolean transferSupported = true;

    // Assume that the underlying kernel sendfile() will work if the target
    // fd is a pipe; set this to false if we find out later that it doesn't
    //
    private static volatile boolean pipeSupported = true;

    // Assume that the underlying kernel sendfile() will work if the target
    // fd is a file; set this to false if we find out later that it doesn't
    //
    private static volatile boolean fileSupported = true;

    private long transferToDirectly(long position, int icount,
				    WritableByteChannel target)
	throws IOException
    {
	if (!transferSupported)
	    return IOStatus.UNSUPPORTED;

	FileDescriptor targetFD = null;
	if (target instanceof FileChannelImpl) {
            if (!fileSupported)
                return IOStatus.UNSUPPORTED_CASE;
	    targetFD = ((FileChannelImpl)target).fd;
	} else if (target instanceof SelChImpl) {
            // Direct transfer to pipe causes EINVAL on some configurations
            if ((target instanceof SinkChannelImpl) && !pipeSupported)
                return IOStatus.UNSUPPORTED_CASE; 
	    targetFD = ((SelChImpl)target).getFD();
        }
	if (targetFD == null)
	    return IOStatus.UNSUPPORTED;
        int thisFDVal = IOUtil.fdVal(fd);
        int targetFDVal = IOUtil.fdVal(targetFD);
        if (thisFDVal == targetFDVal) // Not supported on some configurations
            return IOStatus.UNSUPPORTED;

	long n = -1;
	int ti = -1;
	try {
	    begin();
	    if (!isOpen())
		return -1;
	    ti = threads.add();
	    do {
		n = transferTo0(thisFDVal, position, icount, targetFDVal);
	    } while ((n == IOStatus.INTERRUPTED) && isOpen());
            if (n == IOStatus.UNSUPPORTED_CASE) {
                if (target instanceof SinkChannelImpl)
                    pipeSupported = false;
                if (target instanceof FileChannelImpl)
                    fileSupported = false;
                return IOStatus.UNSUPPORTED_CASE;
            }
            if (n == IOStatus.UNSUPPORTED) {
		// Don't bother trying again
		transferSupported = false;
		return IOStatus.UNSUPPORTED;
	    }
	    return IOStatus.normalize(n);
	} finally {
	    threads.remove(ti);
	    end (n > -1);
	}
    }

    private long transferToTrustedChannel(long position, int icount,
					  WritableByteChannel target)
	throws IOException
    {
	if (  !((target instanceof FileChannelImpl)
		|| (target instanceof SelChImpl)))
	    return IOStatus.UNSUPPORTED;

	// Trusted target: Use a mapped buffer
	MappedByteBuffer dbb = null;
	try {
	    dbb = map(MapMode.READ_ONLY, position, icount);
	    // ## Bug: Closing this channel will not terminate the write
	    return target.write(dbb);
	} finally {
	    if (dbb != null)
		unmap(dbb);
	}
    }

    private long transferToArbitraryChannel(long position, int icount,
					    WritableByteChannel target)
	throws IOException
    {
	// Untrusted target: Use a newly-erased buffer
	int c = Math.min(icount, TRANSFER_SIZE);
        ByteBuffer bb = Util.getTemporaryDirectBuffer(c);
	long tw = 0;			// Total bytes written
	long pos = position;
	try {
            Util.erase(bb);
	    while (tw < icount) {
		bb.limit(Math.min((int)(icount - tw), TRANSFER_SIZE));
		int nr = read(bb, pos);
		if (nr <= 0)
		    break;
		bb.flip();
		// ## Bug: Will block writing target if this channel
		// ##      is asynchronously closed
		int nw = target.write(bb);
		tw += nw;
		if (nw != nr)
		    break;
		pos += nw;
		bb.clear();
	    }
	    return tw;
	} catch (IOException x) {
	    if (tw > 0)
		return tw;
	    throw x;
	} finally {
            Util.releaseTemporaryDirectBuffer(bb);
        }
    }

    public long transferTo(long position, long count,
			   WritableByteChannel target)
	throws IOException
    {
	ensureOpen();
        if (!target.isOpen())
            throw new ClosedChannelException();
        if (!readable)
            throw new NonReadableChannelException();
        if (target instanceof FileChannelImpl &&
            !((FileChannelImpl)target).writable)
            throw new NonWritableChannelException();
        if ((position < 0) || (count < 0))
            throw new IllegalArgumentException();
	long sz = size();
        if (position > sz)
            return 0;
	int icount = (int)Math.min(count, Integer.MAX_VALUE);
        if ((sz - position) < icount)
            icount = (int)(sz - position);

	long n;

	// Attempt a direct transfer, if the kernel supports it
	if ((n = transferToDirectly(position, icount, target)) >= 0)
	    return n;

	// Attempt a mapped transfer, but only to trusted channel types
	if ((n = transferToTrustedChannel(position, icount, target)) >= 0)
	    return n;

	// Slow path for untrusted targets
	return transferToArbitraryChannel(position, icount, target);
    }

    private long transferFromFileChannel(FileChannelImpl src,
                                         long position, long count)
        throws IOException
    {
	MappedByteBuffer dbb = null;
	try {
            long n = 0;
            // Note we could loop here to accumulate more at once
            int icount = (int)Math.min(count, Integer.MAX_VALUE);
            synchronized (src.positionLock) {
                long p = src.position();
		// ## Bug: Closing this channel will not terminate the write
                dbb = src.map(MapMode.READ_ONLY, p, icount);
                n = write(dbb, position);
                src.position(p + n);
            }
            return n;
	} finally {
	    if (dbb != null)
		unmap(dbb);
        }
    }

    private static final int TRANSFER_SIZE = 8192;

    private long transferFromArbitraryChannel(ReadableByteChannel src,
                                              long position, long count)
        throws IOException
    {
	// Untrusted target: Use a newly-erased buffer
	int c = (int)Math.min(count, TRANSFER_SIZE);
        ByteBuffer bb = Util.getTemporaryDirectBuffer(c);
	long tw = 0;			// Total bytes written
	long pos = position;
        try {
            Util.erase(bb);
	    while (tw < count) {
                bb.limit(Math.min((int)(count - tw), TRANSFER_SIZE));
		// ## Bug: Will block reading src if this channel
		// ##      is asynchronously closed
		int nr = src.read(bb);
		if (nr <= 0)
		    break;
		bb.flip();
		int nw = write(bb, pos);
		tw += nw;
		if (nw != nr)
		    break;
		pos += nw;
		bb.clear();
	    }
	    return tw;
	} catch (IOException x) {
	    if (tw > 0)
		return tw;
	    throw x;
	} finally {
            Util.releaseTemporaryDirectBuffer(bb);
        }
    }

    public long transferFrom(ReadableByteChannel src,
			     long position, long count)
	throws IOException
    {
	ensureOpen();
        if (!src.isOpen())
            throw new ClosedChannelException();
        if (!writable)
            throw new NonWritableChannelException();
        if ((position < 0) || (count < 0))
            throw new IllegalArgumentException();
        if (position > size())
            return 0;
        if (src instanceof FileChannelImpl)
           return transferFromFileChannel((FileChannelImpl)src,
					  position, count);

        return transferFromArbitraryChannel(src, position, count);
    }

    public int read(ByteBuffer dst, long position) throws IOException {
        if (dst == null)
            throw new NullPointerException();
        if (position < 0)
            throw new IllegalArgumentException("Negative position");
        if (!readable)
            throw new NonReadableChannelException();
        ensureOpen();
	int n = 0;
	int ti = -1;
	try {
	    begin();
	    if (!isOpen())
		return -1;
	    ti = threads.add();
	    do {
		n = IOUtil.read(fd, dst, position, nd, positionLock);
	    } while ((n == IOStatus.INTERRUPTED) && isOpen());
	    return IOStatus.normalize(n);
	} finally {
	    threads.remove(ti);
	    end(n > 0);
	    assert IOStatus.check(n);
	}
    }

    public int write(ByteBuffer src, long position) throws IOException {
        if (src == null)
            throw new NullPointerException();
        if (position < 0)
            throw new IllegalArgumentException("Negative position");
        if (!writable)
            throw new NonWritableChannelException();
        ensureOpen();
	int n = 0;
	int ti = -1;
	try {
	    begin();
	    if (!isOpen())
		return -1;
	    ti = threads.add();
	    do {
		n = IOUtil.write(fd, src, position, nd, positionLock);
	    } while ((n == IOStatus.INTERRUPTED) && isOpen());
	    return IOStatus.normalize(n);
	} finally {
	    threads.remove(ti);
	    end(n > 0);
	    assert IOStatus.check(n);
	}
    }


    // -- Memory-mapped buffers --

    private static class Unmapper
	implements Runnable
    {

	private long address;
	private long size;

	private Unmapper(long address, long size) {
	    assert (address != 0);
	    this.address = address;
	    this.size = size;
	}

	public void run() {
	    if (address == 0)
		return;
	    unmap0(address, size);
	    address = 0;
	}

    }

    private static void unmap(MappedByteBuffer bb) {
	Cleaner cl = ((DirectBuffer)bb).cleaner();
	if (cl != null)
	    cl.clean();
    }

    private static final int MAP_RO = 0;
    private static final int MAP_RW = 1;
    private static final int MAP_PV = 2;

    public MappedByteBuffer map(MapMode mode, long position, long size)
        throws IOException
    {
        ensureOpen();
	if (position < 0L)
	    throw new IllegalArgumentException("Negative position");
	if (size < 0L)
	    throw new IllegalArgumentException("Negative size");
        if (position + size < 0)
            throw new IllegalArgumentException("Position + size overflow");
	if (size > Integer.MAX_VALUE)
	    throw new IllegalArgumentException("Size exceeds Integer.MAX_VALUE");
	int imode = -1;
        if (mode == MapMode.READ_ONLY)
	    imode = MAP_RO;
	else if (mode == MapMode.READ_WRITE)
	    imode = MAP_RW;
	else if (mode == MapMode.PRIVATE)
	    imode = MAP_PV;
	assert (imode >= 0);
	if ((mode != MapMode.READ_ONLY) && !writable)
	    throw new NonWritableChannelException();
	if (!readable)
	    throw new NonReadableChannelException();

	long addr = -1;
	int ti = -1;
	try {
	    begin();
	    if (!isOpen())
		return null;
	    ti = threads.add();
            if (size() < position + size) { // Extend file size
		int rv;
		do {
		    rv = truncate0(fd, position + size);
		} while ((rv == IOStatus.INTERRUPTED) && isOpen());
            }
            if (size == 0) {
                addr = 0;
                if ((!writable) || (imode == MAP_RO))
                    return Util.newMappedByteBufferR(0, 0, null);
                else
                    return Util.newMappedByteBuffer(0, 0, null);
            }
	    int pagePosition = (int)(position % allocationGranularity);
	    addr = map0(imode, position - pagePosition, size + pagePosition);
            // If no exception was thrown from map0, the address is valid
            assert (IOStatus.checkAll(addr));
            assert (addr % allocationGranularity == 0);
            int isize = (int)size;
            Unmapper um = new Unmapper(addr, size + pagePosition);
            if ((!writable) || (imode == MAP_RO))
                return Util.newMappedByteBufferR(isize, addr + pagePosition, um);
            else
                return Util.newMappedByteBuffer(isize, addr + pagePosition, um);
	} finally {
	    threads.remove(ti);
	    end(IOStatus.checkAll(addr));
	}
    }


    // -- Locks --

    public static final int NO_LOCK = -1;       // Failed to lock
    public static final int LOCKED = 0;         // Obtained requested lock
    public static final int RET_EX_LOCK = 1;    // Obtained exclusive lock
    public static final int INTERRUPTED = 2;    // Request interrupted

    public FileLock lock(long position, long size, boolean shared)
	throws IOException
    {
        ensureOpen();
	if (shared && !readable)
	    throw new NonReadableChannelException();
	if (!shared && !writable)
	    throw new NonWritableChannelException();
        FileLockImpl fli = new FileLockImpl(this, position, size, shared);
        checkList(position, size);
        addList(fli);
        boolean i = true;
	int ti = -1;
        try {
            begin();
	    if (!isOpen())
		return null;
	    ti = threads.add();
            int result = lock0(fd, true, position, size, shared);
            if (result == RET_EX_LOCK) {
                assert shared;
                FileLockImpl fli2 = new FileLockImpl(this, position, size,
                                                     false);
                addList(fli2);
                removeList(fli);
                return fli2;
            }
            if (result == INTERRUPTED || result == NO_LOCK) {
                removeList(fli);
                i = false;
            }
        } catch (IOException e) {
            removeList(fli);
            throw e;
        } finally {
	    threads.remove(ti);
            try {
                end(i);
            } catch (ClosedByInterruptException e) {
                throw new FileLockInterruptionException();
            }
        }
        return fli;
    }

    public FileLock tryLock(long position, long size, boolean shared)
	throws IOException
    {
        ensureOpen();
	if (shared && !readable)
	    throw new NonReadableChannelException();
	if (!shared && !writable)
	    throw new NonWritableChannelException();
        FileLockImpl fli = new FileLockImpl(this, position, size, shared);
        checkList(position, size);
        addList(fli);
	int result = lock0(fd, false, position, size, shared);
	if (result == NO_LOCK) {
	    removeList(fli);
	    return null;
	}
	if (result == RET_EX_LOCK) {
	    assert shared;
	    FileLockImpl fli2 = new FileLockImpl(this, position, size,
						 false);
	    addList(fli2);
	    removeList(fli);
	    return fli2;
	}
        return fli;
    }

    void release(FileLockImpl fli) throws IOException {
	ensureOpen();
        release0(fd, fli.position(), fli.size());
        removeList(fli);
    }

    // Check to see if the specified FileLock is allowed
    void checkList(long position, long size)
        throws OverlappingFileLockException
    {
        synchronized(lockList) {
            Iterator i = lockList.iterator();
            while (i.hasNext()) {
                FileLock fl = (FileLock)i.next();
                if (fl.overlaps(position, size))
                    throw new OverlappingFileLockException();
            }
        }
    }

    // Add the specified FileLock to the list of locks for this channel
    void addList(FileLock fli) {
        synchronized (lockList) {
            lockList.add(fli);
        }
    }

    // Remove the specified FileLock to the list of locks for this channel
    void removeList(FileLock fli) {
        synchronized (lockList) {
            lockList.remove(fli);
        }
    }


    // -- Native methods --

    // Grabs a file lock
    native int lock0(FileDescriptor fd, boolean blocking, long pos, long size,
                     boolean shared) throws IOException;

    // Releases a file lock
    native void release0(FileDescriptor fd, long pos, long size)
        throws IOException;

    // Creates a new mapping
    private native long map0(int prot, long position, long length)
	throws IOException;

    // Removes an existing mapping
    private static native int unmap0(long address, long length);

    // Forces output to device
    private native int force0(FileDescriptor fd, boolean metaData);

    // Truncates a file
    private native int truncate0(FileDescriptor fd, long size);

    // Transfers from src to dst, or returns -2 if kernel can't do that
    private native long transferTo0(int src, long position, long count, int dst);

    // Sets or reports this file's position
    // If offset is -1, the current position is returned
    // otherwise the position is set to offset
    private native long position0(FileDescriptor fd, long offset);

    // Reports this file's size
    private native long size0(FileDescriptor fd);

    // Caches fieldIDs
    private static native long initIDs();

    static {
        Util.load();
	allocationGranularity = initIDs();
        nd = new FileDispatcher();
        isAMappedBufferField = Reflect.lookupField("java.nio.MappedByteBuffer",
                                          "isAMappedBuffer");
    }

}
